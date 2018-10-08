using namespace std;

#include "FieldCacheSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using ValueSource = org::apache::lucene::queries::function::ValueSource;

FieldCacheSource::FieldCacheSource(const wstring &field)
{
  this->field = field;
}

wstring FieldCacheSource::getField() { return field; }

wstring FieldCacheSource::description() { return field; }

bool FieldCacheSource::equals(any o)
{
  if (!(std::dynamic_pointer_cast<FieldCacheSource>(o) != nullptr)) {
    return false;
  }
  shared_ptr<FieldCacheSource> other =
      any_cast<std::shared_ptr<FieldCacheSource>>(o);
  return this->field == other->field;
}

int FieldCacheSource::hashCode() { return field.hashCode(); }
} // namespace org::apache::lucene::queries::function::valuesource