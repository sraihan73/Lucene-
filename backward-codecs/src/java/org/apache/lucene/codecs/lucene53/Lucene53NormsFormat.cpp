using namespace std;

#include "Lucene53NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "Lucene53NormsProducer.h"

namespace org::apache::lucene::codecs::lucene53
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;

Lucene53NormsFormat::Lucene53NormsFormat() {}

shared_ptr<NormsConsumer> Lucene53NormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  throw make_shared<UnsupportedOperationException>(
      L"This format can only be used for reading");
}

shared_ptr<NormsProducer> Lucene53NormsFormat::normsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<Lucene53NormsProducer>(state, DATA_CODEC, DATA_EXTENSION,
                                            METADATA_CODEC, METADATA_EXTENSION);
}

const wstring Lucene53NormsFormat::DATA_CODEC = L"Lucene53NormsData";
const wstring Lucene53NormsFormat::DATA_EXTENSION = L"nvd";
const wstring Lucene53NormsFormat::METADATA_CODEC = L"Lucene53NormsMetadata";
const wstring Lucene53NormsFormat::METADATA_EXTENSION = L"nvm";
} // namespace org::apache::lucene::codecs::lucene53