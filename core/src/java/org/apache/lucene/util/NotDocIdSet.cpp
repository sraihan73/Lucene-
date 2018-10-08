using namespace std;

#include "NotDocIdSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

NotDocIdSet::NotDocIdSet(int maxDoc, shared_ptr<DocIdSet> in_)
    : maxDoc(maxDoc), in_(in_)
{
}

shared_ptr<Bits> NotDocIdSet::bits() 
{
  shared_ptr<Bits> *const inBits = in_->bits();
  if (inBits == nullptr) {
    return nullptr;
  }
  return make_shared<BitsAnonymousInnerClass>(shared_from_this(), inBits);
}

NotDocIdSet::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<NotDocIdSet> outerInstance,
    shared_ptr<org::apache::lucene::util::Bits> inBits)
{
  this->outerInstance = outerInstance;
  this->inBits = inBits;
}

bool NotDocIdSet::BitsAnonymousInnerClass::get(int index)
{
  return !inBits->get(index);
}

int NotDocIdSet::BitsAnonymousInnerClass::length() { return inBits->length(); }

int64_t NotDocIdSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + in_->ramBytesUsed();
}

shared_ptr<DocIdSetIterator> NotDocIdSet::iterator() 
{
  shared_ptr<DocIdSetIterator> *const inIterator = in_->begin();
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this(),
                                                          inIterator);
}

NotDocIdSet::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(shared_ptr<NotDocIdSet> outerInstance,
                                        shared_ptr<DocIdSetIterator> inIterator)
{
  this->outerInstance = outerInstance;
  this->inIterator = inIterator;
  doc = -1;
  nextSkippedDoc = -1;
}

int NotDocIdSet::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return advance(doc + 1);
}

int NotDocIdSet::DocIdSetIteratorAnonymousInnerClass::advance(int target) throw(
    IOException)
{
  doc = target;
  if (doc > nextSkippedDoc) {
    nextSkippedDoc = inIterator->advance(doc);
  }
  while (true) {
    if (doc >= outerInstance->maxDoc) {
      return doc = DocIdSetIterator::NO_MORE_DOCS;
    }
    assert(doc <= nextSkippedDoc);
    if (doc != nextSkippedDoc) {
      return doc;
    }
    doc += 1;
    nextSkippedDoc = inIterator->nextDoc();
  }
}

int NotDocIdSet::DocIdSetIteratorAnonymousInnerClass::docID() { return doc; }

int64_t NotDocIdSet::DocIdSetIteratorAnonymousInnerClass::cost()
{
  // even if there are few docs in this set, iterating over all documents
  // costs O(maxDoc) in all cases
  return outerInstance->maxDoc;
}
} // namespace org::apache::lucene::util