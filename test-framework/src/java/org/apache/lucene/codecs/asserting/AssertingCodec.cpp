using namespace std;

#include "AssertingCodec.h"

namespace org::apache::lucene::codecs::asserting
{
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using PerFieldDocValuesFormat =
    org::apache::lucene::codecs::perfield::PerFieldDocValuesFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;
using TestUtil = org::apache::lucene::util::TestUtil;

AssertingCodec::PerFieldPostingsFormatAnonymousInnerClass::
    PerFieldPostingsFormatAnonymousInnerClass()
{
}

shared_ptr<PostingsFormat>
AssertingCodec::PerFieldPostingsFormatAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return outerInstance->getPostingsFormatForField(field);
}

AssertingCodec::PerFieldDocValuesFormatAnonymousInnerClass::
    PerFieldDocValuesFormatAnonymousInnerClass()
{
}

shared_ptr<DocValuesFormat>
AssertingCodec::PerFieldDocValuesFormatAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return outerInstance->getDocValuesFormatForField(field);
}

AssertingCodec::AssertingCodec()
    : org::apache::lucene::codecs::FilterCodec(L"Asserting",
                                               TestUtil::getDefaultCodec())
{
}

shared_ptr<PostingsFormat> AssertingCodec::postingsFormat() { return postings; }

shared_ptr<TermVectorsFormat> AssertingCodec::termVectorsFormat()
{
  return vectors;
}

shared_ptr<StoredFieldsFormat> AssertingCodec::storedFieldsFormat()
{
  return storedFields;
}

shared_ptr<DocValuesFormat> AssertingCodec::docValuesFormat()
{
  return docValues;
}

shared_ptr<NormsFormat> AssertingCodec::normsFormat() { return norms; }

shared_ptr<LiveDocsFormat> AssertingCodec::liveDocsFormat() { return liveDocs; }

shared_ptr<PointsFormat> AssertingCodec::pointsFormat()
{
  return pointsFormat_;
}

wstring AssertingCodec::toString() { return L"Asserting(" + delegate_ + L")"; }

shared_ptr<PostingsFormat>
AssertingCodec::getPostingsFormatForField(const wstring &field)
{
  return defaultFormat;
}

shared_ptr<DocValuesFormat>
AssertingCodec::getDocValuesFormatForField(const wstring &field)
{
  return defaultDVFormat;
}
} // namespace org::apache::lucene::codecs::asserting