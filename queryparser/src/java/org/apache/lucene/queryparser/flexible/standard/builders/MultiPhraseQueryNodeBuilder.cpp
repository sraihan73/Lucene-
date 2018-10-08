using namespace std;

#include "MultiPhraseQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using MultiPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::MultiPhraseQueryNode;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using TermQuery = org::apache::lucene::search::TermQuery;

MultiPhraseQueryNodeBuilder::MultiPhraseQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<MultiPhraseQuery> MultiPhraseQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<MultiPhraseQueryNode> phraseNode =
      std::static_pointer_cast<MultiPhraseQueryNode>(queryNode);

  shared_ptr<MultiPhraseQuery::Builder> phraseQueryBuilder =
      make_shared<MultiPhraseQuery::Builder>();

  deque<std::shared_ptr<QueryNode>> children = phraseNode->getChildren();

  if (children.size() > 0) {
    map_obj<int, deque<std::shared_ptr<Term>>> positionTermMap =
        map_obj<int, deque<std::shared_ptr<Term>>>();

    for (auto child : children) {
      shared_ptr<FieldQueryNode> termNode =
          std::static_pointer_cast<FieldQueryNode>(child);
      shared_ptr<TermQuery> termQuery = any_cast<std::shared_ptr<TermQuery>>(
          termNode->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));
      deque<std::shared_ptr<Term>> termList =
          positionTermMap[termNode->getPositionIncrement()];

      if (termList.empty()) {
        termList = deque<>();
        positionTermMap.emplace(termNode->getPositionIncrement(), termList);
      }

      termList.push_back(termQuery->getTerm());
    }

    for (auto positionIncrement : positionTermMap) {
      deque<std::shared_ptr<Term>> termList =
          positionTermMap[positionIncrement.first];

      phraseQueryBuilder->add(
          termList.toArray(std::deque<std::shared_ptr<Term>>(termList.size())),
          positionIncrement.first);
    }
  }

  return phraseQueryBuilder->build();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders