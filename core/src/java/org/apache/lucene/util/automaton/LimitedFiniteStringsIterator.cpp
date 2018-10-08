using namespace std;

#include "LimitedFiniteStringsIterator.h"

namespace org::apache::lucene::util::automaton
{
using IntsRef = org::apache::lucene::util::IntsRef;

LimitedFiniteStringsIterator::LimitedFiniteStringsIterator(
    shared_ptr<Automaton> a, int limit)
    : FiniteStringsIterator(a)
{

  if (limit != -1 && limit <= 0) {
    throw invalid_argument(
        L"limit must be -1 (which means no limit), or > 0; got: " +
        to_wstring(limit));
  }

  this->limit = limit > 0 ? limit : numeric_limits<int>::max();
}

shared_ptr<IntsRef> LimitedFiniteStringsIterator::next()
{
  if (count >= limit) {
    // Abort on limit.
    return nullptr;
  }

  shared_ptr<IntsRef> result = FiniteStringsIterator::next();
  if (result != nullptr) {
    count++;
  }

  return result;
}

int LimitedFiniteStringsIterator::size() { return count; }
} // namespace org::apache::lucene::util::automaton