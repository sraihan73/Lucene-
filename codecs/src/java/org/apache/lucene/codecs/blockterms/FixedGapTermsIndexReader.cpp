using namespace std;

#include "FixedGapTermsIndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PagedBytes.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedReader.h"
#include "FixedGapTermsIndexWriter.h"

namespace org::apache::lucene::codecs::blockterms
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;

FixedGapTermsIndexReader::FixedGapTermsIndexReader(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PagedBytes> *const termBytes =
      make_shared<PagedBytes>(PAGED_BYTES_BITS);

  wstring fileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      FixedGapTermsIndexWriter::TERMS_INDEX_EXTENSION);
  shared_ptr<IndexInput> *const in_ =
      state->directory->openInput(fileName, state->context);

  bool success = false;

  try {

    CodecUtil::checkIndexHeader(in_, FixedGapTermsIndexWriter::CODEC_NAME,
                                FixedGapTermsIndexWriter::VERSION_CURRENT,
                                FixedGapTermsIndexWriter::VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);

    CodecUtil::checksumEntireFile(in_);

    indexInterval = in_->readVInt();
    if (indexInterval < 1) {
      throw make_shared<CorruptIndexException>(
          L"invalid indexInterval: " + to_wstring(indexInterval), in_);
    }
    packedIntsVersion = in_->readVInt();
    blocksize = in_->readVInt();

    seekDir(in_);

    // Read directory
    constexpr int numFields = in_->readVInt();
    if (numFields < 0) {
      throw make_shared<CorruptIndexException>(
          L"invalid numFields: " + to_wstring(numFields), in_);
    }
    // System.out.println("FGR: init seg=" + segment + " div=" + indexDivisor +
    // " nF=" + numFields);
    for (int i = 0; i < numFields; i++) {
      constexpr int field = in_->readVInt();
      constexpr int64_t numIndexTerms =
          in_->readVInt(); // TODO: change this to a vLong if we fix writer to
                           // support > 2B index terms
      if (numIndexTerms < 0) {
        throw make_shared<CorruptIndexException>(
            L"invalid numIndexTerms: " + to_wstring(numIndexTerms), in_);
      }
      constexpr int64_t termsStart = in_->readVLong();
      constexpr int64_t indexStart = in_->readVLong();
      constexpr int64_t packedIndexStart = in_->readVLong();
      constexpr int64_t packedOffsetsStart = in_->readVLong();
      if (packedIndexStart < indexStart) {
        throw make_shared<CorruptIndexException>(
            L"invalid packedIndexStart: " + to_wstring(packedIndexStart) +
                L" indexStart: " + to_wstring(indexStart) + L"numIndexTerms: " +
                to_wstring(numIndexTerms),
            in_);
      }
      shared_ptr<FieldInfo> *const fieldInfo =
          state->fieldInfos->fieldInfo(field);
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<FieldIndexData> previous = fields.emplace(
          fieldInfo->name,
          make_shared<FieldIndexData>(shared_from_this(), in_, termBytes,
                                      indexStart, termsStart, packedIndexStart,
                                      packedOffsetsStart, numIndexTerms));
      if (previous != nullptr) {
        throw make_shared<CorruptIndexException>(
            L"duplicate field: " + fieldInfo->name, in_);
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({in_});
    } else {
      IOUtils::closeWhileHandlingException({in_});
    }
    termBytesReader = termBytes->freeze(true);
  }
}

FixedGapTermsIndexReader::IndexEnum::IndexEnum(
    shared_ptr<FixedGapTermsIndexReader> outerInstance,
    shared_ptr<FieldIndexData> fieldIndex)
    : fieldIndex(fieldIndex), outerInstance(outerInstance)
{
}

shared_ptr<BytesRef> FixedGapTermsIndexReader::IndexEnum::term()
{
  return term_;
}

int64_t FixedGapTermsIndexReader::IndexEnum::seek(shared_ptr<BytesRef> target)
{
  int64_t lo = 0; // binary search
  int64_t hi = fieldIndex->numIndexTerms - 1;

  while (hi >= lo) {
    int64_t mid =
        static_cast<int64_t>(static_cast<uint64_t>((lo + hi)) >> 1);

    constexpr int64_t offset = fieldIndex->termOffsets->get(mid);
    constexpr int length =
        static_cast<int>(fieldIndex->termOffsets->get(1 + mid) - offset);
    outerInstance->termBytesReader->fillSlice(
        term_, fieldIndex->termBytesStart + offset, length);

    int delta = target->compareTo(term_);
    if (delta < 0) {
      hi = mid - 1;
    } else if (delta > 0) {
      lo = mid + 1;
    } else {
      assert(mid >= 0);
      ord_ = mid * outerInstance->indexInterval;
      return fieldIndex->termsStart + fieldIndex->termsDictOffsets->get(mid);
    }
  }

  if (hi < 0) {
    assert(hi == -1);
    hi = 0;
  }

  constexpr int64_t offset = fieldIndex->termOffsets->get(hi);
  constexpr int length =
      static_cast<int>(fieldIndex->termOffsets->get(1 + hi) - offset);
  outerInstance->termBytesReader->fillSlice(
      term_, fieldIndex->termBytesStart + offset, length);

  ord_ = hi * outerInstance->indexInterval;
  return fieldIndex->termsStart + fieldIndex->termsDictOffsets->get(hi);
}

