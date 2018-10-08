using namespace std;

#include "Lucene70DocValuesFormat.h"

namespace org::apache::lucene::codecs::lucene70
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;
using SmallFloat = org::apache::lucene::util::SmallFloat;
using DirectWriter = org::apache::lucene::util::packed::DirectWriter;

Lucene70DocValuesFormat::Lucene70DocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(L"Lucene70")
{
}

shared_ptr<DocValuesConsumer> Lucene70DocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<Lucene70DocValuesConsumer>(
      state, DATA_CODEC, DATA_EXTENSION, META_CODEC, META_EXTENSION);
}

shared_ptr<DocValuesProducer> Lucene70DocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<Lucene70DocValuesProducer>(
      state, DATA_CODEC, DATA_EXTENSION, META_CODEC, META_EXTENSION);
}

const wstring Lucene70DocValuesFormat::DATA_CODEC = L"Lucene70DocValuesData";
const wstring Lucene70DocValuesFormat::DATA_EXTENSION = L"dvd";
const wstring Lucene70DocValuesFormat::META_CODEC =
    L"Lucene70DocValuesMetadata";
const wstring Lucene70DocValuesFormat::META_EXTENSION = L"dvm";
} // namespace org::apache::lucene::codecs::lucene70