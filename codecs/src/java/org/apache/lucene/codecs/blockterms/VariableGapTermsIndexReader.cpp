using namespace std;

#include "VariableGapTermsIndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "VariableGapTermsIndexWriter.h"

namespace org::apache::lucene::codecs::blockterms
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

VariableGapTermsIndexReader::VariableGapTermsIndexReader(
    shared_ptr<SegmentReadState> state) 
{
  wstring fileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      VariableGapTermsIndexWriter::TERMS_INDEX_EXTENSION);
  shared_ptr<IndexInput> *const in_ = state->directory->openInput(
      fileName, make_shared<IOContext>(state->context, true));
  bool success = false;

  try {

    CodecUtil::checkIndexHeader(in_, VariableGapTermsIndexWriter::CODEC_NAME,
                                VariableGapTermsIndexWriter::VERSION_START,
                                VariableGapTermsIndexWriter::VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);

    CodecUtil::checksumEntireFile(in_);

    seekDir(in_);

    // Read directory
    constexpr int numFields = in_->readVInt();
    if (numFields < 0) {
      throw make_shared<CorruptIndexException>(
          L"invalid numFields: " + to_wstring(numFields), in_);
    }

    for (int i = 0; i < numFields; i++) {
      constexpr int field = in_->readVInt();
      constexpr int64_t indexStart = in_->readVLong();
      shared_ptr<FieldInfo> *const fieldInfo =
          state->fieldInfos->fieldInfo(field);
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<FieldIndexData> previous = fields.emplace(
          fieldInfo->name, make_shared<FieldIndexData>(shared_from_this(), in_,
                                                       fieldInfo, indexStart));
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
  }
}

VariableGapTermsIndexReader::IndexEnum::IndexEnum(
    shared_ptr<FST<int64_t>> fst)
    : fstEnum(make_shared<BytesRefFSTEnum<>>(fst))
{
}

shared_ptr<BytesRef> VariableGapTermsIndexReader::IndexEnum::term()
{
  if (current == nullptr) {
    return nullptr;
  } else {
    return current->input;
  }
}

int64_t VariableGapTermsIndexReader::IndexEnum::seek(
    shared_ptr<BytesRef> target) 
{
  // System.out.println("VGR: seek field=" + fieldInfo.name + " target=" +
  // target);
  current = fstEnum->seekFloor(target);
  // System.out.println("  got input=" + current.input + " output=" +
  // current.output);
  return current->output;
}

int64_t VariableGapTermsIndexReader::IndexEnum::next() 
{
  // System.out.println("VGR: next field=" + fieldInfo.name);
  current = fstEnum->next();
  if (current == nullptr) {
    // System.out.println("  eof");
    return -1;
  } else {
    return current->output;
  }
}

int64_t VariableGapTermsIndexReader::IndexEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t VariableGapTermsIndexReader::IndexEnum::seek(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

bool VariableGapTermsIndexReader::supportsOrd() { return false; }

VariableGapTermsIndexReader::FieldIndexData::FieldIndexData(
    shared_ptr<VariableGapTermsIndexReader> outerInstance,
    shared_ptr<IndexInput> in_, shared_ptr<FieldInfo> fieldInfo,
    int64_t indexStart) 
    : fst(make_shared<FST<>>(clone, outerInstance->fstOutputs)),
      outerInstance(outerInstance)
{
  shared_ptr<IndexInput> clone = in_->clone();
  clone->seek(indexStart);
  delete clone;

  /*
  final std::wstring dotFileName = segment + "_" + fieldInfo.name + ".dot";
  Writer w = new OutputStreamWriter(new FileOutputStream(dotFileName));
  Util.toDot(fst, w, false, false);
  System.out.println("FST INDEX: SAVED to " + dotFileName);
  w.close();
  */
}

int64_t VariableGapTermsIndexReader::FieldIndexData::ramBytesUsed()
{
  return fst == nullptr ? 0 : fst->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
VariableGapTermsIndexReader::FieldIndexData::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"index data", fst));
  }
}

wstring VariableGapTermsIndexReader::FieldIndexData::toString()
{
  return L"VarGapTermIndex";
}

shared_ptr<FieldIndexEnum>
VariableGapTermsIndexReader::getFieldEnum(shared_ptr<FieldInfo> fieldInfo)
{
  shared_ptr<FieldIndexData> *const fieldData = fields[fieldInfo->name];
  if (fieldData->fst == nullptr) {
    return nullptr;
  } else {
    return make_shared<IndexEnum>(fieldData->fst);
  }
}

VariableGapTermsIndexReader::~VariableGapTermsIndexReader() {}

void VariableGapTermsIndexReader::seekDir(shared_ptr<IndexInput> input) throw(
    IOException)
{
  input->seek(input->length() - CodecUtil::footerLength() - 8);
  int64_t dirOffset = input->readLong();
  input->seek(dirOffset);
}

int64_t VariableGapTermsIndexReader::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto entry : fields) {
    sizeInBytes += entry->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
VariableGapTermsIndexReader::getChildResources()
{
  return Accountables::namedAccountables(L"field", fields);
}

wstring VariableGapTermsIndexReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() + L")";
}
} // namespace org::apache::lucene::codecs::blockterms