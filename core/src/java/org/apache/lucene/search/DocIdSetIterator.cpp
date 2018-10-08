using namespace std;

#include "DocIdSetIterator.h"

namespace org::apache::lucene::search
{

shared_ptr<DocIdSetIterator> DocIdSetIterator::empty()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>();
}

DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass()
{
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass::advance(int target)
{
  assert(!exhausted);
  assert(target >= 0);
  exhausted = true;
  return NO_MORE_DOCS;
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return exhausted ? NO_MORE_DOCS : -1;
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass::nextDoc()
{
  assert(!exhausted);
  exhausted = true;
  return NO_MORE_DOCS;
}

int64_t DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return 0;
}

shared_ptr<DocIdSetIterator> DocIdSetIterator::all(int maxDoc)
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass2>(maxDoc);
}

DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2::
    DocIdSetIteratorAnonymousInnerClass2(int maxDoc)
{
  this->maxDoc = maxDoc;
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2::docID()
{
  return doc;
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2::nextDoc() throw(
    IOException)
{
  return outerInstance->advance(doc + 1);
}

int DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2::advance(
    int target) 
{
  doc = target;
  if (doc >= maxDoc) {
    doc = NO_MORE_DOCS;
  }
  return doc;
}

int64_t DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2::cost()
{
  return maxDoc;
}

int DocIdSetIterator::slowAdvance(int target) 
{
  assert(docID() < target);
  int doc;
  do {
    doc = nextDoc();
  } while (doc < target);
  return doc;
}
} // namespace org::apache::lucene::search