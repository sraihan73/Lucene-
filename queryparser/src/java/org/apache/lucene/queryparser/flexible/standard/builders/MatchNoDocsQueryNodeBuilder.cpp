using namespace std;

#include "MatchNoDocsQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using MatchNoDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchNoDocsQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using EscapeQuerySyntaxImpl = org::apache::lucene::queryparser::flexible::
    standard::parser::EscapeQuerySyntaxImpl;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;

MatchNoDocsQueryNodeBuilder::MatchNoDocsQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<MatchNoDocsQuery> MatchNoDocsQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{

  // validates node
  if (!(std::dynamic_pointer_cast<MatchNoDocsQueryNode>(queryNode) !=
        nullptr)) {
    throw make_shared<QueryNodeException>(make_shared<MessageImpl>(
        QueryParserMessages::LUCENE_QUERY_CONVERSION_ERROR,
        queryNode->toQueryString(make_shared<EscapeQuerySyntaxImpl>()),
        queryNode->getClassName()));
  }

  return make_shared<MatchNoDocsQuery>();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders