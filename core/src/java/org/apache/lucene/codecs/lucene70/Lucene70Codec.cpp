using namespace std;

#include "Lucene70Codec.h"

namespace org::apache::lucene::codecs::lucene70
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
using Mode =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode;
using Lucene50TermVectorsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50TermVectorsFormat;
using Lucene60FieldInfosFormat =
    org::apache::lucene::codecs::lucene60::Lucene60FieldInfosFormat;
using Lucene60PointsFormat =
    org::apache::lucene::codecs::lucene60::Lucene60PointsFormat;
using PerFieldDocValuesFormat =
    org::apache::lucene::codecs::perfield::PerFieldDocValuesFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;

Lucene70Codec::PerFieldPostingsFormatAnonymousInnerClass::
    PerFieldPostingsFormatAnonymousInnerClass()
{
}

shared_ptr<PostingsFormat>
Lucene70Codec::PerFieldPostingsFormatAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return outerInstance->getPostingsFormatForField(field);
}

Lucene70Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    PerFieldDocValuesFormatAnonymousInnerClass()
{
}

shared_ptr<DocValuesFormat>
Lucene70Codec::PerFieldDocValuesFormatAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return outerInstance->getDocValuesFormatForField(field);
}

Lucene70Codec::Lucene70Codec() : Lucene70Codec(Mode::BEST_SPEED) {}

Lucene70Codec::Lucene70Codec(Lucene50StoredFieldsFormat::Mode mode)
    : org::apache::lucene::codecs::Codec(L"Lucene70"),
      storedFieldsFormat(make_shared<Lucene50StoredFieldsFormat>(
          Objects::requireNonNull(mode)))
{
}

shared_ptr<StoredFieldsFormat> Lucene70Codec::storedFieldsFormat()
{
  return storedFieldsFormat_;
}

shared_ptr<TermVectorsFormat> Lucene70Codec::termVectorsFormat()
{
  return vectorsFormat;
}

shared_ptr<PostingsFormat> Lucene70Codec::postingsFormat()
{
  return postingsFormat;
}

shared_ptr<FieldInfosFormat> Lucene70Codec::fieldInfosFormat()
{
  return fieldInfosFormat_;
}

shared_ptr<SegmentInfoFormat> Lucene70Codec::segmentInfoFormat()
{
  return segmentInfosFormat;
}

shared_ptr<LiveDocsFormat> Lucene70Codec::liveDocsFormat()
{
  return liveDocsFormat_;
}

shared_ptr<CompoundFormat> Lucene70Codec::compoundFormat()
{
  return compoundFormat_;
}

shared_ptr<PointsFormat> Lucene70Codec::pointsFormat()
{
  return make_shared<Lucene60PointsFormat>();
}

shared_ptr<PostingsFormat>
Lucene70Codec::getPostingsFormatForField(const wstring &field)
{
  return defaultFormat;
}

shared_ptr<DocValuesFormat>
Lucene70Codec::getDocValuesFormatForField(const wstring &field)
{
  return defaultDVFormat;
}

shared_ptr<DocValuesFormat> Lucene70Codec::docValuesFormat()
{
  return docValuesFormat;
}

shared_ptr<NormsFormat> Lucene70Codec::normsFormat() { return normsFormat_; }
} // namespace org::apache::lucene::codecs::lucene70