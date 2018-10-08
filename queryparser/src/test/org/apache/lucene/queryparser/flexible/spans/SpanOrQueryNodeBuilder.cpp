using namespace std;

#include "SpanOrQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StandardQueryBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryBuilder;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

shared_ptr<SpanOrQuery> SpanOrQueryNodeBuilder::build(
    shared_ptr<QueryNode> node) 
{

  // validates node
  shared_ptr<BooleanQueryNode> booleanNode =
      std::static_pointer_cast<BooleanQueryNode>(node);

  deque<std::shared_ptr<QueryNode>> children = booleanNode->getChildren();
  std::deque<std::shared_ptr<SpanQuery>> spanQueries(children.size());

  int i = 0;
  for (auto child : children) {
    spanQueries[i++] = any_cast<std::shared_ptr<SpanQuery>>(
        child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));
  }

  return make_shared<SpanOrQuery>(spanQueries);
}
} // namespace org::apache::lucene::queryparser::flexible::spans