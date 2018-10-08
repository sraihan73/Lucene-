using namespace std;

#include "BooleanQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using EscapeQuerySyntaxImpl = org::apache::lucene::queryparser::flexible::
    standard::parser::EscapeQuerySyntaxImpl;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TooManyClauses =
    org::apache::lucene::search::BooleanQuery::TooManyClauses;

BooleanQueryNodeBuilder::BooleanQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<BooleanQuery> BooleanQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<BooleanQueryNode> booleanNode =
      std::static_pointer_cast<BooleanQueryNode>(queryNode);

  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();
  deque<std::shared_ptr<QueryNode>> children = booleanNode->getChildren();

  if (children.size() > 0) {

    for (auto child : children) {
      any obj = child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID);

      if (obj != nullptr) {
        shared_ptr<Query> query = any_cast<std::shared_ptr<Query>>(obj);

        try {
          bQuery->add(query, getModifierValue(child));

        } catch (const BooleanQuery::TooManyClauses &ex) {

          throw make_shared<QueryNodeException>(
              make_shared<MessageImpl>(
                  QueryParserMessages::TOO_MANY_BOOLEAN_CLAUSES,
                  BooleanQuery::getMaxClauseCount(),
                  queryNode->toQueryString(
                      make_shared<EscapeQuerySyntaxImpl>())),
              ex);
        }
      }
    }
  }

  return bQuery->build();
}

BooleanClause::Occur
BooleanQueryNodeBuilder::getModifierValue(shared_ptr<QueryNode> node)
{

  if (std::dynamic_pointer_cast<ModifierQueryNode>(node) != nullptr) {
    shared_ptr<ModifierQueryNode> mNode =
        (std::static_pointer_cast<ModifierQueryNode>(node));
    switch (mNode->getModifier()) {

    case MOD_REQ:
      return BooleanClause::Occur::MUST;

    case MOD_NOT:
      return BooleanClause::Occur::MUST_NOT;

    case MOD_NONE:
      return BooleanClause::Occur::SHOULD;
    }
  }

  return BooleanClause::Occur::SHOULD;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders