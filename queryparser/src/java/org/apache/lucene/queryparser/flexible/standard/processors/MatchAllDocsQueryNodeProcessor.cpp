using namespace std;

#include "MatchAllDocsQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using MatchAllDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchAllDocsQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;

MatchAllDocsQueryNodeProcessor::MatchAllDocsQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> MatchAllDocsQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FieldQueryNode>(node) != nullptr) {
    shared_ptr<FieldQueryNode> fqn =
        std::static_pointer_cast<FieldQueryNode>(node);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (fqn->getField()->toString()->equals(L"*") &&
        fqn->getText()->toString()->equals(L"*")) {

      return make_shared<MatchAllDocsQueryNode>();
    }
  }

  return node;
}

shared_ptr<QueryNode> MatchAllDocsQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>>
MatchAllDocsQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors