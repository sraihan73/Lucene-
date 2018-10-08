using namespace std;

#include "DirectDocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "DirectDocValuesConsumer.h"
#include "DirectDocValuesProducer.h"

namespace org::apache::lucene::codecs::memory
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

DirectDocValuesFormat::DirectDocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(L"Direct")
{
}

shared_ptr<DocValuesConsumer> DirectDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<DirectDocValuesConsumer>(
      state, DATA_CODEC, DATA_EXTENSION, METADATA_CODEC, METADATA_EXTENSION);
}

shared_ptr<DocValuesProducer> DirectDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<DirectDocValuesProducer>(
      state, DATA_CODEC, DATA_EXTENSION, METADATA_CODEC, METADATA_EXTENSION);
}

const wstring DirectDocValuesFormat::DATA_CODEC = L"DirectDocValuesData";
const wstring DirectDocValuesFormat::DATA_EXTENSION = L"dvdd";
const wstring DirectDocValuesFormat::METADATA_CODEC =
    L"DirectDocValuesMetadata";
const wstring DirectDocValuesFormat::METADATA_EXTENSION = L"dvdm";
} // namespace org::apache::lucene::codecs::memory