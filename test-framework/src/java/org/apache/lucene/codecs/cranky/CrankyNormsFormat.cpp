using namespace std;

#include "CrankyNormsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

CrankyNormsFormat::CrankyNormsFormat(shared_ptr<NormsFormat> delegate_,
                                     shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

shared_ptr<NormsConsumer> CrankyNormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from NormsFormat.normsConsumer()");
  }
  return make_shared<CrankyNormsConsumer>(delegate_->normsConsumer(state),
                                          random);
}

shared_ptr<NormsProducer> CrankyNormsFormat::normsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return delegate_->normsProducer(state);
}

CrankyNormsFormat::CrankyNormsConsumer::CrankyNormsConsumer(
    shared_ptr<NormsConsumer> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

CrankyNormsFormat::CrankyNormsConsumer::~CrankyNormsConsumer()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from NormsConsumer.close()");
  }
}

void CrankyNormsFormat::CrankyNormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    shared_ptr<NormsProducer> valuesProducer) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from NormsConsumer.addNormsField()");
  }
  delegate_->addNormsField(field, valuesProducer);
}
} // namespace org::apache::lucene::codecs::cranky