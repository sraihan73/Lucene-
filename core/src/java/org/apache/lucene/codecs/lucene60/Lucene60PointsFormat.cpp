using namespace std;

#include "Lucene60PointsFormat.h"
#include "../../index/SegmentReadState.h"
#include "../../index/SegmentWriteState.h"
#include "../PointsReader.h"
#include "../PointsWriter.h"
#include "Lucene60PointsReader.h"
#include "Lucene60PointsWriter.h"

namespace org::apache::lucene::codecs::lucene60
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
const wstring Lucene60PointsFormat::DATA_CODEC_NAME =
    L"Lucene60PointsFormatData";
const wstring Lucene60PointsFormat::META_CODEC_NAME =
    L"Lucene60PointsFormatMeta";
const wstring Lucene60PointsFormat::DATA_EXTENSION = L"dim";
const wstring Lucene60PointsFormat::INDEX_EXTENSION = L"dii";

Lucene60PointsFormat::Lucene60PointsFormat() {}

shared_ptr<PointsWriter> Lucene60PointsFormat::fieldsWriter(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<Lucene60PointsWriter>(state);
}

shared_ptr<PointsReader> Lucene60PointsFormat::fieldsReader(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<Lucene60PointsReader>(state);
}
} // namespace org::apache::lucene::codecs::lucene60