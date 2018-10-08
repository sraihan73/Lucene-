using namespace std;

#include "BufferedInputIterator.h"

namespace org::apache::lucene::search::suggest
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using Counter = org::apache::lucene::util::Counter;

BufferedInputIterator::BufferedInputIterator(
    shared_ptr<InputIterator> source) 
    : hasPayloads(source->hasPayloads()), hasContexts(source->hasContexts())
{
  shared_ptr<BytesRef> spare;
  int freqIndex = 0;
  while ((spare = source->next()) != nullptr) {
    entries->append(spare);
    if (hasPayloads_) {
      payloads->append(source->payload());
    }
    if (hasContexts_) {
      contextSets.push_back(source->contexts());
    }
    if (freqIndex >= freqs.size()) {
      freqs = ArrayUtil::grow(freqs, freqs.size() + 1);
    }
    freqs[freqIndex++] = source->weight();
  }
}

int64_t BufferedInputIterator::weight() { return freqs[curPos]; }

shared_ptr<BytesRef> BufferedInputIterator::next() 
{
  if (++curPos < entries->size()) {
    entries->get(spare, curPos);
    return spare->get();
  }
  return nullptr;
}

shared_ptr<BytesRef> BufferedInputIterator::payload()
{
  if (hasPayloads_ && curPos < payloads->size()) {
    return payloads->get(payloadSpare, curPos);
  }
  return nullptr;
}

bool BufferedInputIterator::hasPayloads() { return hasPayloads_; }

shared_ptr<Set<std::shared_ptr<BytesRef>>> BufferedInputIterator::contexts()
{
  if (hasContexts_ && curPos < contextSets.size()) {
    return contextSets[curPos];
  }
  return nullptr;
}

bool BufferedInputIterator::hasContexts() { return hasContexts_; }
} // namespace org::apache::lucene::search::suggest