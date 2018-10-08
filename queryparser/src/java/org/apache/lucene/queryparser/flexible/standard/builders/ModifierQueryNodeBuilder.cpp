using namespace std;

#include "ModifierQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Query = org::apache::lucene::search::Query;

ModifierQueryNodeBuilder::ModifierQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> ModifierQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<ModifierQueryNode> modifierNode =
      std::static_pointer_cast<ModifierQueryNode>(queryNode);

  return any_cast<std::shared_ptr<Query>>(
      (modifierNode)
          ->getChild()
          ->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders