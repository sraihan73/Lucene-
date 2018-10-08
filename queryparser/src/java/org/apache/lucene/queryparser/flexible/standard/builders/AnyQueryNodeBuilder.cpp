using namespace std;

#include "AnyQueryNodeBuilder.h"

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
using AnyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::AnyQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TooManyClauses =
    org::apache::lucene::search::BooleanQuery::TooManyClauses;

AnyQueryNodeBuilder::AnyQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<BooleanQuery> AnyQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<AnyQueryNode> andNode =
      std::static_pointer_cast<AnyQueryNode>(queryNode);

  shared_ptr<BooleanQuery::Builder> bQuery =
      make_shared<BooleanQuery::Builder>();
  deque<std::shared_ptr<QueryNode>> children = andNode->getChildren();

  if (children.size() > 0) {

    for (auto child : children) {
      any obj = child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID);

      if (obj != nullptr) {
        shared_ptr<Query> query = any_cast<std::shared_ptr<Query>>(obj);

        try {
          bQuery->add(query, BooleanClause::Occur::SHOULD);
        } catch (const BooleanQuery::TooManyClauses &ex) {

          throw make_shared<QueryNodeException>(
              make_shared<MessageImpl>(QueryParserMessages::EMPTY_MESSAGE), ex);
        }
      }
    }
  }

  bQuery->setMinimumNumberShouldMatch(andNode->getMinimumMatchingElements());

  return bQuery->build();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders