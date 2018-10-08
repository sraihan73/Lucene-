using namespace std;

#include "DoubleValues.h"

namespace org::apache::lucene::search
{

shared_ptr<DoubleValues> DoubleValues::withDefault(shared_ptr<DoubleValues> in_,
                                                   double missingValue)
{
  return make_shared<DoubleValuesAnonymousInnerClass>(in_, missingValue);
}

DoubleValues::DoubleValuesAnonymousInnerClass::DoubleValuesAnonymousInnerClass(
    shared_ptr<org::apache::lucene::search::DoubleValues> in_,
    double missingValue)
{
  this->in_ = in_;
  this->missingValue = missingValue;
}

double
DoubleValues::DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return hasValue ? in_->doubleValue() : missingValue;
}

bool DoubleValues::DoubleValuesAnonymousInnerClass::advanceExact(int doc) throw(
    IOException)
{
  hasValue = in_->advanceExact(doc);
  return true;
}

const shared_ptr<DoubleValues> DoubleValues::EMPTY =
    make_shared<DoubleValuesAnonymousInnerClass2>();

DoubleValues::DoubleValuesAnonymousInnerClass2::
    DoubleValuesAnonymousInnerClass2()
{
}

double
DoubleValues::DoubleValuesAnonymousInnerClass2::doubleValue() 
{
  throw make_shared<UnsupportedOperationException>();
}

bool DoubleValues::DoubleValuesAnonymousInnerClass2::advanceExact(
    int doc) 
{
  return false;
}
} // namespace org::apache::lucene::search