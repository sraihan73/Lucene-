using namespace std;

#include "FilteredDocIdSetIterator.h"

namespace org::apache::lucene::search
{

FilteredDocIdSetIterator::FilteredDocIdSetIterator(
    shared_ptr<DocIdSetIterator> innerIter)
{
  if (innerIter == nullptr) {
    throw invalid_argument(L"null iterator");
  }
  _innerIter = innerIter;
  doc = -1;
}

shared_ptr<DocIdSetIterator> FilteredDocIdSetIterator::getDelegate()
{
  return _innerIter;
}

int FilteredDocIdSetIterator::docID() { return doc; }

int FilteredDocIdSetIterator::nextDoc() 
{
  while ((doc = _innerIter->nextDoc()) != NO_MORE_DOCS) {
    if (match(doc)) {
      return doc;
    }
  }
  return doc;
}

int FilteredDocIdSetIterator::advance(int target) 
{
  doc = _innerIter->advance(target);
  if (doc != NO_MORE_DOCS) {
    if (match(doc)) {
      return doc;
    } else {
      while ((doc = _innerIter->nextDoc()) != NO_MORE_DOCS) {
        if (match(doc)) {
          return doc;
        }
      }
      return doc;
    }
  }
  return doc;
}

int64_t FilteredDocIdSetIterator::cost() { return _innerIter->cost(); }
} // namespace org::apache::lucene::search