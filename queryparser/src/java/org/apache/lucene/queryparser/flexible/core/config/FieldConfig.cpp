using namespace std;

#include "FieldConfig.h"

namespace org::apache::lucene::queryparser::flexible::core::config
{

FieldConfig::FieldConfig(const wstring &fieldName)
{

  if (fieldName == L"") {
    throw invalid_argument(L"field name must not be null!");
  }

  this->fieldName = fieldName;
}

wstring FieldConfig::getField() { return this->fieldName; }

wstring FieldConfig::toString()
{
  return L"<fieldconfig name=\"" + this->fieldName +
         L"\" configurations=\""
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + AbstractQueryConfig::toString() + L"\"/>";
}
} // namespace org::apache::lucene::queryparser::flexible::core::config