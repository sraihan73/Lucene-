using namespace std;

#include "FilterNumericDocValues.h"

namespace org::apache::lucene::index
{

FilterNumericDocValues::FilterNumericDocValues(shared_ptr<NumericDocValues> in_)
    : in_(in_)
{
}

int FilterNumericDocValues::docID() { return in_->docID(); }

int FilterNumericDocValues::nextDoc() 
{
  return in_->nextDoc();
}

int FilterNumericDocValues::advance(int target) 
{
  return in_->advance(target);
}

bool FilterNumericDocValues::advanceExact(int target) 
{
  return in_->advanceExact(target);
}

int64_t FilterNumericDocValues::cost() { return in_->cost(); }

int64_t FilterNumericDocValues::longValue() 
{
  return in_->longValue();
}
} // namespace org::apache::lucene::index