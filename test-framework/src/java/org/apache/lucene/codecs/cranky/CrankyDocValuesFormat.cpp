using namespace std;

#include "CrankyDocValuesFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

CrankyDocValuesFormat::CrankyDocValuesFormat(
    shared_ptr<DocValuesFormat> delegate_, shared_ptr<Random> random)
    : org::apache::lucene::codecs::DocValuesFormat(delegate_->getName()),
      delegate_(delegate_), random(random)
{
  // we impersonate the passed-in codec, so we don't need to be in SPI,
  // and so we dont change file formats
}

shared_ptr<DocValuesConsumer> CrankyDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesFormat.fieldsConsumer()");
  }
  return make_shared<CrankyDocValuesConsumer>(delegate_->fieldsConsumer(state),
                                              random);
}

shared_ptr<DocValuesProducer> CrankyDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return delegate_->fieldsProducer(state);
}

CrankyDocValuesFormat::CrankyDocValuesConsumer::CrankyDocValuesConsumer(
    shared_ptr<DocValuesConsumer> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

CrankyDocValuesFormat::CrankyDocValuesConsumer::~CrankyDocValuesConsumer()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.close()");
  }
}

void CrankyDocValuesFormat::CrankyDocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.addNumericField()");
  }
  delegate_->addNumericField(field, valuesProducer);
}

void CrankyDocValuesFormat::CrankyDocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.addBinaryField()");
  }
  delegate_->addBinaryField(field, valuesProducer);
}

void CrankyDocValuesFormat::CrankyDocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.addSortedField()");
  }
  delegate_->addSortedField(field, valuesProducer);
}

void CrankyDocValuesFormat::CrankyDocValuesConsumer::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.addSortedNumericField()");
  }
  delegate_->addSortedNumericField(field, valuesProducer);
}

void CrankyDocValuesFormat::CrankyDocValuesConsumer::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from DocValuesConsumer.addSortedSetField()");
  }
  delegate_->addSortedSetField(field, valuesProducer);
}
} // namespace org::apache::lucene::codecs::cranky