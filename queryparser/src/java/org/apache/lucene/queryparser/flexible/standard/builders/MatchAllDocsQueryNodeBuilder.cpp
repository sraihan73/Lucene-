using namespace std;

#include "MatchAllDocsQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using MatchAllDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchAllDocsQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using EscapeQuerySyntaxImpl = org::apache::lucene::queryparser::flexible::
    standard::parser::EscapeQuerySyntaxImpl;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;

MatchAllDocsQueryNodeBuilder::MatchAllDocsQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<MatchAllDocsQuery> MatchAllDocsQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{

  // validates node
  if (!(std::dynamic_pointer_cast<MatchAllDocsQueryNode>(queryNode) !=
        nullptr)) {
    throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
        QueryParserMessages::LUCENE_QUERY_CONVERSION_ERROR,
        queryNode->toQueryString(make_shared<EscapeQuerySyntaxImpl>()),
        queryNode->getClassName()));
  }

  return make_shared<MatchAllDocsQuery>();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders