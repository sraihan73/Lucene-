using namespace std;

#include "SlopQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SlopQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::SlopQueryNode;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;

SlopQueryNodeBuilder::SlopQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<Query> SlopQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<SlopQueryNode> phraseSlopNode =
      std::static_pointer_cast<SlopQueryNode>(queryNode);

  shared_ptr<Query> query =
      any_cast<std::shared_ptr<Query>>(phraseSlopNode->getChild()->getTag(
          QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID));

  if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    builder->setSlop(phraseSlopNode->getValue());
    shared_ptr<PhraseQuery> pq = std::static_pointer_cast<PhraseQuery>(query);
    std::deque<std::shared_ptr<org::apache::lucene::index::Term>> terms =
        pq->getTerms();
    std::deque<int> positions = pq->getPositions();
    for (int i = 0; i < terms.size(); ++i) {
      builder->add(terms[i], positions[i]);
    }
    query = builder->build();

  } else {
    shared_ptr<MultiPhraseQuery> mpq =
        std::static_pointer_cast<MultiPhraseQuery>(query);

    int slop = phraseSlopNode->getValue();

    if (slop != mpq->getSlop()) {
      query =
          (make_shared<MultiPhraseQuery::Builder>(mpq))->setSlop(slop)->build();
    }
  }

  return query;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders