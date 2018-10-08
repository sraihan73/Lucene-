using namespace std;

#include "StandardQueryParser.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserHelper =
    org::apache::lucene::queryparser::flexible::core::QueryParserHelper;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using StandardQueryTreeBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryTreeBuilder;
using FuzzyConfig =
    org::apache::lucene::queryparser::flexible::standard::config::FuzzyConfig;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using StandardSyntaxParser = org::apache::lucene::queryparser::flexible::
    standard::parser::StandardSyntaxParser;
using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;

StandardQueryParser::StandardQueryParser()
    : org::apache::lucene::queryparser::flexible::core::QueryParserHelper(
          new StandardQueryConfigHandler(), new StandardSyntaxParser(),
          new StandardQueryNodeProcessorPipeline(nullptr),
          new StandardQueryTreeBuilder())
{
  setEnablePositionIncrements(true);
}

StandardQueryParser::StandardQueryParser(shared_ptr<Analyzer> analyzer)
    : StandardQueryParser()
{

  this->setAnalyzer(analyzer);
}

wstring StandardQueryParser::toString()
{
  return L"<StandardQueryParser config=\"" + this->getQueryConfigHandler() +
         L"\"/>";
}

shared_ptr<Query> StandardQueryParser::parse(
    const wstring &query, const wstring &defaultField) 
{

  return std::static_pointer_cast<Query>(
      QueryParserHelper::parse(query, defaultField));
}

StandardQueryConfigHandler::Operator StandardQueryParser::getDefaultOperator()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::DEFAULT_OPERATOR);
}

void StandardQueryParser::setDefaultOperator(
    StandardQueryConfigHandler::Operator operator_)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::DEFAULT_OPERATOR,
      operator_);
}

void StandardQueryParser::setAllowLeadingWildcard(bool allowLeadingWildcard)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::ALLOW_LEADING_WILDCARD,
      allowLeadingWildcard);
}

void StandardQueryParser::setEnablePositionIncrements(bool enabled)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::ENABLE_POSITION_INCREMENTS,
      enabled);
}

bool StandardQueryParser::getEnablePositionIncrements()
{
  optional<bool> enablePositionsIncrements = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::
          ENABLE_POSITION_INCREMENTS);

  if (!enablePositionsIncrements) {
    return false;

  } else {
    return enablePositionsIncrements;
  }
}

void StandardQueryParser::setMultiTermRewriteMethod(
    shared_ptr<MultiTermQuery::RewriteMethod> method)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::MULTI_TERM_REWRITE_METHOD,
      method);
}

shared_ptr<MultiTermQuery::RewriteMethod>
StandardQueryParser::getMultiTermRewriteMethod()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::MULTI_TERM_REWRITE_METHOD);
}

void StandardQueryParser::setMultiFields(
    std::deque<std::shared_ptr<std::wstring>> &fields)
{

  if (fields.empty()) {
    fields = std::deque<std::shared_ptr<std::wstring>>(0);
  }

  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::MULTI_FIELDS, fields);
}

std::deque<std::shared_ptr<std::wstring>> StandardQueryParser::getMultiFields()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::MULTI_FIELDS);
}

void StandardQueryParser::setFuzzyPrefixLength(int fuzzyPrefixLength)
{
  shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();
  shared_ptr<FuzzyConfig> fuzzyConfig =
      config->get(StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG);

  if (fuzzyConfig == nullptr) {
    fuzzyConfig = make_shared<FuzzyConfig>();
    config->set(StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG,
                fuzzyConfig);
  }

  fuzzyConfig->setPrefixLength(fuzzyPrefixLength);
}

void StandardQueryParser::setPointsConfigMap(
    unordered_map<wstring, std::shared_ptr<PointsConfig>> &pointsConfigMap)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::POINTS_CONFIG_MAP,
      pointsConfigMap);
}

unordered_map<wstring, std::shared_ptr<PointsConfig>>
StandardQueryParser::getPointsConfigMap()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::POINTS_CONFIG_MAP);
}

void StandardQueryParser::setLocale(shared_ptr<Locale> locale)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::LOCALE, locale);
}

shared_ptr<Locale> StandardQueryParser::getLocale()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::LOCALE);
}

void StandardQueryParser::setTimeZone(shared_ptr<TimeZone> timeZone)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::TIMEZONE, timeZone);
}

shared_ptr<TimeZone> StandardQueryParser::getTimeZone()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::TIMEZONE);
}

void StandardQueryParser::setPhraseSlop(int defaultPhraseSlop)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::PHRASE_SLOP,
      defaultPhraseSlop);
}

void StandardQueryParser::setAnalyzer(shared_ptr<Analyzer> analyzer)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::ANALYZER, analyzer);
}

shared_ptr<Analyzer> StandardQueryParser::getAnalyzer()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::ANALYZER);
}

bool StandardQueryParser::getAllowLeadingWildcard()
{
  optional<bool> allowLeadingWildcard = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::ALLOW_LEADING_WILDCARD);

  if (!allowLeadingWildcard) {
    return false;

  } else {
    return allowLeadingWildcard;
  }
}

float StandardQueryParser::getFuzzyMinSim()
{
  shared_ptr<FuzzyConfig> fuzzyConfig = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG);

  if (fuzzyConfig == nullptr) {
    return FuzzyQuery::defaultMinSimilarity;
  } else {
    return fuzzyConfig->getMinSimilarity();
  }
}

int StandardQueryParser::getFuzzyPrefixLength()
{
  shared_ptr<FuzzyConfig> fuzzyConfig = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG);

  if (fuzzyConfig == nullptr) {
    return FuzzyQuery::defaultPrefixLength;
  } else {
    return fuzzyConfig->getPrefixLength();
  }
}

int StandardQueryParser::getPhraseSlop()
{
  optional<int> phraseSlop = getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::PHRASE_SLOP);

  if (!phraseSlop) {
    return 0;

  } else {
    return phraseSlop;
  }
}

void StandardQueryParser::setFuzzyMinSim(float fuzzyMinSim)
{
  shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();
  shared_ptr<FuzzyConfig> fuzzyConfig =
      config->get(StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG);

  if (fuzzyConfig == nullptr) {
    fuzzyConfig = make_shared<FuzzyConfig>();
    config->set(StandardQueryConfigHandler::ConfigurationKeys::FUZZY_CONFIG,
                fuzzyConfig);
  }

  fuzzyConfig->setMinSimilarity(fuzzyMinSim);
}

void StandardQueryParser::setFieldsBoost(unordered_map<wstring, float> &boosts)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::FIELD_BOOST_MAP, boosts);
}

unordered_map<wstring, float> StandardQueryParser::getFieldsBoost()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::FIELD_BOOST_MAP);
}

void StandardQueryParser::setDateResolution(
    DateTools::Resolution dateResolution)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::DATE_RESOLUTION,
      dateResolution);
}

DateTools::Resolution StandardQueryParser::getDateResolution()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::DATE_RESOLUTION);
}

unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>
StandardQueryParser::getDateResolutionMap()
{
  return getQueryConfigHandler()->get(
      StandardQueryConfigHandler::ConfigurationKeys::FIELD_DATE_RESOLUTION_MAP);
}

void StandardQueryParser::setDateResolutionMap(
    unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>
        &dateRes)
{
  getQueryConfigHandler()->set(
      StandardQueryConfigHandler::ConfigurationKeys::FIELD_DATE_RESOLUTION_MAP,
      dateRes);
}
} // namespace org::apache::lucene::queryparser::flexible::standard