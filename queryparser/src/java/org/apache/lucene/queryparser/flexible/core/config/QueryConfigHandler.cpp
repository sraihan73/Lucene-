using namespace std;

#include "QueryConfigHandler.h"

namespace org::apache::lucene::queryparser::flexible::core::config
{
using QueryNodeProcessor = org::apache::lucene::queryparser::flexible::core::
    processors::QueryNodeProcessor;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;

shared_ptr<FieldConfig>
QueryConfigHandler::getFieldConfig(const wstring &fieldName)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<FieldConfig> fieldConfig =
      make_shared<FieldConfig>(StringUtils::toString(fieldName));

  for (auto listener : this->listeners) {
    listener->buildFieldConfig(fieldConfig);
  }

  return fieldConfig;
}

void QueryConfigHandler::addFieldConfigListener(
    shared_ptr<FieldConfigListener> listener)
{
  this->listeners.push_back(listener);
}
} // namespace org::apache::lucene::queryparser::flexible::core::config