using namespace std;

#include "InMemorySorter.h"

namespace org::apache::lucene::search::suggest
{
using BytesRefSorter =
    org::apache::lucene::search::suggest::fst::BytesRefSorter;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using Counter = org::apache::lucene::util::Counter;

InMemorySorter::InMemorySorter(
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator)
    : comparator(comparator)
{
}

void InMemorySorter::add(shared_ptr<BytesRef> utf8)
{
  if (closed) {
    throw make_shared<IllegalStateException>();
  }
  buffer->append(utf8);
}

shared_ptr<BytesRefIterator> InMemorySorter::iterator()
{
  closed = true;
  return buffer->iterator(comparator);
}

shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
InMemorySorter::getComparator()
{
  return comparator;
}
} // namespace org::apache::lucene::search::suggest