using namespace std;

#include "Lucene62Codec.h"
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

namespace org::apache::lucene::codecs::lucene62
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
using Lucene50StoredFieldsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat;
using Lucene50TermVectorsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50TermVectorsFormat;
using Mode =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode;
using Lucene53NormsFormat =
    org::apache::lucene::codecs::lucene53::Lucene53NormsFormat;
using Lucene60FieldInfosFormat =
    org::apache::lucene::codecs::lucene60::Lucene60FieldInfosFormat;
using Lucene60PointsFormat =
    org::apache::lucene::codecs::lucene60::Lucene60PointsFormat;
using PerFieldDocValuesFormat =
    org::apache::lucene::codecs::perfield::PerFieldDocValuesFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;

Lucene62Codec::PerFieldPostingsFormatAnonymousInnerClass::
    PerFieldPostingsFormatAnonymousInnerClass()
{
}

shared_ptr<PostingsFormat>
Lucene62Codec::PerFieldPostingsFormatAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return outerInstance->getPostingsFormatForField(field);
}

Lucene62Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    PerFieldDocValuesFormatAnonymousInnerClass()
{
}

shared_ptr<DocValuesFormat>
Lucene62Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return outerInstance->getDocValuesFormatForField(field);
}

Lucene62Codec::Lucene62Codec() : Lucene62Codec(Mode::BEST_SPEED) {}

Lucene62Codec::Lucene62Codec(Lucene50StoredFieldsFormat::Mode mode)
    : org::apache::lucene::codecs::Codec(L"Lucene62"),
      storedFieldsFormat(make_shared<Lucene50StoredFieldsFormat>(
          Objects::requireNonNull(mode)))
{
}

shared_ptr<StoredFieldsFormat> Lucene62Codec::storedFieldsFormat()
{
  return storedFieldsFormat_;
}

shared_ptr<TermVectorsFormat> Lucene62Codec::termVectorsFormat()
{
  return vectorsFormat;
}

shared_ptr<PostingsFormat> Lucene62Codec::postingsFormat()
{
  return postingsFormat;
}

shared_ptr<FieldInfosFormat> Lucene62Codec::fieldInfosFormat()
{
  return fieldInfosFormat_;
}

shared_ptr<SegmentInfoFormat> Lucene62Codec::segmentInfoFormat()
{
  return segmentInfosFormat;
}

shared_ptr<LiveDocsFormat> Lucene62Codec::liveDocsFormat()
{
  return liveDocsFormat_;
}

shared_ptr<CompoundFormat> Lucene62Codec::compoundFormat()
{
  return compoundFormat_;
}

shared_ptr<PointsFormat> Lucene62Codec::pointsFormat()
{
  return make_shared<Lucene60PointsFormat>();
}

shared_ptr<PostingsFormat>
Lucene62Codec::getPostingsFormatForField(const wstring &field)
{
  return defaultFormat;
}

shared_ptr<DocValuesFormat>
Lucene62Codec::getDocValuesFormatForField(const wstring &field)
{
  return defaultDVFormat;
}

shared_ptr<DocValuesFormat> Lucene62Codec::docValuesFormat()
{
  return docValuesFormat;
}

shared_ptr<NormsFormat> Lucene62Codec::normsFormat() { return normsFormat_; }
} // namespace org::apache::lucene::codecs::lucene62