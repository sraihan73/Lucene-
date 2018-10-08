using namespace std;

#include "FieldBoostMapFCListener.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using FieldConfigListener = org::apache::lucene::queryparser::flexible::core::
    config::FieldConfigListener;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

FieldBoostMapFCListener::FieldBoostMapFCListener(
    shared_ptr<QueryConfigHandler> config)
{
  this->config = config;
}

void FieldBoostMapFCListener::buildFieldConfig(
    shared_ptr<FieldConfig> fieldConfig)
{
  unordered_map<wstring, float> fieldBoostMap =
      this->config->get(ConfigurationKeys::FIELD_BOOST_MAP);

  if (fieldBoostMap.size() > 0) {
    optional<float> boost = fieldBoostMap[fieldConfig->getField()];

    if (boost) {
      fieldConfig->set(ConfigurationKeys::BOOST, boost);
    }
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config