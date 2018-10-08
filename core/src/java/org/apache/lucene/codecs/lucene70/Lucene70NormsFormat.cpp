using namespace std;

#include "Lucene70NormsFormat.h"

namespace org::apache::lucene::codecs::lucene70
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using DataOutput = org::apache::lucene::store::DataOutput;

Lucene70NormsFormat::Lucene70NormsFormat() {}

shared_ptr<NormsConsumer> Lucene70NormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<Lucene70NormsConsumer>(state, DATA_CODEC, DATA_EXTENSION,
                                            METADATA_CODEC, METADATA_EXTENSION);
}

shared_ptr<NormsProducer> Lucene70NormsFormat::normsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<Lucene70NormsProducer>(state, DATA_CODEC, DATA_EXTENSION,
                                            METADATA_CODEC, METADATA_EXTENSION);
}

const wstring Lucene70NormsFormat::DATA_CODEC = L"Lucene70NormsData";
const wstring Lucene70NormsFormat::DATA_EXTENSION = L"nvd";
const wstring Lucene70NormsFormat::METADATA_CODEC = L"Lucene70NormsMetadata";
const wstring Lucene70NormsFormat::METADATA_EXTENSION = L"nvm";
} // namespace org::apache::lucene::codecs::lucene70