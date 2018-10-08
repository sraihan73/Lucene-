using namespace std;

#include "CrankyPostingsFormat.h"

namespace org::apache::lucene::codecs::cranky
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Fields = org::apache::lucene::index::Fields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

CrankyPostingsFormat::CrankyPostingsFormat(shared_ptr<PostingsFormat> delegate_,
                                           shared_ptr<Random> random)
    : org::apache::lucene::codecs::PostingsFormat(delegate_->getName()),
      delegate_(delegate_), random(random)
{
  // we impersonate the passed-in codec, so we don't need to be in SPI,
  // and so we dont change file formats
}

shared_ptr<FieldsConsumer> CrankyPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from PostingsFormat.fieldsConsumer()");
  }
  return make_shared<CrankyFieldsConsumer>(delegate_->fieldsConsumer(state),
                                           random);
}

shared_ptr<FieldsProducer> CrankyPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return delegate_->fieldsProducer(state);
}

CrankyPostingsFormat::CrankyFieldsConsumer::CrankyFieldsConsumer(
    shared_ptr<FieldsConsumer> delegate_, shared_ptr<Random> random)
    : delegate_(delegate_), random(random)
{
}

void CrankyPostingsFormat::CrankyFieldsConsumer::write(
    shared_ptr<Fields> fields) 
{
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from FieldsConsumer.write()");
  }
  delegate_->write(fields);
}

CrankyPostingsFormat::CrankyFieldsConsumer::~CrankyFieldsConsumer()
{
  delete delegate_;
  if (random->nextInt(100) == 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from FieldsConsumer.close()");
  }
}
} // namespace org::apache::lucene::codecs::cranky