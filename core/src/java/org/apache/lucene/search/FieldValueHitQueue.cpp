using namespace std;

#include "FieldValueHitQueue.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

FieldValueHitQueue<T>::Entry::Entry(int slot, int doc, float score)
    : ScoreDoc(doc, score)
{
  this->slot = slot;
}

wstring FieldValueHitQueue<T>::Entry::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"slot:" + to_wstring(slot) + L" " + ScoreDoc::toString();
}
} // namespace org::apache::lucene::search