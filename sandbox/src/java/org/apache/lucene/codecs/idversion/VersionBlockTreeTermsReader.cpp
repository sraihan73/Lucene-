using namespace std;

#include "VersionBlockTreeTermsReader.h"

namespace org::apache::lucene::codecs::idversion
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::fst::PairOutputs::Pair;

VersionBlockTreeTermsReader::VersionBlockTreeTermsReader(
    shared_ptr<PostingsReaderBase> postingsReader,
    shared_ptr<SegmentReadState> state) 
    : in_(state->directory->openInput(termsFile, state->context)),
      postingsReader(postingsReader)
{

  wstring termsFile = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      VersionBlockTreeTermsWriter::TERMS_EXTENSION);

  bool success = false;
  shared_ptr<IndexInput> indexIn = nullptr;

  try {
    int termsVersion = CodecUtil::checkIndexHeader(
        in_, VersionBlockTreeTermsWriter::TERMS_CODEC_NAME,
        VersionBlockTreeTermsWriter::VERSION_START,
        VersionBlockTreeTermsWriter::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);

    wstring indexFile = IndexFileNames::segmentFileName(
        state->segmentInfo->name, state->segmentSuffix,
        VersionBlockTreeTermsWriter::TERMS_INDEX_EXTENSION);
    indexIn = state->directory->openInput(indexFile, state->context);
    int indexVersion = CodecUtil::checkIndexHeader(
        indexIn, VersionBlockTreeTermsWriter::TERMS_INDEX_CODEC_NAME,
        VersionBlockTreeTermsWriter::VERSION_START,
        VersionBlockTreeTermsWriter::VERSION_CURRENT,
        state->segmentInfo->getId(), state->segmentSuffix);

    if (indexVersion != termsVersion) {
      throw make_shared<CorruptIndexException>(
          L"mixmatched version files: " + in_ + L"=" +
              to_wstring(termsVersion) + L"," + indexIn + L"=" +
              to_wstring(indexVersion),
          indexIn);
    }

    // verify
    CodecUtil::checksumEntireFile(indexIn);

    // Have PostingsReader init itself
    postingsReader->init(in_, state);

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(in_);

    // Read per-field details
    seekDir(in_);
    seekDir(indexIn);

    constexpr int numFields = in_->readVInt();
    if (numFields < 0) {
      throw make_shared<CorruptIndexException>(
          L"invalid numFields: " + to_wstring(numFields), in_);
    }

    for (int i = 0; i < numFields; i++) {
      constexpr int field = in_->readVInt();
      constexpr int64_t numTerms = in_->readVLong();
      assert(numTerms >= 0);
      constexpr int numBytes = in_->readVInt();
      shared_ptr<BytesRef> *const code =
          make_shared<BytesRef>(std::deque<char>(numBytes));
      in_->readBytes(code->bytes, 0, numBytes);
      code->length = numBytes;
      constexpr int64_t version = in_->readVLong();
      shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>> *const rootCode =
          VersionBlockTreeTermsWriter::FST_OUTPUTS->newPair(code, version);
      shared_ptr<FieldInfo> *const fieldInfo =
          state->fieldInfos->fieldInfo(field);
      assert((fieldInfo != nullptr, L"field=" + to_wstring(field)));
      constexpr int64_t sumTotalTermFreq = numTerms;
      constexpr int64_t sumDocFreq = numTerms;
      assert((numTerms <= std, : numeric_limits<int>::max()));
      constexpr int docCount = static_cast<int>(numTerms);
      constexpr int longsSize = in_->readVInt();

      shared_ptr<BytesRef> minTerm = readBytesRef(in_);
      shared_ptr<BytesRef> maxTerm = readBytesRef(in_);
      if (docCount < 0 ||
          docCount > state->segmentInfo
                         ->maxDoc()) { // #docs with field must be <= #docs
        throw make_shared<CorruptIndexException>(
            L"invalid docCount: " + to_wstring(docCount) + L" maxDoc: " +
                to_wstring(state->segmentInfo->maxDoc()),
            in_);
      }
      if (sumDocFreq < docCount) { // #postings must be >= #docs with field
        throw make_shared<CorruptIndexException>(
            L"invalid sumDocFreq: " + to_wstring(sumDocFreq) + L" docCount: " +
                to_wstring(docCount),
            in_);
      }
      if (sumTotalTermFreq != -1 &&
          sumTotalTermFreq < sumDocFreq) { // #positions must be >= #postings
        throw make_shared<CorruptIndexException>(
            L"invalid sumTotalTermFreq: " + to_wstring(sumTotalTermFreq) +
                L" sumDocFreq: " + to_wstring(sumDocFreq),
            in_);
      }
      constexpr int64_t indexStartFP = indexIn->readVLong();
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<VersionFieldReader> previous = fields.emplace(
          fieldInfo->name,
          make_shared<VersionFieldReader>(
              shared_from_this(), fieldInfo, numTerms, rootCode,
              sumTotalTermFreq, sumDocFreq, docCount, indexStartFP, longsSize,
              indexIn, minTerm, maxTerm));
      if (previous != nullptr) {
        throw make_shared<CorruptIndexException>(
            L"duplicate field: " + fieldInfo->name, in_);
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

shared_ptr<BytesRef> VersionBlockTreeTermsReader::readBytesRef(
    shared_ptr<IndexInput> in_) 
{
  shared_ptr<BytesRef> bytes = make_shared<BytesRef>();
  bytes->length = in_->readVInt();
  bytes->bytes = std::deque<char>(bytes->length);
  in_->readBytes(bytes->bytes, 0, bytes->length);
  return bytes;
}

void VersionBlockTreeTermsReader::seekDir(shared_ptr<IndexInput> input) throw(
    IOException)
{
  input->seek(input->length() - CodecUtil::footerLength() - 8);
  int64_t dirOffset = input->readLong();
  input->seek(dirOffset);
}

VersionBlockTreeTermsReader::~VersionBlockTreeTermsReader()
{
  try {
    IOUtils::close({in_, postingsReader});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // Clear so refs to terms index is GCable even if
    // app hangs onto us:
    fields.clear();
  }
}

shared_ptr<Iterator<wstring>> VersionBlockTreeTermsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
VersionBlockTreeTermsReader::terms(const wstring &field) 
{
  assert(field != L"");
  return fields[field];
}

int VersionBlockTreeTermsReader::size() { return fields.size(); }

wstring VersionBlockTreeTermsReader::brToString(shared_ptr<BytesRef> b)
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

int64_t VersionBlockTreeTermsReader::ramBytesUsed()
{
  int64_t sizeInBytes = postingsReader->ramBytesUsed();
  for (auto reader : fields) {
    sizeInBytes += reader->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
VersionBlockTreeTermsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.addAll(Accountables::namedAccountables(L"field", fields));
  resources.push_back(
      Accountables::namedAccountable(L"delegate", postingsReader));
  return Collections::unmodifiableList(resources);
}

void VersionBlockTreeTermsReader::checkIntegrity() 
{
  // term dictionary
  CodecUtil::checksumEntireFile(in_);

  // postings
  postingsReader->checkIntegrity();
}

wstring VersionBlockTreeTermsReader::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(fields=" + fields.size() +
         L",delegate=" + postingsReader->toString() + L")";
}
} // namespace org::apache::lucene::codecs::idversion