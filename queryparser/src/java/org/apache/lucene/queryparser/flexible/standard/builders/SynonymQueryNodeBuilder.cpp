using namespace std;

#include "SynonymQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SynonymQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::SynonymQueryNode;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;

SynonymQueryNodeBuilder::SynonymQueryNodeBuilder() {}

shared_ptr<Query> SynonymQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  // TODO: use SynonymQuery instead
  shared_ptr<SynonymQueryNode> node =
      std::static_pointer_cast<SynonymQueryNode>(queryNode);
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  for (auto child : node->getChildren()) {
    any obj = child->getTag(QueryTreeBuilder::QUERY_TREE_BUILDER_TAGID);

    if (obj != nullptr) {
      shared_ptr<Query> query = any_cast<std::shared_ptr<Query>>(obj);
      builder->add(query, Occur::SHOULD);
    }
  }
  return builder->build();
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders