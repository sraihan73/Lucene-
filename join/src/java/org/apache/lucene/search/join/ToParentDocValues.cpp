using namespace std;

#include "ToParentDocValues.h"

namespace org::apache::lucene::search::join
{
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using ConjunctionDISI = org::apache::lucene::search::ConjunctionDISI;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Type = org::apache::lucene::search::join::BlockJoinSelector::Type;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;

ToParentDocValues::SortedDVs::SortedDVs(shared_ptr<SortedDocValues> values,
                                        BlockJoinSelector::Type selection,
                                        shared_ptr<BitSet> parents,
                                        shared_ptr<DocIdSetIterator> children)
    : values(values), selection(selection),
      iter(make_shared<ToParentDocValues>(values, parents, children,
                                          shared_from_this()))
{
}

int ToParentDocValues::SortedDVs::docID() { return iter->docID(); }

void ToParentDocValues::SortedDVs::reset() 
{
  ord = values->ordValue();
}

void ToParentDocValues::SortedDVs::increment() 
{
  if (selection == BlockJoinSelector::Type::MIN) {
    ord = min(ord, values->ordValue());
  } else if (selection == BlockJoinSelector::Type::MAX) {
    ord = max(ord, values->ordValue());
  } else {
    throw make_shared<AssertionError>();
  }
}

int ToParentDocValues::SortedDVs::nextDoc() 
{
  return iter->nextDoc();
}

int ToParentDocValues::SortedDVs::advance(int target) 
{
  return iter->advance(target);
}

bool ToParentDocValues::SortedDVs::advanceExact(int targetParentDocID) throw(
    IOException)
{
  return iter->advanceExact(targetParentDocID);
}

int ToParentDocValues::SortedDVs::ordValue() { return ord; }

shared_ptr<BytesRef>
ToParentDocValues::SortedDVs::lookupOrd(int ord) 
{
  return values->lookupOrd(ord);
}

int ToParentDocValues::SortedDVs::getValueCount()
{
  return values->getValueCount();
}

int64_t ToParentDocValues::SortedDVs::cost() { return values->cost(); }

ToParentDocValues::NumDV::NumDV(shared_ptr<NumericDocValues> values,
                                BlockJoinSelector::Type selection,
                                shared_ptr<BitSet> parents,
                                shared_ptr<DocIdSetIterator> children)
    : values(values), selection(selection),
      iter(make_shared<ToParentDocValues>(values, parents, children,
                                          shared_from_this()))
{
}

void ToParentDocValues::NumDV::reset() 
{
  value = values->longValue();
}

void ToParentDocValues::NumDV::increment() 
{
  switch (selection) {
  case Type::MIN:
    value = min(value, values->longValue());
    break;
  case Type::MAX:
    value = max(value, values->longValue());
    break;
  default:
    throw make_shared<AssertionError>();
  }
}

int ToParentDocValues::NumDV::nextDoc() 
{
  return iter->nextDoc();
}

int ToParentDocValues::NumDV::advance(int targetParentDocID) 
{
  return iter->advance(targetParentDocID);
}

bool ToParentDocValues::NumDV::advanceExact(int targetParentDocID) throw(
    IOException)
{
  return iter->advanceExact(targetParentDocID);
}

int64_t ToParentDocValues::NumDV::longValue() { return value; }

int ToParentDocValues::NumDV::docID() { return iter->docID(); }

int64_t ToParentDocValues::NumDV::cost() { return values->cost(); }

ToParentDocValues::ToParentDocValues(shared_ptr<DocIdSetIterator> values,
                                     shared_ptr<BitSet> parents,
                                     shared_ptr<DocIdSetIterator> children,
                                     shared_ptr<Accumulator> collect)
    : parents(parents), collector(collect)
{
  childWithValues =
      ConjunctionDISI::intersectIterators(Arrays::asList(children, values));
}

int ToParentDocValues::docID() { return docID_; }

int ToParentDocValues::nextDoc() 
{
  assert(docID_ != NO_MORE_DOCS);

  assert(childWithValues->docID() != docID_ || docID_ == -1);
  if (childWithValues->docID() < docID_ || docID_ == -1) {
    childWithValues->nextDoc();
  }
  if (childWithValues->docID() == NO_MORE_DOCS) {
    docID_ = NO_MORE_DOCS;
    return docID_;
  }

  assert(parents->get(childWithValues->docID()) == false);

  int nextParentDocID = parents->nextSetBit(childWithValues->docID());
  collector->reset();
  seen = true;

  while (true) {
    int childDocID = childWithValues->nextDoc();
    assert(childDocID != nextParentDocID);
    if (childDocID > nextParentDocID) {
      break;
    }
    collector->increment();
  }

  docID_ = nextParentDocID;

  return docID_;
}

int ToParentDocValues::advance(int target) 
{
  if (target >= parents->length()) {
    docID_ = NO_MORE_DOCS;
    return docID_;
  }
  if (target == 0) {
    assert(docID() == -1);
    return nextDoc();
  }
  int prevParentDocID = parents->prevSetBit(target - 1);
  if (childWithValues->docID() <= prevParentDocID) {
    childWithValues->advance(prevParentDocID + 1);
  }
  return nextDoc();
}

bool ToParentDocValues::advanceExact(int targetParentDocID) 
{
  if (targetParentDocID < docID_) {
    throw invalid_argument(
        L"target must be after the current document: current=" +
        to_wstring(docID_) + L" target=" + to_wstring(targetParentDocID));
  }
  int previousDocId = docID_;
  docID_ = targetParentDocID;
  if (targetParentDocID == previousDocId) {
    return seen; // ord != -1; rlly???
  }
  docID_ = targetParentDocID;
  seen = false;
  // ord = -1;
  if (parents->get(targetParentDocID) == false) {
    return false;
  }
  int prevParentDocId = docID_ == 0 ? -1 : parents->prevSetBit(docID_ - 1);
  int childDoc = childWithValues->docID();
  if (childDoc <= prevParentDocId) {
    childDoc = childWithValues->advance(prevParentDocId + 1);
  }
  if (childDoc >= docID_) {
    return false;
  }

  if (childWithValues->docID() < docID_) {
    collector->reset();
    seen = true;
    childWithValues->nextDoc();
  }

  if (seen == false) {
    return false;
  }

  for (int doc = childWithValues->docID(); doc < docID_;
       doc = childWithValues->nextDoc()) {
    collector->increment();
  }
  return true;
}

int64_t ToParentDocValues::cost() { return 0; }

shared_ptr<NumericDocValues>
ToParentDocValues::wrap(shared_ptr<NumericDocValues> values, Type selection,
                        shared_ptr<BitSet> parents2,
                        shared_ptr<DocIdSetIterator> children)
{
  return make_shared<ToParentDocValues::NumDV>(values, selection, parents2,
                                               children);
}

shared_ptr<SortedDocValues>
ToParentDocValues::wrap(shared_ptr<SortedDocValues> values, Type selection,
                        shared_ptr<BitSet> parents2,
                        shared_ptr<DocIdSetIterator> children)
{
  return make_shared<ToParentDocValues::SortedDVs>(values, selection, parents2,
                                                   children);
}
} // namespace org::apache::lucene::search::join