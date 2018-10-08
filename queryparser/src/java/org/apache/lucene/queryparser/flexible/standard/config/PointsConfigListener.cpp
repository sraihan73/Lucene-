using namespace std;

#include "PointsConfigListener.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using FieldConfigListener = org::apache::lucene::queryparser::flexible::core::
    config::FieldConfigListener;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;

PointsConfigListener::PointsConfigListener(
    shared_ptr<QueryConfigHandler> config)
    : config(config)
{
  if (config == nullptr) {
    throw invalid_argument(L"config must not be null!");
  }
}

void PointsConfigListener::buildFieldConfig(shared_ptr<FieldConfig> fieldConfig)
{
  unordered_map<wstring, std::shared_ptr<PointsConfig>> pointsConfigMap =
      config->get(ConfigurationKeys::POINTS_CONFIG_MAP);

  if (pointsConfigMap.size() > 0) {
    shared_ptr<PointsConfig> pointsConfig =
        pointsConfigMap[fieldConfig->getField()];

    if (pointsConfig != nullptr) {
      fieldConfig->set(ConfigurationKeys::POINTS_CONFIG, pointsConfig);
    }
  }
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config