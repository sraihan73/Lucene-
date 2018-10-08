using namespace std;

#include "DoubleDocValuesField.h"

namespace org::apache::lucene::document
{

DoubleDocValuesField::DoubleDocValuesField(const wstring &name, double value)
    : NumericDocValuesField(name, Double::doubleToRawLongBits(value))
{
}

void DoubleDocValuesField::setDoubleValue(double value)
{
  NumericDocValuesField::setLongValue(Double::doubleToRawLongBits(value));
}

void DoubleDocValuesField::setLongValue(int64_t value)
{
  throw invalid_argument(L"cannot change value type from Double to Long");
}
} // namespace org::apache::lucene::document