using namespace std;

#include "BoostQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using BoostQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BoostQueryNode;
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using StringUtils =
    org::apache::lucene::queryparser::flexible::core::util::StringUtils;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;

shared_ptr<QueryNode> BoostQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FieldableNode>(node) != nullptr &&
      (node->getParent() == nullptr ||
       !(std::dynamic_pointer_cast<FieldableNode>(node->getParent()) !=
         nullptr))) {

    shared_ptr<FieldableNode> fieldNode =
        std::static_pointer_cast<FieldableNode>(node);
    shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();

    if (config != nullptr) {
      shared_ptr<std::wstring> field = fieldNode->getField();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      shared_ptr<FieldConfig> fieldConfig =
          config->getFieldConfig(StringUtils::toString(field));

      if (fieldConfig != nullptr) {
        optional<float> boost = fieldConfig->get(ConfigurationKeys::BOOST);

        if (boost) {
          return make_shared<BoostQueryNode>(node, boost);
        }
      }
    }
  }

  return node;
}

shared_ptr<QueryNode> BoostQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>> BoostQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors