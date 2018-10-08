using namespace std;

#include "FloatDocValuesField.h"

namespace org::apache::lucene::document
{

FloatDocValuesField::FloatDocValuesField(const wstring &name, float value)
    : NumericDocValuesField(name, Float::floatToRawIntBits(value))
{
}

void FloatDocValuesField::setFloatValue(float value)
{
  NumericDocValuesField::setLongValue(Float::floatToRawIntBits(value));
}

void FloatDocValuesField::setLongValue(int64_t value)
{
  throw invalid_argument(L"cannot change value type from Float to Long");
}
} // namespace org::apache::lucene::document