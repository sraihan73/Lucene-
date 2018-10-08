using namespace std;

#include "ReaderUtil.h"

namespace org::apache::lucene::index
{

ReaderUtil::ReaderUtil() {} // no instance

shared_ptr<IndexReaderContext>
ReaderUtil::getTopLevelContext(shared_ptr<IndexReaderContext> context)
{
  while (context->parent != nullptr) {
    context = context->parent;
  }
  return context;
}

int ReaderUtil::subIndex(int n, std::deque<int> &docStarts)
{ // find
  // searcher/reader for doc n:
  int size = docStarts.size();
  int lo = 0;        // search starts array
  int hi = size - 1; // for first element less than n, return its index
  while (hi >= lo) {
    int mid = static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    int midValue = docStarts[mid];
    if (n < midValue) {
      hi = mid - 1;
    } else if (n > midValue) {
      lo = mid + 1;
    } else { // found a match
      while (mid + 1 < size && docStarts[mid + 1] == midValue) {
        mid++; // scan to last match
      }
      return mid;
    }
  }
  return hi;
}

int ReaderUtil::subIndex(int n,
                         deque<std::shared_ptr<LeafReaderContext>> &leaves)
{ // find
  // searcher/reader for doc n:
  int size = leaves.size();
  int lo = 0;        // search starts array
  int hi = size - 1; // for first element less than n, return its index
  while (hi >= lo) {
    int mid = static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    int midValue = leaves[mid]->docBase;
    if (n < midValue) {
      hi = mid - 1;
    } else if (n > midValue) {
      lo = mid + 1;
    } else { // found a match
      while (mid + 1 < size && leaves[mid + 1]->docBase == midValue) {
        mid++; // scan to last match
      }
      return mid;
    }
  }
  return hi;
}
} // namespace org::apache::lucene::index