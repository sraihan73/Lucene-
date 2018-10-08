using namespace std;

#include "UniqueFieldQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

shared_ptr<QueryNode> UniqueFieldQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

shared_ptr<QueryNode> UniqueFieldQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FieldableNode>(node) != nullptr) {
    shared_ptr<FieldableNode> fieldNode =
        std::static_pointer_cast<FieldableNode>(node);

    shared_ptr<QueryConfigHandler> queryConfig = getQueryConfigHandler();

    if (queryConfig == nullptr) {
      throw invalid_argument(L"A config handler is expected by the processor "
                             L"UniqueFieldQueryNodeProcessor!");
    }

    if (!queryConfig->has(SpansQueryConfigHandler::UNIQUE_FIELD)) {
      throw invalid_argument(
          L"UniqueFieldAttribute should be defined in the config handler!");
    }

    wstring uniqueField =
        queryConfig->get(SpansQueryConfigHandler::UNIQUE_FIELD);
    fieldNode->setField(uniqueField);
  }

  return node;
}

deque<std::shared_ptr<QueryNode>>
UniqueFieldQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::spans