using namespace std;

#include "IDVersionPostingsReader.h"

namespace org::apache::lucene::codecs::idversion
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;

void IDVersionPostingsReader::init(
    shared_ptr<IndexInput> termsIn,
    shared_ptr<SegmentReadState> state) 
{
  // Make sure we are talking to the matching postings writer
  CodecUtil::checkIndexHeader(termsIn, IDVersionPostingsWriter::TERMS_CODEC,
                              IDVersionPostingsWriter::VERSION_START,
                              IDVersionPostingsWriter::VERSION_CURRENT,
                              state->segmentInfo->getId(),
                              state->segmentSuffix);
}

shared_ptr<BlockTermState> IDVersionPostingsReader::newTermState()
{
  return make_shared<IDVersionTermState>();
}

IDVersionPostingsReader::~IDVersionPostingsReader() {}

void IDVersionPostingsReader::decodeTerm(std::deque<int64_t> &longs,
                                         shared_ptr<DataInput> in_,
                                         shared_ptr<FieldInfo> fieldInfo,
                                         shared_ptr<BlockTermState> _termState,
                                         bool absolute) 
{
  shared_ptr<IDVersionTermState> *const termState =
      std::static_pointer_cast<IDVersionTermState>(_termState);
  termState->docID = in_->readVInt();
  if (absolute) {
    termState->idVersion = in_->readVLong();
  } else {
    termState->idVersion += in_->readZLong();
  }
}

shared_ptr<PostingsEnum> IDVersionPostingsReader::postings(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<BlockTermState> termState,
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  shared_ptr<SingleDocsEnum> docsEnum;

  if (PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    shared_ptr<SinglePostingsEnum> posEnum;

    if (std::dynamic_pointer_cast<SinglePostingsEnum>(reuse) != nullptr) {
      posEnum = std::static_pointer_cast<SinglePostingsEnum>(reuse);
    } else {
      posEnum = make_shared<SinglePostingsEnum>();
    }
    shared_ptr<IDVersionTermState> _termState =
        std::static_pointer_cast<IDVersionTermState>(termState);
    posEnum->reset(_termState->docID, _termState->idVersion);
    return posEnum;
  }

  if (std::dynamic_pointer_cast<SingleDocsEnum>(reuse) != nullptr) {
    docsEnum = std::static_pointer_cast<SingleDocsEnum>(reuse);
  } else {
    docsEnum = make_shared<SingleDocsEnum>();
  }
  docsEnum->reset(
      (std::static_pointer_cast<IDVersionTermState>(termState))->docID);

  return docsEnum;
}

int64_t IDVersionPostingsReader::ramBytesUsed() { return 0; }

void IDVersionPostingsReader::checkIntegrity()  {}

wstring IDVersionPostingsReader::toString()
{
  return getClass().getSimpleName();
}
} // namespace org::apache::lucene::codecs::idversion