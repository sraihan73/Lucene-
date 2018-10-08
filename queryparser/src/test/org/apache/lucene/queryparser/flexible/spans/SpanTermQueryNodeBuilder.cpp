using namespace std;

#include "SpanTermQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using StandardQueryBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryBuilder;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

shared_ptr<SpanTermQuery> SpanTermQueryNodeBuilder::build(
    shared_ptr<QueryNode> node) 
{
  shared_ptr<FieldQueryNode> fieldQueryNode =
      std::static_pointer_cast<FieldQueryNode>(node);

  return make_shared<SpanTermQuery>(make_shared<Term>(
      fieldQueryNode->getFieldAsString(), fieldQueryNode->getTextAsString()));
}
} // namespace org::apache::lucene::queryparser::flexible::spans