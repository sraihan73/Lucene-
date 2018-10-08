using namespace std;

#include "IntArrayDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

IntArrayDocIdSet::IntArrayDocIdSet(std::deque<int> &docs, int length)
    : docs(docs), length(length)
{
  if (docs[length] != DocIdSetIterator::NO_MORE_DOCS) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
}

int64_t IntArrayDocIdSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(docs);
}

shared_ptr<DocIdSetIterator> IntArrayDocIdSet::iterator() 
{
  return make_shared<IntArrayDocIdSetIterator>(docs, length);
}

IntArrayDocIdSet::IntArrayDocIdSetIterator::IntArrayDocIdSetIterator(
    std::deque<int> &docs, int length)
    : docs(docs), length(length)
{
}

int IntArrayDocIdSet::IntArrayDocIdSetIterator::docID() { return doc; }

int IntArrayDocIdSet::IntArrayDocIdSetIterator::nextDoc() 
{
  return doc = docs[++i];
}

int IntArrayDocIdSet::IntArrayDocIdSetIterator::advance(int target) throw(
    IOException)
{
  i = Arrays::binarySearch(docs, i + 1, length, target);
  if (i < 0) {
    i = -1 - i;
  }
  return doc = docs[i];
}

int64_t IntArrayDocIdSet::IntArrayDocIdSetIterator::cost() { return length; }
} // namespace org::apache::lucene::util