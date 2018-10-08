using namespace std;

#include "CrankyPointsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

CrankyPointsFormat::CrankyPointsFormat(shared_ptr<PointsFormat> delegate_,
                                       shared_ptr<Random> random)
{
  this->delegate_ = delegate_;
  this->random = random;
}

shared_ptr<PointsWriter> CrankyPointsFormat::fieldsWriter(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<CrankyPointsWriter>(delegate_->fieldsWriter(state),
                                         random);
}

shared_ptr<PointsReader> CrankyPointsFormat::fieldsReader(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<CrankyPointsReader>(delegate_->fieldsReader(state),
                                         random);
}

CrankyPointsFormat::CrankyPointsWriter::CrankyPointsWriter(
    shared_ptr<PointsWriter> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

void CrankyPointsFormat::CrankyPointsWriter::writeField(
    shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<PointsReader> values) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  delegate_->writeField(fieldInfo, values);
}

void CrankyPointsFormat::CrankyPointsWriter::finish() 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  delegate_->finish();
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

void CrankyPointsFormat::CrankyPointsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  delegate_->merge(mergeState);
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

CrankyPointsFormat::CrankyPointsWriter::~CrankyPointsWriter()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

CrankyPointsFormat::CrankyPointsReader::CrankyPointsReader(
    shared_ptr<PointsReader> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

void CrankyPointsFormat::CrankyPointsReader::checkIntegrity() 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  delegate_->checkIntegrity();
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

shared_ptr<PointValues> CrankyPointsFormat::CrankyPointsReader::getValues(
    const wstring &fieldName) 
{
  shared_ptr<PointValues> *const delegate_ =
      this->delegate_->getValues(fieldName);
  if (delegate_ == nullptr) {
    return nullptr;
  }
  return make_shared<PointValuesAnonymousInnerClass>(shared_from_this(),
                                                     delegate_);
}

CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    PointValuesAnonymousInnerClass(shared_ptr<CrankyPointsReader> outerInstance,
                                   shared_ptr<PointValues> delegate_)
{
  this->outerInstance = outerInstance;
  this->delegate_ = delegate_;
}

void CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    intersect(shared_ptr<PointValues::IntersectVisitor> visitor) throw(
        IOException)
{
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  delegate_->intersect(visitor);
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

int64_t
CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    estimatePointCount(shared_ptr<PointValues::IntersectVisitor> visitor)
{
  return delegate_->estimatePointCount(visitor);
}

std::deque<char> CrankyPointsFormat::CrankyPointsReader::
    PointValuesAnonymousInnerClass::getMinPackedValue() 
{
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  return delegate_->getMinPackedValue();
}

std::deque<char> CrankyPointsFormat::CrankyPointsReader::
    PointValuesAnonymousInnerClass::getMaxPackedValue() 
{
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  return delegate_->getMaxPackedValue();
}

int CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    getNumDimensions() 
{
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  return delegate_->getNumDimensions();
}

int CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    getBytesPerDimension() 
{
  if (outerInstance->random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
  return delegate_->getBytesPerDimension();
}

int64_t
CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::size()
{
  return delegate_->size();
}

int CrankyPointsFormat::CrankyPointsReader::PointValuesAnonymousInnerClass::
    getDocCount()
{
  return delegate_->getDocCount();
}

CrankyPointsFormat::CrankyPointsReader::~CrankyPointsReader()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException");
  }
}

int64_t CrankyPointsFormat::CrankyPointsReader::ramBytesUsed()
{
  return delegate_->ramBytesUsed();
}
} // namespace org::apache::lucene::codecs::cranky