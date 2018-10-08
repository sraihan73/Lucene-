using namespace std;

#include "Lucene60Codec.h"
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
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene60/Lucene60PointsFormat.h"

namespace org::apache::lucene::codecs::lucene60
{
using Codec = org::apache::lucene::codecs::Codec;
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using Lucene50CompoundFormat =
    org::apache::lucene::codecs::lucene50::Lucene50CompoundFormat;
using Lucene50LiveDocsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50LiveDocsFormat;
using Lucene50SegmentInfoFormat =
    org::apache::lucene::codecs::lucene50::Lucene50SegmentInfoFormat;
using Mode =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode;
using Lucene50StoredFieldsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat;
using Lucene50TermVectorsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50TermVectorsFormat;
using Lucene53NormsFormat =
    org::apache::lucene::codecs::lucene53::Lucene53NormsFormat;
using PerFieldDocValuesFormat =
    org::apache::lucene::codecs::perfield::PerFieldDocValuesFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;

Lucene60Codec::PerFieldPostingsFormatAnonymousInnerClass::
    PerFieldPostingsFormatAnonymousInnerClass()
{
}

shared_ptr<PostingsFormat>
Lucene60Codec::PerFieldPostingsFormatAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return outerInstance->getPostingsFormatForField(field);
}

Lucene60Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    PerFieldDocValuesFormatAnonymousInnerClass()
{
}

shared_ptr<DocValuesFormat>
Lucene60Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return outerInstance->getDocValuesFormatForField(field);
}

Lucene60Codec::Lucene60Codec() : Lucene60Codec(Mode::BEST_SPEED) {}

Lucene60Codec::Lucene60Codec(Mode mode)
    : org::apache::lucene::codecs::Codec(L"Lucene60"),
      storedFieldsFormat(make_shared<Lucene50StoredFieldsFormat>(
          Objects::requireNonNull(mode)))
{
}

shared_ptr<StoredFieldsFormat> Lucene60Codec::storedFieldsFormat()
{
  return storedFieldsFormat_;
}

shared_ptr<TermVectorsFormat> Lucene60Codec::termVectorsFormat()
{
  return vectorsFormat;
}

shared_ptr<PostingsFormat> Lucene60Codec::postingsFormat()
{
  return postingsFormat;
}

shared_ptr<FieldInfosFormat> Lucene60Codec::fieldInfosFormat()
{
  return fieldInfosFormat_;
}

shared_ptr<SegmentInfoFormat> Lucene60Codec::segmentInfoFormat()
{
  return segmentInfosFormat;
}

shared_ptr<LiveDocsFormat> Lucene60Codec::liveDocsFormat()
{
  return liveDocsFormat_;
}

shared_ptr<CompoundFormat> Lucene60Codec::compoundFormat()
{
  return compoundFormat_;
}

shared_ptr<PointsFormat> Lucene60Codec::pointsFormat()
{
  return make_shared<Lucene60PointsFormat>();
}

shared_ptr<PostingsFormat>
Lucene60Codec::getPostingsFormatForField(const wstring &field)
{
  return defaultFormat;
}

shared_ptr<DocValuesFormat>
Lucene60Codec::getDocValuesFormatForField(const wstring &field)
{
  return defaultDVFormat;
}

shared_ptr<DocValuesFormat> Lucene60Codec::docValuesFormat()
{
  return docValuesFormat;
}

shared_ptr<NormsFormat> Lucene60Codec::normsFormat() { return normsFormat_; }
} // namespace org::apache::lucene::codecs::lucene60