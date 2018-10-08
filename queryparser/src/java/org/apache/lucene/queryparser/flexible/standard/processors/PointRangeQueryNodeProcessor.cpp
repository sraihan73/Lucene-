using namespace std;

#include "PointRangeQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNodeParseException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeParseException;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using PointQueryNode =
    org::apache::lucene::queryparser::flexible::standard::nodes::PointQueryNode;
using PointRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PointRangeQueryNode;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;

PointRangeQueryNodeProcessor::PointRangeQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> PointRangeQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<TermRangeQueryNode>(node) != nullptr) {
    shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();

    if (config != nullptr) {
      shared_ptr<TermRangeQueryNode> termRangeNode =
          std::static_pointer_cast<TermRangeQueryNode>(node);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      shared_ptr<FieldConfig> fieldConfig = config->getFieldConfig(
          StringUtils::toString(termRangeNode->getField()));

      if (fieldConfig != nullptr) {
        shared_ptr<PointsConfig> numericConfig =
            fieldConfig->get(ConfigurationKeys::POINTS_CONFIG);

        if (numericConfig != nullptr) {
          shared_ptr<FieldQueryNode> lower = termRangeNode->getLowerBound();
          shared_ptr<FieldQueryNode> upper = termRangeNode->getUpperBound();

          wstring lowerText = lower->getTextAsString();
          wstring upperText = upper->getTextAsString();
          shared_ptr<NumberFormat> numberFormat =
              numericConfig->getNumberFormat();
          shared_ptr<Number> lowerNumber = nullptr, upperNumber = nullptr;

          if (lowerText.length() > 0) {

            try {
              lowerNumber = numberFormat->parse(lowerText);

            } catch (const ParseException &e) {
              throw make_shared<QueryNodeParseException>(
                  make_shared<MessageImpl>(
                      QueryParserMessages::COULD_NOT_PARSE_NUMBER,
                      lower->getTextAsString(),
                      numberFormat->getClass().getCanonicalName()),
                  e);
            }
          }

          if (upperText.length() > 0) {

            try {
              upperNumber = numberFormat->parse(upperText);

            } catch (const ParseException &e) {
              throw make_shared<QueryNodeParseException>(
                  make_shared<MessageImpl>(
                      QueryParserMessages::COULD_NOT_PARSE_NUMBER,
                      upper->getTextAsString(),
                      numberFormat->getClass().getCanonicalName()),
                  e);
            }
          }

          if (Integer::typeid->equals(numericConfig->getType())) {
            if (upperNumber != nullptr) {
              upperNumber = upperNumber->intValue();
            }
            if (lowerNumber != nullptr) {
              lowerNumber = lowerNumber->intValue();
            }
          } else if (Long::typeid->equals(numericConfig->getType())) {
            if (upperNumber != nullptr) {
              upperNumber = upperNumber->longValue();
            }
            if (lowerNumber != nullptr) {
              lowerNumber = lowerNumber->longValue();
            }
          } else if (Double::typeid->equals(numericConfig->getType())) {
            if (upperNumber != nullptr) {
              upperNumber = upperNumber->doubleValue();
            }
            if (lowerNumber != nullptr) {
              lowerNumber = lowerNumber->doubleValue();
            }
          } else if (Float::typeid->equals(numericConfig->getType())) {
            if (upperNumber != nullptr) {
              upperNumber = upperNumber->floatValue();
            }
            if (lowerNumber != nullptr) {
              lowerNumber = lowerNumber->floatValue();
            }
          }

          shared_ptr<PointQueryNode> lowerNode = make_shared<PointQueryNode>(
              termRangeNode->getField(), lowerNumber, numberFormat);
          shared_ptr<PointQueryNode> upperNode = make_shared<PointQueryNode>(
              termRangeNode->getField(), upperNumber, numberFormat);

          bool lowerInclusive = termRangeNode->isLowerInclusive();
          bool upperInclusive = termRangeNode->isUpperInclusive();

          return make_shared<PointRangeQueryNode>(
              lowerNode, upperNode, lowerInclusive, upperInclusive,
              numericConfig);
        }
      }
    }
  }
  return node;
}

shared_ptr<QueryNode> PointRangeQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  return node;
}

deque<std::shared_ptr<QueryNode>>
PointRangeQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors