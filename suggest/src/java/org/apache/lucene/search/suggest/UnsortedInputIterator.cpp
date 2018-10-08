using namespace std;

#include "UnsortedInputIterator.h"

namespace org::apache::lucene::search::suggest
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

UnsortedInputIterator::UnsortedInputIterator(
    shared_ptr<InputIterator> source) 
    : BufferedInputIterator(source), ords(std::deque<int>(entries->size()))
{
  shared_ptr<Random> random = make_shared<Random>();
  for (int i = 0; i < ords.size(); i++) {
    ords[i] = i;
  }
  for (int i = 0; i < ords.size(); i++) {
    int randomPosition = random->nextInt(ords.size());
    int temp = ords[i];
    ords[i] = ords[randomPosition];
    ords[randomPosition] = temp;
  }
}

int64_t UnsortedInputIterator::weight()
{
  assert(currentOrd == ords[curPos]);
  return freqs[currentOrd];
}

shared_ptr<BytesRef> UnsortedInputIterator::next() 
{
  if (++curPos < entries->size()) {
    currentOrd = ords[curPos];
    return entries->get(spare, currentOrd);
  }
  return nullptr;
}

shared_ptr<BytesRef> UnsortedInputIterator::payload()
{
  if (hasPayloads() && curPos < payloads->size()) {
    assert(currentOrd == ords[curPos]);
    return payloads->get(payloadSpare, currentOrd);
  }
  return nullptr;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>> UnsortedInputIterator::contexts()
{
  if (hasContexts() && curPos < contextSets.size()) {
    assert(currentOrd == ords[curPos]);
    return contextSets[currentOrd];
  }
  return nullptr;
}
} // namespace org::apache::lucene::search::suggest