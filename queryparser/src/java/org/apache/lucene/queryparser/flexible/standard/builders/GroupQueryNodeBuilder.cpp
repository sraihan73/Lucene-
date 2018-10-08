using namespace std;

#include "GroupQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using Query = org::apache::lucene::search::Query;

GroupQueryNodeBuilder::GroupQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> GroupQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<GroupQueryNode> groupNode =
      std::static_pointer_cast<GroupQueryNode>(queryNode);

  return any_cast<std::shared_ptr<Query>>((groupNode)->getChild()->getTag(
      QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders