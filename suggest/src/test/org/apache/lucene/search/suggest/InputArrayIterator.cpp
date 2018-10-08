using namespace std;

#include "InputArrayIterator.h"

namespace org::apache::lucene::search::suggest
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

InputArrayIterator::InputArrayIterator(
    shared_ptr<Iterator<std::shared_ptr<Input>>> i)
    : i(i)
{
  if (i->hasNext()) {
    current = i->next();
    first = true;
    this->hasPayloads_ = current->hasPayloads_;
    this->hasContexts_ = current->hasContexts_;
  } else {
    this->hasPayloads_ = false;
    this->hasContexts_ = false;
  }
}

InputArrayIterator::InputArrayIterator(std::deque<std::shared_ptr<Input>> &i)
    : InputArrayIterator(Arrays::asList(i))
{
}

InputArrayIterator::InputArrayIterator(deque<std::shared_ptr<Input>> &i)
    : InputArrayIterator(i.iterator())
{
}

int64_t InputArrayIterator::weight() { return current->v; }

shared_ptr<BytesRef> InputArrayIterator::next()
{
  if (i->hasNext() || (first && current != nullptr)) {
    if (first) {
      first = false;
    } else {
      current = i->next();
    }
    spare->copyBytes(current->term);
    return spare->get();
  }
  return nullptr;
}

shared_ptr<BytesRef> InputArrayIterator::payload() { return current->payload; }

bool InputArrayIterator::hasPayloads() { return hasPayloads_; }

shared_ptr<Set<std::shared_ptr<BytesRef>>> InputArrayIterator::contexts()
{
  return current->contexts;
}

bool InputArrayIterator::hasContexts() { return hasContexts_; }
} // namespace org::apache::lucene::search::suggest