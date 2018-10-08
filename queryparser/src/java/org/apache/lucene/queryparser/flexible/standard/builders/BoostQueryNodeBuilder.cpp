using namespace std;

#include "BoostQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using BoostQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BoostQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;

BoostQueryNodeBuilder::BoostQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> BoostQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<BoostQueryNode> boostNode =
      std::static_pointer_cast<BoostQueryNode>(queryNode);
  shared_ptr<QueryNode> child = boostNode->getChild();

  if (child == nullptr) {
    return nullptr;
  }

  shared_ptr<Query> query = any_cast<std::shared_ptr<Query>>(
      child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));

  return make_shared<BoostQuery>(query, boostNode->getValue());
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders