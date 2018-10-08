using namespace std;

#include "CompletionPostingsFormat.h"

namespace org::apache::lucene::search::suggest::document
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using FST = org::apache::lucene::util::fst::FST;
const wstring CompletionPostingsFormat::CODEC_NAME = L"completion";
const wstring CompletionPostingsFormat::INDEX_EXTENSION = L"cmp";
const wstring CompletionPostingsFormat::DICT_EXTENSION = L"lkp";

CompletionPostingsFormat::CompletionPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(CODEC_NAME)
{
}

shared_ptr<FieldsConsumer> CompletionPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsFormat> delegatePostingsFormat =
      this->delegatePostingsFormat();
  if (delegatePostingsFormat == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"Error - " + getClassName() +
        L" has been constructed without a choice of PostingsFormat");
  }
  return make_shared<CompletionFieldsConsumer>(delegatePostingsFormat, state);
}

shared_ptr<FieldsProducer> CompletionPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<CompletionFieldsProducer>(state);
}
} // namespace org::apache::lucene::search::suggest::document