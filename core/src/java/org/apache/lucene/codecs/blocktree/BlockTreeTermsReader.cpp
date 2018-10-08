using namespace std;

#include "BlockTreeTermsReader.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/IndexFileNames.h"
#include "../../index/IndexOptions.h"
#include "../../index/SegmentReadState.h"
#include "../../index/Terms.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountable.h"
#include "../../util/Accountables.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../CodecUtil.h"
#include "../PostingsReaderBase.h"
#include "FieldReader.h"

namespace org::apache::lucene::codecs::blocktree
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using Outputs = org::apache::lucene::util::fst::Outputs;
const shared_ptr<org::apache::lucene::util::fst::Outputs<
    std::shared_ptr<org::apache::lucene::util::BytesRef>>>
    BlockTreeTermsReader::FST_OUTPUTS =
        org::apache::lucene::util::fst::ByteSequenceOutputs::getSingleton();
const shared_ptr<org::apache::lucene::util::BytesRef>
    BlockTreeTermsReader::NO_OUTPUT = FST_OUTPUTS->getNoOutput();
const wstring BlockTreeTermsReader::TERMS_EXTENSION = L"tim";
const wstring BlockTreeTermsReader::TERMS_CODEC_NAME = L"BlockTreeTermsDict";
const wstring BlockTreeTermsReader::TERMS_INDEX_EXTENSION = L"tip";
const wstring BlockTreeTermsReader::TERMS_INDEX_CODEC_NAME =
    L"BlockTreeTermsIndex";

BlockTreeTermsReader::BlockTreeTermsReader(
    shared_ptr<PostingsReaderBase> postingsReader,
    shared_ptr<SegmentReadState> state) 
    : postingsReader(postingsReader), segment(state->segmentInfo->name)
{
  bool success = false;
  shared_ptr<IndexInput> indexIn = nullptr;

  wstring termsName = IndexFileNames::segmentFileName(
      segment, state->segmentSuffix, TERMS_EXTENSION);
  try {
    termsIn = state->directory->openInput(termsName, state->context);
    version = CodecUtil::checkIndexHeader(
        termsIn, TERMS_CODEC_NAME, VERSION_START, VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);

    if (version < VERSION_AUTO_PREFIX_TERMS_REMOVED) {
      // pre-6.2 index, records whether auto-prefix terms are enabled in the
      // header
      char b = termsIn->readByte();
      if (b != 0) {
        throw make_shared<CorruptIndexException>(
            L"Index header pretends the index has auto-prefix terms: " +
                to_wstring(b),
            termsIn);
      }
    }

    wstring indexName = IndexFileNames::segmentFileName(
        segment, state->segmentSuffix, TERMS_INDEX_EXTENSION);
    indexIn = state->directory->openInput(indexName, state->context);
    CodecUtil::checkIndexHeader(indexIn, TERMS_INDEX_CODEC_NAME, version,
                                version, state->segmentInfo->getId(),
                                state->segmentSuffix);
    CodecUtil::checksumEntireFile(indexIn);

    // Have PostingsReader init itself
    postingsReader->init(termsIn, state);

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(termsIn);

    // Read per-field details
    seekDir(termsIn);
    seekDir(indexIn);

    constexpr int numFields = termsIn->readVInt();
    if (numFields < 0) {
      throw make_shared<CorruptIndexException>(
          L"invalid numFields: " + to_wstring(numFields), termsIn);
    }

    for (int i = 0; i < numFields; ++i) {
      constexpr int field = termsIn->readVInt();
      constexpr int64_t numTerms = termsIn->readVLong();
      if (numTerms <= 0) {
        throw make_shared<CorruptIndexException>(
            L"Illegal numTerms for field number: " + to_wstring(field),
            termsIn);
      }
      shared_ptr<BytesRef> *const rootCode = readBytesRef(termsIn);
      shared_ptr<FieldInfo> *const fieldInfo =
          state->fieldInfos->fieldInfo(field);
      if (fieldInfo == nullptr) {
        throw make_shared<CorruptIndexException>(
            L"invalid field number: " + to_wstring(field), termsIn);
      }
      constexpr int64_t sumTotalTermFreq =
          fieldInfo->getIndexOptions() == IndexOptions::DOCS
              ? -1
              : termsIn->readVLong();
      constexpr int64_t sumDocFreq = termsIn->readVLong();
      constexpr int docCount = termsIn->readVInt();
      constexpr int longsSize = termsIn->readVInt();
      if (longsSize < 0) {
        throw make_shared<CorruptIndexException>(
            L"invalid longsSize for field: " + fieldInfo->name +
                L", longsSize=" + to_wstring(longsSize),
            termsIn);
      }
      shared_ptr<BytesRef> minTerm = readBytesRef(termsIn);
      shared_ptr<BytesRef> maxTerm = readBytesRef(termsIn);
      if (docCount < 0 ||
          docCount > state->segmentInfo
                         ->maxDoc()) { // #docs with field must be <= #docs
        throw make_shared<CorruptIndexException>(
            L"invalid docCount: " + to_wstring(docCount) + L" maxDoc: " +
                to_wstring(state->segmentInfo->maxDoc()),
            termsIn);
      }
      if (sumDocFreq < docCount) { // #postings must be >= #docs with field
        throw make_shared<CorruptIndexException>(
            L"invalid sumDocFreq: " + to_wstring(sumDocFreq) + L" docCount: " +
                to_wstring(docCount),
            termsIn);
      }
      if (sumTotalTermFreq != -1 &&
          sumTotalTermFreq < sumDocFreq) { // #positions must be >= #postings
        throw make_shared<CorruptIndexException>(
            L"invalid sumTotalTermFreq: " + to_wstring(sumTotalTermFreq) +
                L" sumDocFreq: " + to_wstring(sumDocFreq),
            termsIn);
      }
      constexpr int64_t indexStartFP = indexIn->readVLong();
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<FieldReader> previous = fields.emplace(
          fieldInfo->name,
          make_shared<FieldReader>(shared_from_this(), fieldInfo, numTerms,
                                   rootCode, sumTotalTermFreq, sumDocFreq,
                                   docCount, indexStartFP, longsSize, indexIn,
                                   minTerm, maxTerm));
      if (previous != nullptr) {
        throw make_shared<CorruptIndexException>(
            L"duplicate field: " + fieldInfo->name, termsIn);
      }
    }

    delete indexIn;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // this.close() will close in:
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({indexIn, shared_from_this()});
    }
  }
}

