using namespace std;

#include "Lucene54DocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "Lucene54DocValuesConsumer.h"
#include "Lucene54DocValuesProducer.h"

namespace org::apache::lucene::codecs::lucene54
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using Lucene70DocValuesFormat =
    org::apache::lucene::codecs::lucene70::Lucene70DocValuesFormat;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SmallFloat = org::apache::lucene::util::SmallFloat;
using DirectWriter = org::apache::lucene::util::packed::DirectWriter;

Lucene54DocValuesFormat::Lucene54DocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(L"Lucene54")
{
}

shared_ptr<DocValuesConsumer> Lucene54DocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<Lucene54DocValuesConsumer>(
      state, DATA_CODEC, DATA_EXTENSION, META_CODEC, META_EXTENSION);
}

shared_ptr<DocValuesProducer> Lucene54DocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<Lucene54DocValuesProducer>(
      state, DATA_CODEC, DATA_EXTENSION, META_CODEC, META_EXTENSION);
}

const wstring Lucene54DocValuesFormat::DATA_CODEC = L"Lucene54DocValuesData";
const wstring Lucene54DocValuesFormat::DATA_EXTENSION = L"dvd";
const wstring Lucene54DocValuesFormat::META_CODEC =
    L"Lucene54DocValuesMetadata";
const wstring Lucene54DocValuesFormat::META_EXTENSION = L"dvm";
} // namespace org::apache::lucene::codecs::lucene54