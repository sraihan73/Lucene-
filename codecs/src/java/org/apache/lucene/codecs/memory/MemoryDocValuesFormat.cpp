using namespace std;

#include "MemoryDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "MemoryDocValuesConsumer.h"
#include "MemoryDocValuesProducer.h"

namespace org::apache::lucene::codecs::memory
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

MemoryDocValuesFormat::MemoryDocValuesFormat()
    : MemoryDocValuesFormat(PackedInts::DEFAULT)
{
}

MemoryDocValuesFormat::MemoryDocValuesFormat(float acceptableOverheadRatio)
    : org::apache::lucene::codecs::DocValuesFormat(L"Memory"),
      acceptableOverheadRatio(acceptableOverheadRatio)
{
}

shared_ptr<DocValuesConsumer> MemoryDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<MemoryDocValuesConsumer>(
      state, DATA_CODEC, DATA_EXTENSION, METADATA_CODEC, METADATA_EXTENSION,
      acceptableOverheadRatio);
}

shared_ptr<DocValuesProducer> MemoryDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<MemoryDocValuesProducer>(
      state, DATA_CODEC, DATA_EXTENSION, METADATA_CODEC, METADATA_EXTENSION);
}

const wstring MemoryDocValuesFormat::DATA_CODEC = L"MemoryDocValuesData";
const wstring MemoryDocValuesFormat::DATA_EXTENSION = L"mdvd";
const wstring MemoryDocValuesFormat::METADATA_CODEC =
    L"MemoryDocValuesMetadata";
const wstring MemoryDocValuesFormat::METADATA_EXTENSION = L"mdvm";
} // namespace org::apache::lucene::codecs::memory