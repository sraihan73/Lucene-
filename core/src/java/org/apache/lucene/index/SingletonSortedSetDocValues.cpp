using namespace std;

#include "SingletonSortedSetDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

SingletonSortedSetDocValues::SingletonSortedSetDocValues(
    shared_ptr<SortedDocValues> in_)
    : in_(in_)
{
  if (in_->docID() != -1) {
    throw make_shared<IllegalStateException>(
        L"iterator has already been used: docID=" + to_wstring(in_->docID()));
  }
}

shared_ptr<SortedDocValues> SingletonSortedSetDocValues::getSortedDocValues()
{
  if (in_->docID() != -1) {
    throw make_shared<IllegalStateException>(
        L"iterator has already been used: docID=" + to_wstring(in_->docID()));
  }
  return in_;
}

int SingletonSortedSetDocValues::docID() { return in_->docID(); }

int64_t SingletonSortedSetDocValues::nextOrd()
{
  int64_t v = ord;
  ord = NO_MORE_ORDS;
  return v;
}

int SingletonSortedSetDocValues::nextDoc() 
{
  int docID = in_->nextDoc();
  if (docID != NO_MORE_DOCS) {
    ord = in_->ordValue();
  }
  return docID;
}

int SingletonSortedSetDocValues::advance(int target) 
{
  int docID = in_->advance(target);
  if (docID != NO_MORE_DOCS) {
    ord = in_->ordValue();
  }
  return docID;
}

bool SingletonSortedSetDocValues::advanceExact(int target) 
{
  if (in_->advanceExact(target)) {
    ord = in_->ordValue();
    return true;
  }
  return false;
}

shared_ptr<BytesRef>
SingletonSortedSetDocValues::lookupOrd(int64_t ord) 
{
  // cast is ok: single-valued cannot exceed Integer.MAX_VALUE
  return in_->lookupOrd(static_cast<int>(ord));
}

int64_t SingletonSortedSetDocValues::getValueCount()
{
  return in_->getValueCount();
}

int64_t SingletonSortedSetDocValues::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  return in_->lookupTerm(key);
}

shared_ptr<TermsEnum>
SingletonSortedSetDocValues::termsEnum() 
{
  return in_->termsEnum();
}

int64_t SingletonSortedSetDocValues::cost() { return in_->cost(); }
} // namespace org::apache::lucene::index