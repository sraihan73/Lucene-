using namespace std;

#include "SimpleTextDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "SimpleTextDocValuesReader.h"
#include "SimpleTextDocValuesWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

SimpleTextDocValuesFormat::SimpleTextDocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(L"SimpleText")
{
}

shared_ptr<DocValuesConsumer> SimpleTextDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<SimpleTextDocValuesWriter>(state, L"dat");
}

shared_ptr<DocValuesProducer> SimpleTextDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<SimpleTextDocValuesReader>(state, L"dat");
}
} // namespace org::apache::lucene::codecs::simpletext