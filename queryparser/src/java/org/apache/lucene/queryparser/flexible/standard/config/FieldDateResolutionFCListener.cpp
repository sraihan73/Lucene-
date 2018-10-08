using namespace std;

#include "FieldDateResolutionFCListener.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using FieldConfigListener = org::apache::lucene::queryparser::flexible::core::
    config::FieldConfigListener;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

FieldDateResolutionFCListener::FieldDateResolutionFCListener(
    shared_ptr<QueryConfigHandler> config)
{
  this->config = config;
}

void FieldDateResolutionFCListener::buildFieldConfig(
    shared_ptr<FieldConfig> fieldConfig)
{
  DateTools::Resolution dateRes = nullptr;
  unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>
      dateResMap =
          this->config->get(ConfigurationKeys::FIELD_DATE_RESOLUTION_MAP);

  if (dateResMap.size() > 0) {
    dateRes = dateResMap[fieldConfig->getField()];
  }

  if (dateRes == nullptr) {
    dateRes = this->config->get(ConfigurationKeys::DATE_RESOLUTION);
  }

  if (dateRes != nullptr) {
    fieldConfig->set(ConfigurationKeys::DATE_RESOLUTION, dateRes);
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config