using namespace std;

#include "SimpleTextCodec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CompoundFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldInfosFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LiveDocsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PointsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/StoredFieldsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsFormat.h"

namespace org::apache::lucene::codecs::simpletext
{
using Codec = org::apache::lucene::codecs::Codec;
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;

SimpleTextCodec::SimpleTextCodec()
    : org::apache::lucene::codecs::Codec(L"SimpleText")
{
}

shared_ptr<PostingsFormat> SimpleTextCodec::postingsFormat()
{
  return postings;
}

shared_ptr<StoredFieldsFormat> SimpleTextCodec::storedFieldsFormat()
{
  return storedFields;
}

shared_ptr<TermVectorsFormat> SimpleTextCodec::termVectorsFormat()
{
  return vectorsFormat;
}

shared_ptr<FieldInfosFormat> SimpleTextCodec::fieldInfosFormat()
{
  return fieldInfosFormat_;
}

shared_ptr<SegmentInfoFormat> SimpleTextCodec::segmentInfoFormat()
{
  return segmentInfos;
}

shared_ptr<NormsFormat> SimpleTextCodec::normsFormat() { return normsFormat_; }

shared_ptr<LiveDocsFormat> SimpleTextCodec::liveDocsFormat()
{
  return liveDocs;
}

shared_ptr<DocValuesFormat> SimpleTextCodec::docValuesFormat()
{
  return dvFormat;
}

shared_ptr<CompoundFormat> SimpleTextCodec::compoundFormat()
{
  return compoundFormat_;
}

shared_ptr<PointsFormat> SimpleTextCodec::pointsFormat()
{
  return pointsFormat_;
}
} // namespace org::apache::lucene::codecs::simpletext