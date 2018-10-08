using namespace std;

#include "LongValues.h"

namespace org::apache::lucene::util
{

const shared_ptr<LongValues> LongValues::IDENTITY =
    make_shared<LongValuesAnonymousInnerClass>();

LongValues::LongValuesAnonymousInnerClass::LongValuesAnonymousInnerClass() {}

int64_t LongValues::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return index;
}

const shared_ptr<LongValues> LongValues::ZEROES =
    make_shared<LongValuesAnonymousInnerClass2>();

LongValues::LongValuesAnonymousInnerClass2::LongValuesAnonymousInnerClass2() {}

int64_t LongValues::LongValuesAnonymousInnerClass2::get(int64_t index)
{
  return 0;
}
} // namespace org::apache::lucene::util