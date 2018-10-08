using namespace std;

#include "AssertingPointsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using AssertingLeafReader = org::apache::lucene::index::AssertingLeafReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

AssertingPointsFormat::AssertingPointsFormat()
    : AssertingPointsFormat(TestUtil::getDefaultCodec()->pointsFormat())
{
}

AssertingPointsFormat::AssertingPointsFormat(shared_ptr<PointsFormat> in_)
    : in_(in_)
{
}

shared_ptr<PointsWriter> AssertingPointsFormat::fieldsWriter(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<AssertingPointsWriter>(state, in_->fieldsWriter(state));
}

shared_ptr<PointsReader> AssertingPointsFormat::fieldsReader(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<AssertingPointsReader>(state->segmentInfo->maxDoc(),
                                            in_->fieldsReader(state));
}

AssertingPointsFormat::AssertingPointsReader::AssertingPointsReader(
    int maxDoc, shared_ptr<PointsReader> in_)
    : in_(in_), maxDoc(maxDoc)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

AssertingPointsFormat::AssertingPointsReader::~AssertingPointsReader()
{
  delete in_;
  delete in_; // close again
}

shared_ptr<PointValues> AssertingPointsFormat::AssertingPointsReader::getValues(
    const wstring &field) 
{
  shared_ptr<PointValues> values = this->in_->getValues(field);
  if (values == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingLeafReader::AssertingPointValues>(values, maxDoc);
}

int64_t AssertingPointsFormat::AssertingPointsReader::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingPointsFormat::AssertingPointsReader::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingPointsFormat::AssertingPointsReader::checkIntegrity() throw(
    IOException)
{
  in_->checkIntegrity();
}

shared_ptr<PointsReader>
AssertingPointsFormat::AssertingPointsReader::getMergeInstance() throw(
    IOException)
{
  return make_shared<AssertingPointsReader>(maxDoc, in_->getMergeInstance());
}

wstring AssertingPointsFormat::AssertingPointsReader::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}

AssertingPointsFormat::AssertingPointsWriter::AssertingPointsWriter(
    shared_ptr<SegmentWriteState> writeState, shared_ptr<PointsWriter> in_)
    : in_(in_)
{
}

void AssertingPointsFormat::AssertingPointsWriter::writeField(
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<PointsReader> values) 
{
  if (fieldInfo->getPointDimensionCount() == 0) {
    throw invalid_argument(L"writing field=\"" + fieldInfo->name +
                           L"\" but pointDimensionalCount is 0");
  }
  in_->writeField(fieldInfo, values);
}

void AssertingPointsFormat::AssertingPointsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  in_->merge(mergeState);
}

void AssertingPointsFormat::AssertingPointsWriter::finish() 
{
  in_->finish();
}

AssertingPointsFormat::AssertingPointsWriter::~AssertingPointsWriter()
{
  delete in_;
  delete in_; // close again
}
} // namespace org::apache::lucene::codecs::asserting