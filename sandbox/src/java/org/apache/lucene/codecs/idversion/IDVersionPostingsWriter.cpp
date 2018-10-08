using namespace std;

#include "IDVersionPostingsWriter.h"

namespace org::apache::lucene::codecs::idversion
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PushPostingsWriterBase =
    org::apache::lucene::codecs::PushPostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
const wstring IDVersionPostingsWriter::TERMS_CODEC =
    L"IDVersionPostingsWriterTerms";
const shared_ptr<IDVersionTermState> IDVersionPostingsWriter::emptyState =
    make_shared<IDVersionTermState>();

IDVersionPostingsWriter::IDVersionPostingsWriter(shared_ptr<Bits> liveDocs)
    : liveDocs(liveDocs)
{
}

shared_ptr<BlockTermState> IDVersionPostingsWriter::newTermState()
{
  return make_shared<IDVersionTermState>();
}

void IDVersionPostingsWriter::init(
    shared_ptr<IndexOutput> termsOut,
    shared_ptr<SegmentWriteState> state) 
{
  CodecUtil::writeIndexHeader(termsOut, TERMS_CODEC, VERSION_CURRENT,
                              state->segmentInfo->getId(),
                              state->segmentSuffix);
  segment = state->segmentInfo->name;
}

int IDVersionPostingsWriter::setField(shared_ptr<FieldInfo> fieldInfo)
{
  PushPostingsWriterBase::setField(fieldInfo);
  if (fieldInfo->getIndexOptions() !=
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) {
    throw invalid_argument(
        L"field must be index using IndexOptions.DOCS_AND_FREQS_AND_POSITIONS");
  }
  // LUCENE-5693: because CheckIndex cross-checks term vectors with postings
  // even for deleted docs, and because our PF only indexes the non-deleted
  // documents on flush, CheckIndex will see this as corruption:
  if (fieldInfo->hasVectors()) {
    throw invalid_argument(L"field cannot index term vectors: CheckIndex will "
                           L"report this as index corruption");
  }
  lastState = emptyState;
  return 0;
}

void IDVersionPostingsWriter::startTerm() { lastDocID = -1; }

void IDVersionPostingsWriter::startDoc(int docID,
                                       int termDocFreq) 
{
  // TODO: LUCENE-5693: we don't need this check if we fix IW to not send
  // deleted docs to us on flush:
  if (liveDocs != nullptr && liveDocs->get(docID) == false) {
    return;
  }
  if (lastDocID != -1) {
    throw invalid_argument(L"term appears in more than one document: " +
                           to_wstring(lastDocID) + L" and " +
                           to_wstring(docID));
  }
  if (termDocFreq != 1) {
    throw invalid_argument(L"term appears more than once in the document");
  }

  lastDocID = docID;
  lastPosition = -1;
  lastVersion = -1;
}

void IDVersionPostingsWriter::addPosition(int position,
                                          shared_ptr<BytesRef> payload,
                                          int startOffset,
                                          int endOffset) 
{
  if (lastDocID == -1) {
    // Doc is deleted; skip it
    return;
  }
  if (lastPosition != -1) {
    throw invalid_argument(L"term appears more than once in document");
  }
  lastPosition = position;
  if (payload == nullptr) {
    throw invalid_argument(L"token doens't have a payload");
  }
  if (payload->length != 8) {
    throw invalid_argument(L"payload.length != 8 (got " +
                           to_wstring(payload->length) + L")");
  }

  lastVersion = IDVersionPostingsFormat::bytesToLong(payload);
  if (lastVersion < IDVersionPostingsFormat::MIN_VERSION) {
    throw invalid_argument(L"version must be >= MIN_VERSION=" +
                           to_wstring(IDVersionPostingsFormat::MIN_VERSION) +
                           L" (got: " + to_wstring(lastVersion) +
                           L"; payload=" + payload + L")");
  }
  if (lastVersion > IDVersionPostingsFormat::MAX_VERSION) {
    throw invalid_argument(L"version must be <= MAX_VERSION=" +
                           to_wstring(IDVersionPostingsFormat::MAX_VERSION) +
                           L" (got: " + to_wstring(lastVersion) +
                           L"; payload=" + payload + L")");
  }
}

void IDVersionPostingsWriter::finishDoc() 
{
  if (lastDocID == -1) {
    // Doc is deleted; skip it
    return;
  }
  if (lastPosition == -1) {
    throw invalid_argument(L"missing addPosition");
  }
}

void IDVersionPostingsWriter::finishTerm(
    shared_ptr<BlockTermState> _state) 
{
  if (lastDocID == -1) {
    return;
  }
  shared_ptr<IDVersionTermState> state =
      std::static_pointer_cast<IDVersionTermState>(_state);
  assert(state->docFreq > 0);

  state->docID = lastDocID;
  state->idVersion = lastVersion;
}

void IDVersionPostingsWriter::encodeTerm(std::deque<int64_t> &longs,
                                         shared_ptr<DataOutput> out,
                                         shared_ptr<FieldInfo> fieldInfo,
                                         shared_ptr<BlockTermState> _state,
                                         bool absolute) 
{
  shared_ptr<IDVersionTermState> state =
      std::static_pointer_cast<IDVersionTermState>(_state);
  out->writeVInt(state->docID);
  if (absolute) {
    out->writeVLong(state->idVersion);
  } else {
    int64_t delta = state->idVersion - lastEncodedVersion;
    out->writeZLong(delta);
  }
  lastEncodedVersion = state->idVersion;
}

IDVersionPostingsWriter::~IDVersionPostingsWriter() {}
} // namespace org::apache::lucene::codecs::idversion