using namespace std;

#include "SimpleTextPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "SimpleTextFieldsReader.h"
#include "SimpleTextFieldsWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

SimpleTextPostingsFormat::SimpleTextPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(L"SimpleText")
{
}

shared_ptr<FieldsConsumer> SimpleTextPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<SimpleTextFieldsWriter>(state);
}

shared_ptr<FieldsProducer> SimpleTextPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<SimpleTextFieldsReader>(state);
}

const wstring SimpleTextPostingsFormat::POSTINGS_EXTENSION = L"pst";

wstring
SimpleTextPostingsFormat::getPostingsFileName(const wstring &segment,
                                              const wstring &segmentSuffix)
{
  return IndexFileNames::segmentFileName(segment, segmentSuffix,
                                         POSTINGS_EXTENSION);
}
} // namespace org::apache::lucene::codecs::simpletext