using namespace std;

#include "SimpleTextPointsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "SimpleTextPointsReader.h"
#include "SimpleTextPointsWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

shared_ptr<PointsWriter> SimpleTextPointsFormat::fieldsWriter(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<SimpleTextPointsWriter>(state);
}

shared_ptr<PointsReader> SimpleTextPointsFormat::fieldsReader(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<SimpleTextPointsReader>(state);
}

const wstring SimpleTextPointsFormat::POINT_EXTENSION = L"dim";
const wstring SimpleTextPointsFormat::POINT_INDEX_EXTENSION = L"dii";
} // namespace org::apache::lucene::codecs::simpletext