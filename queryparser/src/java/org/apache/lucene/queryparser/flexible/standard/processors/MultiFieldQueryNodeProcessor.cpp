using namespace std;

#include "MultiFieldQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using OrQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::OrQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;

MultiFieldQueryNodeProcessor::MultiFieldQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> MultiFieldQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

void MultiFieldQueryNodeProcessor::processChildren(
    shared_ptr<QueryNode> queryTree) 
{

  if (this->processChildren_) {
    QueryNodeProcessorImpl::processChildren(queryTree);

  } else {
    this->processChildren_ = true;
  }
}

shared_ptr<QueryNode> MultiFieldQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FieldableNode>(node) != nullptr) {
    this->processChildren_ = false;
    shared_ptr<FieldableNode> fieldNode =
        std::static_pointer_cast<FieldableNode>(node);

    if (fieldNode->getField() == nullptr) {
      std::deque<std::shared_ptr<std::wstring>> fields =
          getQueryConfigHandler()->get(ConfigurationKeys::MULTI_FIELDS);

      if (fields.empty()) {
        throw invalid_argument(
            L"StandardQueryConfigHandler.ConfigurationKeys.MULTI_FIELDS should "
            L"be set on the QueryConfigHandler");
      }

      if (fields.size() > 0 && fields.size() > 0) {
        fieldNode->setField(fields[0]);

        if (fields.size() == 1) {
          return fieldNode;
        } else {
          deque<std::shared_ptr<QueryNode>> children =
              deque<std::shared_ptr<QueryNode>>(fields.size());

          children.push_back(fieldNode);
          for (int i = 1; i < fields.size(); i++) {
            try {
              fieldNode = std::static_pointer_cast<FieldableNode>(
                  fieldNode->cloneTree());
              fieldNode->setField(fields[i]);

              children.push_back(fieldNode);
            } catch (const CloneNotSupportedException &e) {
              throw runtime_error(e);
            }
          }

          return make_shared<GroupQueryNode>(
              make_shared<OrQueryNode>(children));
        }
      }
    }
  }

  return node;
}

deque<std::shared_ptr<QueryNode>>
MultiFieldQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors