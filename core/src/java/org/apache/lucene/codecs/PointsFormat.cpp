using namespace std;

#include "PointsFormat.h"
#include "../index/PointValues.h"
#include "../index/SegmentReadState.h"
#include "../index/SegmentWriteState.h"
#include "PointsWriter.h"

namespace org::apache::lucene::codecs
{
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

PointsFormat::PointsFormat() {}

const shared_ptr<PointsFormat> PointsFormat::EMPTY =
    make_shared<PointsFormatAnonymousInnerClass>();

PointsFormat::PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass()
{
}

shared_ptr<PointsWriter>
PointsFormat::PointsFormatAnonymousInnerClass::fieldsWriter(
    shared_ptr<SegmentWriteState> state)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PointsReader>
PointsFormat::PointsFormatAnonymousInnerClass::fieldsReader(
    shared_ptr<SegmentReadState> state)
{
  return make_shared<PointsReaderAnonymousInnerClass>(shared_from_this());
}

PointsFormat::PointsFormatAnonymousInnerClass::PointsReaderAnonymousInnerClass::
    PointsReaderAnonymousInnerClass(
        shared_ptr<PointsFormatAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

PointsFormat::PointsFormatAnonymousInnerClass::PointsReaderAnonymousInnerClass::
    ~PointsReaderAnonymousInnerClass()
{
}

int64_t PointsFormat::PointsFormatAnonymousInnerClass::
    PointsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0LL;
}

void PointsFormat::PointsFormatAnonymousInnerClass::
    PointsReaderAnonymousInnerClass::checkIntegrity()
{
}

shared_ptr<PointValues> PointsFormat::PointsFormatAnonymousInnerClass::
    PointsReaderAnonymousInnerClass::getValues(const wstring &field)
{
  throw invalid_argument(L"field=\"" + field +
                         L"\" was not indexed with points");
}
} // namespace org::apache::lucene::codecs