int64_t FixedGapTermsIndexReader::IndexEnum::next()
{
  constexpr int64_t idx = 1 + (ord_ / outerInstance->indexInterval);
  if (idx >= fieldIndex->numIndexTerms) {
    return -1;
  }
  ord_ += outerInstance->indexInterval;

  constexpr int64_t offset = fieldIndex->termOffsets->get(idx);
  constexpr int length =
      static_cast<int>(fieldIndex->termOffsets->get(1 + idx) - offset);
  outerInstance->termBytesReader->fillSlice(
      term_, fieldIndex->termBytesStart + offset, length);
  return fieldIndex->termsStart + fieldIndex->termsDictOffsets->get(idx);
}

int64_t FixedGapTermsIndexReader::IndexEnum::ord() { return ord_; }

int64_t FixedGapTermsIndexReader::IndexEnum::seek(int64_t ord)
{
  int64_t idx = ord / outerInstance->indexInterval;
  // caller must ensure ord is in bounds
  assert(idx < fieldIndex->numIndexTerms);
  constexpr int64_t offset = fieldIndex->termOffsets->get(idx);
  constexpr int length =
      static_cast<int>(fieldIndex->termOffsets->get(1 + idx) - offset);
  outerInstance->termBytesReader->fillSlice(
      term_, fieldIndex->termBytesStart + offset, length);
  this->ord_ = idx * outerInstance->indexInterval;
  return fieldIndex->termsStart + fieldIndex->termsDictOffsets->get(idx);
}

bool FixedGapTermsIndexReader::supportsOrd() { return true; }

FixedGapTermsIndexReader::FieldIndexData::FieldIndexData(
    shared_ptr<FixedGapTermsIndexReader> outerInstance,
    shared_ptr<IndexInput> in_, shared_ptr<PagedBytes> termBytes,
    int64_t indexStart, int64_t termsStart, int64_t packedIndexStart,
    int64_t packedOffsetsStart, int64_t numIndexTerms) 
    : termBytesStart(termBytes->getPointer()), numIndexTerms(numIndexTerms),
      termsStart(termsStart), outerInstance(outerInstance)
{

  shared_ptr<IndexInput> clone = in_->clone();
  clone->seek(indexStart);

  assert(
      (this->numIndexTerms > 0, L"numIndexTerms=" + to_wstring(numIndexTerms)));

  // slurp in the images from disk:

  try {
    constexpr int64_t numTermBytes = packedIndexStart - indexStart;
    termBytes->copy(clone, numTermBytes);

    // records offsets into main terms dict file
    termsDictOffsets = MonotonicBlockPackedReader::of(
        clone, outerInstance->packedIntsVersion, outerInstance->blocksize,
        numIndexTerms, false);

    // records offsets into byte[] term data
    termOffsets = MonotonicBlockPackedReader::of(
        clone, outerInstance->packedIntsVersion, outerInstance->blocksize,
        1 + numIndexTerms, false);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete clone;
  }
}

int64_t FixedGapTermsIndexReader::FieldIndexData::ramBytesUsed()
{
  return ((termOffsets != nullptr) ? termOffsets->ramBytesUsed() : 0) +
         ((termsDictOffsets != nullptr) ? termsDictOffsets->ramBytesUsed() : 0);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FixedGapTermsIndexReader::FieldIndexData::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (termOffsets != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"term lengths", termOffsets));
  }
  if (termsDictOffsets != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"offsets", termsDictOffsets));
  }
  return Collections::unmodifiableList(resources);
}

wstring FixedGapTermsIndexReader::FieldIndexData::toString()
{
  return L"FixedGapTermIndex(indexterms=" + to_wstring(numIndexTerms) + L")";
}

shared_ptr<FieldIndexEnum>
FixedGapTermsIndexReader::getFieldEnum(shared_ptr<FieldInfo> fieldInfo)
{
  return make_shared<IndexEnum>(shared_from_this(), fields[fieldInfo->name]);
}

FixedGapTermsIndexReader::~FixedGapTermsIndexReader() {}

void FixedGapTermsIndexReader::seekDir(shared_ptr<IndexInput> input) throw(
    IOException)
{
  input->seek(input->length() - CodecUtil::footerLength() - 8);
  int64_t dirOffset = input->readLong();
  input->seek(dirOffset);
}

int64_t FixedGapTermsIndexReader::ramBytesUsed()
{
  int64_t sizeInBytes =
      ((termBytesReader != nullptr) ? termBytesReader->ramBytesUsed() : 0);
  for (auto entry : fields) {
    sizeInBytes += entry->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FixedGapTermsIndexReader::getChildResources()
{
  return Accountables::namedAccountables(L"field", fields);
}

wstring FixedGapTermsIndexReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() +
         L",interval=" + to_wstring(indexInterval) + L")";
}
} // namespace org::apache::lucene::codecs::blockterms