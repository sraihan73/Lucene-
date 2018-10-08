using namespace std;

#include "SingletonSortedNumericDocValues.h"

namespace org::apache::lucene::index
{

SingletonSortedNumericDocValues::SingletonSortedNumericDocValues(
    shared_ptr<NumericDocValues> in_)
    : in_(in_)
{
  if (in_->docID() != -1) {
    throw make_shared<IllegalStateException>(
        L"iterator has already been used: docID=" + to_wstring(in_->docID()));
  }
}

shared_ptr<NumericDocValues>
SingletonSortedNumericDocValues::getNumericDocValues()
{
  if (in_->docID() != -1) {
    throw make_shared<IllegalStateException>(
        L"iterator has already been used: docID=" + to_wstring(in_->docID()));
  }
  return in_;
}

int SingletonSortedNumericDocValues::docID() { return in_->docID(); }

int SingletonSortedNumericDocValues::nextDoc() 
{
  return in_->nextDoc();
}

int SingletonSortedNumericDocValues::advance(int target) 
{
  return in_->advance(target);
}

bool SingletonSortedNumericDocValues::advanceExact(int target) throw(
    IOException)
{
  return in_->advanceExact(target);
}

int64_t SingletonSortedNumericDocValues::cost() { return in_->cost(); }

int64_t SingletonSortedNumericDocValues::nextValue() 
{
  return in_->longValue();
}

int SingletonSortedNumericDocValues::docValueCount() { return 1; }
} // namespace org::apache::lucene::index