using namespace std;

#include "FilterCodec.h"
#include "CompoundFormat.h"
#include "DocValuesFormat.h"
#include "FieldInfosFormat.h"
#include "LiveDocsFormat.h"
#include "NormsFormat.h"
#include "PointsFormat.h"
#include "PostingsFormat.h"
#include "SegmentInfoFormat.h"
#include "StoredFieldsFormat.h"
#include "TermVectorsFormat.h"

namespace org::apache::lucene::codecs
{

FilterCodec::FilterCodec(const wstring &name, shared_ptr<Codec> delegate_)
    : Codec(name), delegate_(delegate_)
{
}

shared_ptr<DocValuesFormat> FilterCodec::docValuesFormat()
{
  return delegate_->docValuesFormat();
}

shared_ptr<FieldInfosFormat> FilterCodec::fieldInfosFormat()
{
  return delegate_->fieldInfosFormat();
}

shared_ptr<LiveDocsFormat> FilterCodec::liveDocsFormat()
{
  return delegate_->liveDocsFormat();
}

shared_ptr<NormsFormat> FilterCodec::normsFormat()
{
  return delegate_->normsFormat();
}

shared_ptr<PostingsFormat> FilterCodec::postingsFormat()
{
  return delegate_->postingsFormat();
}

shared_ptr<SegmentInfoFormat> FilterCodec::segmentInfoFormat()
{
  return delegate_->segmentInfoFormat();
}

shared_ptr<StoredFieldsFormat> FilterCodec::storedFieldsFormat()
{
  return delegate_->storedFieldsFormat();
}

shared_ptr<TermVectorsFormat> FilterCodec::termVectorsFormat()
{
  return delegate_->termVectorsFormat();
}

shared_ptr<CompoundFormat> FilterCodec::compoundFormat()
{
  return delegate_->compoundFormat();
}

shared_ptr<PointsFormat> FilterCodec::pointsFormat()
{
  return delegate_->pointsFormat();
}
} // namespace org::apache::lucene::codecs