using namespace std;

#include "TestBloomFilteredLucenePostings.h"

namespace org::apache::lucene::codecs::bloom
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<FuzzySet>
TestBloomFilteredLucenePostings::LowMemoryBloomFactory::getSetForField(
    shared_ptr<SegmentWriteState> state, shared_ptr<FieldInfo> info)
{
  return FuzzySet::createSetBasedOnMaxMemory(1024);
}

bool TestBloomFilteredLucenePostings::LowMemoryBloomFactory::isSaturated(
    shared_ptr<FuzzySet> bloomFilter, shared_ptr<FieldInfo> fieldInfo)
{
  // For test purposes always maintain the BloomFilter - even past the point
  // of usefulness when all bits are set
  return false;
}

TestBloomFilteredLucenePostings::TestBloomFilteredLucenePostings()
    : org::apache::lucene::codecs::PostingsFormat(
          L"TestBloomFilteredLucenePostings")
{
  delegate_ = make_shared<BloomFilteringPostingsFormat>(
      TestUtil::getDefaultPostingsFormat(),
      make_shared<LowMemoryBloomFactory>());
}

shared_ptr<FieldsConsumer> TestBloomFilteredLucenePostings::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return delegate_->fieldsConsumer(state);
}

shared_ptr<FieldsProducer> TestBloomFilteredLucenePostings::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return delegate_->fieldsProducer(state);
}

wstring TestBloomFilteredLucenePostings::toString()
{
  return L"TestBloomFilteredLucenePostings(" + delegate_ + L")";
}
} // namespace org::apache::lucene::codecs::bloom