shared_ptr<BytesRef> BlockTreeTermsReader::readBytesRef(
    shared_ptr<IndexInput> in_) 
{
  int numBytes = in_->readVInt();
  if (numBytes < 0) {
    throw make_shared<CorruptIndexException>(
        L"invalid bytes length: " + to_wstring(numBytes), in_);
  }

  shared_ptr<BytesRef> bytes = make_shared<BytesRef>();
  bytes->length = numBytes;
  bytes->bytes = std::deque<char>(numBytes);
  in_->readBytes(bytes->bytes, 0, numBytes);

  return bytes;
}

void BlockTreeTermsReader::seekDir(shared_ptr<IndexInput> input) throw(
    IOException)
{
  input->seek(input->length() - CodecUtil::footerLength() - 8);
  int64_t offset = input->readLong();
  input->seek(offset);
}

BlockTreeTermsReader::~BlockTreeTermsReader()
{
  try {
    IOUtils::close({termsIn, postingsReader});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // Clear so refs to terms index is GCable even if
    // app hangs onto us:
    fields.clear();
  }
}

shared_ptr<Iterator<wstring>> BlockTreeTermsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
BlockTreeTermsReader::terms(const wstring &field) 
{
  assert(field != L"");
  return fields[field];
}

int BlockTreeTermsReader::size() { return fields.size(); }

wstring BlockTreeTermsReader::brToString(shared_ptr<BytesRef> b)
{
  if (b == nullptr) {
    return L"null";
  } else {
    try {
      return b->utf8ToString() + L" " + b;
    } catch (const runtime_error &t) {
      // If BytesRef isn't actually UTF8, or it's eg a
      // prefix of UTF8 that ends mid-unicode-char, we
      // fallback to hex:
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return b->toString();
    }
  }
}

int64_t BlockTreeTermsReader::ramBytesUsed()
{
  int64_t sizeInBytes = postingsReader->ramBytesUsed();
  for (auto reader : fields) {
    sizeInBytes += reader->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
BlockTreeTermsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(Accountables::namedAccountables(L"field", fields));
  resources.push_back(
      Accountables::namedAccountable(L"delegate", postingsReader));
  return Collections::unmodifiableList(resources);
}

void BlockTreeTermsReader::checkIntegrity() 
{
  // term dictionary
  CodecUtil::checksumEntireFile(termsIn);

  // postings
  postingsReader->checkIntegrity();
}

wstring BlockTreeTermsReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() +
         L",delegate=" + postingsReader + L")";
}
} // namespace org::apache::lucene::codecs::blocktree