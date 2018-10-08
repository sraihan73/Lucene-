using namespace std;

#include "StandardQueryConfigHandler.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using ConfigurationKey =
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using StandardQueryParser =
    org::apache::lucene::queryparser::flexible::standard::StandardQueryParser;
using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using RewriteMethod =
    org::apache::lucene::search::MultiTermQuery::RewriteMethod;
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<bool>>
    StandardQueryConfigHandler::ConfigurationKeys::ENABLE_POSITION_INCREMENTS =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<bool>>
    StandardQueryConfigHandler::ConfigurationKeys::ALLOW_LEADING_WILDCARD =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        std::shared_ptr<org::apache::lucene::analysis::Analyzer>>>
    StandardQueryConfigHandler::ConfigurationKeys::ANALYZER =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<Operator>>
    StandardQueryConfigHandler::ConfigurationKeys::DEFAULT_OPERATOR =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<int>>
    StandardQueryConfigHandler::ConfigurationKeys::PHRASE_SLOP =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<std::shared_ptr<java::util::Locale>>>
    StandardQueryConfigHandler::ConfigurationKeys::LOCALE =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<std::shared_ptr<java::util::TimeZone>>>
    StandardQueryConfigHandler::ConfigurationKeys::TIMEZONE =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        std::shared_ptr<
            org::apache::lucene::search::MultiTermQuery::RewriteMethod>>>
    StandardQueryConfigHandler::ConfigurationKeys::MULTI_TERM_REWRITE_METHOD =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        std::deque<std::shared_ptr<std::wstring>>>>
    StandardQueryConfigHandler::ConfigurationKeys::MULTI_FIELDS =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<unordered_map<wstring, float>>>
    StandardQueryConfigHandler::ConfigurationKeys::FIELD_BOOST_MAP =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        unordered_map<std::shared_ptr<std::wstring>,
                      org::apache::lucene::document::DateTools::Resolution>>>
    StandardQueryConfigHandler::ConfigurationKeys::FIELD_DATE_RESOLUTION_MAP =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<std::shared_ptr<FuzzyConfig>>>
    StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        org::apache::lucene::document::DateTools::Resolution>>
    StandardQueryConfigHandler::ConfigurationKeys::DATE_RESOLUTION =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<float>>
    StandardQueryConfigHandler::ConfigurationKeys::BOOST = org::apache::lucene::
        queryparser::flexible::core::config::ConfigurationKey::newInstance();
const shared_ptr<org::apache::lucene::queryparser::flexible::core::config::
                     ConfigurationKey<std::shared_ptr<PointsConfig>>>
    StandardQueryConfigHandler::ConfigurationKeys::POINTS_CONFIG =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();
const shared_ptr<
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey<
        unordered_map<wstring, std::shared_ptr<PointsConfig>>>>
    StandardQueryConfigHandler::ConfigurationKeys::POINTS_CONFIG_MAP =
        org::apache::lucene::queryparser::flexible::core::config::
            ConfigurationKey::newInstance();

StandardQueryConfigHandler::StandardQueryConfigHandler()
{
  // Add listener that will build the FieldConfig.
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  addFieldConfigListener(
      make_shared<FieldBoostMapFCListener>(shared_from_this()));
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  addFieldConfigListener(
      make_shared<FieldDateResolutionFCListener>(shared_from_this()));
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  addFieldConfigListener(make_shared<PointsConfigListener>(shared_from_this()));

  // Default Values
  set(ConfigurationKeys::ALLOW_LEADING_WILDCARD, false); // default in 2.9
  set(ConfigurationKeys::ANALYZER, nullptr);             // default value 2.4
  set(ConfigurationKeys::DEFAULT_OPERATOR, Operator::OR);
  set(ConfigurationKeys::PHRASE_SLOP, 0); // default value 2.4
  set(ConfigurationKeys::ENABLE_POSITION_INCREMENTS, false); // default
                                                             // value 2.4
  set(ConfigurationKeys::FIELD_BOOST_MAP,
      make_shared<LinkedHashMap<wstring, float>>());
  set(ConfigurationKeys::FUZZY_CONFIG, make_shared<FuzzyConfig>());
  set(ConfigurationKeys::LOCALE, Locale::getDefault());
  set(ConfigurationKeys::MULTI_TERM_REWRITE_METHOD,
      MultiTermQuery::CONSTANT_SCORE_REWRITE);
  set(ConfigurationKeys::FIELD_DATE_RESOLUTION_MAP,
      unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>());
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config