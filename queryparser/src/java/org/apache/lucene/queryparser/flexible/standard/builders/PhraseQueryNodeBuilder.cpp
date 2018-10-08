using namespace std;

#include "PhraseQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using TokenizedPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::TokenizedPhraseQueryNode;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

PhraseQueryNodeBuilder::PhraseQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> PhraseQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<TokenizedPhraseQueryNode> phraseNode =
      std::static_pointer_cast<TokenizedPhraseQueryNode>(queryNode);

  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();

  deque<std::shared_ptr<QueryNode>> children = phraseNode->getChildren();

  if (children.size() > 0) {

    for (auto child : children) {
      shared_ptr<TermQuery> termQuery = any_cast<std::shared_ptr<TermQuery>>(
          child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));
      shared_ptr<FieldQueryNode> termNode =
          std::static_pointer_cast<FieldQueryNode>(child);

      builder->add(termQuery->getTerm(), termNode->getPositionIncrement());
    }
  }

  return builder->build();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders