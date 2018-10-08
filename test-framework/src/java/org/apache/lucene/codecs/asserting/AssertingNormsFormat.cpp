using namespace std;

#include "AssertingNormsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using AssertingLeafReader = org::apache::lucene::index::AssertingLeafReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

shared_ptr<NormsConsumer> AssertingNormsFormat::normsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<NormsConsumer> consumer = in_->normsConsumer(state);
  assert(consumer != nullptr);
  return make_shared<AssertingNormsConsumer>(consumer,
                                             state->segmentInfo->maxDoc());
}

shared_ptr<NormsProducer> AssertingNormsFormat::normsProducer(
    shared_ptr<SegmentReadState> state) 
{
  assert(state->fieldInfos->hasNorms());
  shared_ptr<NormsProducer> producer = in_->normsProducer(state);
  assert(producer != nullptr);
  return make_shared<AssertingNormsProducer>(producer,
                                             state->segmentInfo->maxDoc());
}

AssertingNormsFormat::AssertingNormsConsumer::AssertingNormsConsumer(
    shared_ptr<NormsConsumer> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
}

void AssertingNormsFormat::AssertingNormsConsumer::addNormsField(
    shared_ptr<FieldInfo> field,
    shared_ptr<NormsProducer> valuesProducer) 
{
  shared_ptr<NumericDocValues> values = valuesProducer->getNorms(field);

  int docID;
  int lastDocID = -1;
  while ((docID = values->nextDoc()) != NO_MORE_DOCS) {
    assert(docID >= 0 && docID < maxDoc);
    assert(docID > lastDocID);
    lastDocID = docID;
    int64_t value = values->longValue();
  }

  in_->addNormsField(field, valuesProducer);
}

AssertingNormsFormat::AssertingNormsConsumer::~AssertingNormsConsumer()
{
  delete in_;
  delete in_; // close again
}

AssertingNormsFormat::AssertingNormsProducer::AssertingNormsProducer(
    shared_ptr<NormsProducer> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

shared_ptr<NumericDocValues>
AssertingNormsFormat::AssertingNormsProducer::getNorms(
    shared_ptr<FieldInfo> field) 
{
  assert(field->hasNorms());
  shared_ptr<NumericDocValues> values = in_->getNorms(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingNumericDocValues>(values,
                                                                     maxDoc);
}

AssertingNormsFormat::AssertingNormsProducer::~AssertingNormsProducer()
{
  delete in_;
  delete in_; // close again
}

int64_t AssertingNormsFormat::AssertingNormsProducer::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingNormsFormat::AssertingNormsProducer::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingNormsFormat::AssertingNormsProducer::checkIntegrity() throw(
    IOException)
{
  in_->checkIntegrity();
}

shared_ptr<NormsProducer>
AssertingNormsFormat::AssertingNormsProducer::getMergeInstance() throw(
    IOException)
{
  return make_shared<AssertingNormsProducer>(in_->getMergeInstance(), maxDoc);
}

wstring AssertingNormsFormat::AssertingNormsProducer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}
} // namespace org::apache::lucene::codecs::asserting