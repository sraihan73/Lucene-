using namespace std;

#include "SimpleTextNormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "SimpleTextDocValuesReader.h"
#include "SimpleTextDocValuesWriter.h"

namespace org::apache::lucene::codecs::simpletext
{
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using EmptyDocValuesProducer =
    org::apache::lucene::index::EmptyDocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;
const wstring SimpleTextNormsFormat::NORMS_SEG_EXTENSION = L"len";

shared_ptr<NormsConsumer> SimpleTextNormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<SimpleTextNormsConsumer>(state);
}

shared_ptr<NormsProducer> SimpleTextNormsFormat::normsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<SimpleTextNormsProducer>(state);
}

SimpleTextNormsFormat::SimpleTextNormsProducer::SimpleTextNormsProducer(
    shared_ptr<SegmentReadState> state) 
    : impl(make_shared<SimpleTextDocValuesReader>(state, NORMS_SEG_EXTENSION))
{
  // All we do is change the extension from .dat -> .len;
  // otherwise this is a normal simple doc values file:
}

shared_ptr<NumericDocValues>
SimpleTextNormsFormat::SimpleTextNormsProducer::getNorms(
    shared_ptr<FieldInfo> field) 
{
  return impl->getNumeric(field);
}

SimpleTextNormsFormat::SimpleTextNormsProducer::~SimpleTextNormsProducer()
{
  delete impl;
}

int64_t SimpleTextNormsFormat::SimpleTextNormsProducer::ramBytesUsed()
{
  return impl->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
SimpleTextNormsFormat::SimpleTextNormsProducer::getChildResources()
{
  return impl->getChildResources();
}

void SimpleTextNormsFormat::SimpleTextNormsProducer::checkIntegrity() throw(
    IOException)
{
  impl->checkIntegrity();
}

wstring SimpleTextNormsFormat::SimpleTextNormsProducer::toString()
{
  return getClass().getSimpleName() + L"(" + impl + L")";
}

SimpleTextNormsFormat::SimpleTextNormsConsumer::SimpleTextNormsConsumer(
    shared_ptr<SegmentWriteState> state) 
    : impl(make_shared<SimpleTextDocValuesWriter>(state, NORMS_SEG_EXTENSION))
{
  // All we do is change the extension from .dat -> .len;
  // otherwise this is a normal simple doc values file:
}

void SimpleTextNormsFormat::SimpleTextNormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    shared_ptr<NormsProducer> normsProducer) 
{
  impl->addNumericField(field,
                        make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                            shared_from_this(), field, normsProducer));
}

SimpleTextNormsFormat::SimpleTextNormsConsumer::
    EmptyDocValuesProducerAnonymousInnerClass::
        EmptyDocValuesProducerAnonymousInnerClass(
            shared_ptr<SimpleTextNormsConsumer> outerInstance,
            shared_ptr<FieldInfo> field,
            shared_ptr<NormsProducer> normsProducer)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->normsProducer = normsProducer;
}

shared_ptr<NumericDocValues> SimpleTextNormsFormat::SimpleTextNormsConsumer::
    EmptyDocValuesProducerAnonymousInnerClass::getNumeric(
        shared_ptr<FieldInfo> field) 
{
  return normsProducer->getNorms(field);
}

SimpleTextNormsFormat::SimpleTextNormsConsumer::~SimpleTextNormsConsumer()
{
  delete impl;
}
} // namespace org::apache::lucene::codecs::simpletext