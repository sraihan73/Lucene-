using namespace std;

#include "SpansQueryTreeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StandardQueryBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryBuilder;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

SpansQueryTreeBuilder::SpansQueryTreeBuilder()
{
  setBuilder(BooleanQueryNode::typeid, make_shared<SpanOrQueryNodeBuilder>());
  setBuilder(FieldQueryNode::typeid, make_shared<SpanTermQueryNodeBuilder>());
}

shared_ptr<SpanQuery> SpansQueryTreeBuilder::build(
    shared_ptr<QueryNode> queryTree) 
{
  return std::static_pointer_cast<SpanQuery>(
      QueryTreeBuilder::build(queryTree));
}
} // namespace org::apache::lucene::queryparser::flexible::spans