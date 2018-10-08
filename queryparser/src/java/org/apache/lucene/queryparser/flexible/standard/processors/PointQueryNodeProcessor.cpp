using namespace std;

#include "PointQueryNodeProcessor.h"

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
using RangeQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::RangeQueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using PointsConfig =
    org::apache::lucene::queryparser::flexible::standard::config::PointsConfig;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using PointQueryNode =
    org::apache::lucene::queryparser::flexible::standard::nodes::PointQueryNode;
using PointRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PointRangeQueryNode;

PointQueryNodeProcessor::PointQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> PointQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FieldQueryNode>(node) != nullptr &&
      !(std::dynamic_pointer_cast<RangeQueryNode>(node->getParent()) !=
        nullptr)) {

    shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();

    if (config != nullptr) {
      shared_ptr<FieldQueryNode> fieldNode =
          std::static_pointer_cast<FieldQueryNode>(node);
      shared_ptr<FieldConfig> fieldConfig =
          config->getFieldConfig(fieldNode->getFieldAsString());

      if (fieldConfig != nullptr) {
        shared_ptr<PointsConfig> numericConfig =
            fieldConfig->get(ConfigurationKeys::POINTS_CONFIG);

        if (numericConfig != nullptr) {

          shared_ptr<NumberFormat> numberFormat =
              numericConfig->getNumberFormat();
          wstring text = fieldNode->getTextAsString();
          shared_ptr<Number> number = nullptr;

          if (text.length() > 0) {

            try {
              number = numberFormat->parse(text);

            } catch (const ParseException &e) {
              throw make_shared<QueryNodeParseException>(
                  make_shared<MessageImpl>(
                      QueryParserMessages::COULD_NOT_PARSE_NUMBER,
                      fieldNode->getTextAsString(),
                      numberFormat->getClass().getCanonicalName()),
                  e);
            }

            if (Integer::typeid->equals(numericConfig->getType())) {
              number = number->intValue();
            } else if (Long::typeid->equals(numericConfig->getType())) {
              number = number->longValue();
            } else if (Double::typeid->equals(numericConfig->getType())) {
              number = number->doubleValue();
            } else if (Float::typeid->equals(numericConfig->getType())) {
              number = number->floatValue();
            }

          } else {
            throw make_shared<QueryNodeParseException>(make_shared<MessageImpl>(
                QueryParserMessages::NUMERIC_CANNOT_BE_EMPTY,
                fieldNode->getFieldAsString()));
          }

          shared_ptr<PointQueryNode> lowerNode = make_shared<PointQueryNode>(
              fieldNode->getField(), number, numberFormat);
          shared_ptr<PointQueryNode> upperNode = make_shared<PointQueryNode>(
              fieldNode->getField(), number, numberFormat);

          return make_shared<PointRangeQueryNode>(lowerNode, upperNode, true,
                                                  true, numericConfig);
        }
      }
    }
  }
  return node;
}

shared_ptr<QueryNode> PointQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  return node;
}

deque<std::shared_ptr<QueryNode>> PointQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors