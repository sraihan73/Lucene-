using namespace std;

#include "LabelAndValue.h"

namespace org::apache::lucene::facet
{

LabelAndValue::LabelAndValue(const wstring &label, shared_ptr<Number> value)
    : label(label), value(value)
{
}

wstring LabelAndValue::toString() { return label + L" (" + value + L")"; }

bool LabelAndValue::equals(any _other)
{
  if ((std::dynamic_pointer_cast<LabelAndValue>(_other) != nullptr) == false) {
    return false;
  }
  shared_ptr<LabelAndValue> other =
      any_cast<std::shared_ptr<LabelAndValue>>(_other);
  return label == other->label && value->equals(other->value);
}

int LabelAndValue::hashCode()
{
  return label.hashCode() + 1439 * value->hashCode();
}
} // namespace org::apache::lucene::facet