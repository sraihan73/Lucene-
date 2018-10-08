using namespace std;

#include "FieldQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using TermQuery = org::apache::lucene::search::TermQuery;

FieldQueryNodeBuilder::FieldQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<TermQuery> FieldQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<FieldQueryNode> fieldNode =
      std::static_pointer_cast<FieldQueryNode>(queryNode);

  return make_shared<TermQuery>(make_shared<Term>(
      fieldNode->getFieldAsString(), fieldNode->getTextAsString()));
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders