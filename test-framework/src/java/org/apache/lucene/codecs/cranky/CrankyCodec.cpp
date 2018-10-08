using namespace std;

#include "CrankyCodec.h"

namespace org::apache::lucene::codecs::cranky
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

CrankyCodec::CrankyCodec(shared_ptr<Codec> delegate_, shared_ptr<Random> random)
    : org::apache::lucene::codecs::FilterCodec(delegate_->getName(), delegate_),
      random(random)
{
  // we impersonate the passed-in codec, so we don't need to be in SPI,
  // and so we dont change file formats
}

shared_ptr<DocValuesFormat> CrankyCodec::docValuesFormat()
{
  return make_shared<CrankyDocValuesFormat>(delegate_->docValuesFormat(),
                                            random);
}

shared_ptr<FieldInfosFormat> CrankyCodec::fieldInfosFormat()
{
  return make_shared<CrankyFieldInfosFormat>(delegate_->fieldInfosFormat(),
                                             random);
}

shared_ptr<LiveDocsFormat> CrankyCodec::liveDocsFormat()
{
  return make_shared<CrankyLiveDocsFormat>(delegate_->liveDocsFormat(), random);
}

shared_ptr<NormsFormat> CrankyCodec::normsFormat()
{
  return make_shared<CrankyNormsFormat>(delegate_->normsFormat(), random);
}

shared_ptr<PostingsFormat> CrankyCodec::postingsFormat()
{
  return make_shared<CrankyPostingsFormat>(delegate_->postingsFormat(), random);
}

shared_ptr<SegmentInfoFormat> CrankyCodec::segmentInfoFormat()
{
  return make_shared<CrankySegmentInfoFormat>(delegate_->segmentInfoFormat(),
                                              random);
}

shared_ptr<StoredFieldsFormat> CrankyCodec::storedFieldsFormat()
{
  return make_shared<CrankyStoredFieldsFormat>(delegate_->storedFieldsFormat(),
                                               random);
}

shared_ptr<TermVectorsFormat> CrankyCodec::termVectorsFormat()
{
  return make_shared<CrankyTermVectorsFormat>(delegate_->termVectorsFormat(),
                                              random);
}

shared_ptr<CompoundFormat> CrankyCodec::compoundFormat()
{
  return make_shared<CrankyCompoundFormat>(delegate_->compoundFormat(), random);
}

shared_ptr<PointsFormat> CrankyCodec::pointsFormat()
{
  return make_shared<CrankyPointsFormat>(delegate_->pointsFormat(), random);
}

wstring CrankyCodec::toString() { return L"Cranky(" + delegate_ + L")"; }
} // namespace org::apache::lucene::codecs::cranky