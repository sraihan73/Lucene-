using namespace std;

#include "StandardQueryTreeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using BoostQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BoostQueryNode;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using MatchAllDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchAllDocsQueryNode;
using MatchNoDocsQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::MatchNoDocsQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SlopQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::SlopQueryNode;
using TokenizedPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::TokenizedPhraseQueryNode;
using MultiPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::MultiPhraseQueryNode;
using PointQueryNode =
    org::apache::lucene::queryparser::flexible::standard::nodes::PointQueryNode;
using PointRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PointRangeQueryNode;
using PrefixWildcardQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PrefixWildcardQueryNode;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;
using SynonymQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::SynonymQueryNode;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using Query = org::apache::lucene::search::Query;

StandardQueryTreeBuilder::StandardQueryTreeBuilder()
{
  setBuilder(GroupQueryNode::typeid, make_shared<GroupQueryNodeBuilder>());
  setBuilder(FieldQueryNode::typeid, make_shared<FieldQueryNodeBuilder>());
  setBuilder(BooleanQueryNode::typeid, make_shared<BooleanQueryNodeBuilder>());
  setBuilder(FuzzyQueryNode::typeid, make_shared<FuzzyQueryNodeBuilder>());
  setBuilder(PointQueryNode::typeid, make_shared<DummyQueryNodeBuilder>());
  setBuilder(PointRangeQueryNode::typeid,
             make_shared<PointRangeQueryNodeBuilder>());
  setBuilder(BoostQueryNode::typeid, make_shared<BoostQueryNodeBuilder>());
  setBuilder(ModifierQueryNode::typeid,
             make_shared<ModifierQueryNodeBuilder>());
  setBuilder(WildcardQueryNode::typeid,
             make_shared<WildcardQueryNodeBuilder>());
  setBuilder(TokenizedPhraseQueryNode::typeid,
             make_shared<PhraseQueryNodeBuilder>());
  setBuilder(MatchNoDocsQueryNode::typeid,
             make_shared<MatchNoDocsQueryNodeBuilder>());
  setBuilder(PrefixWildcardQueryNode::typeid,
             make_shared<PrefixWildcardQueryNodeBuilder>());
  setBuilder(TermRangeQueryNode::typeid,
             make_shared<TermRangeQueryNodeBuilder>());
  setBuilder(RegexpQueryNode::typeid, make_shared<RegexpQueryNodeBuilder>());
  setBuilder(SlopQueryNode::typeid, make_shared<SlopQueryNodeBuilder>());
  setBuilder(SynonymQueryNode::typeid, make_shared<SynonymQueryNodeBuilder>());
  setBuilder(MultiPhraseQueryNode::typeid,
             make_shared<MultiPhraseQueryNodeBuilder>());
  setBuilder(MatchAllDocsQueryNode::typeid,
             make_shared<MatchAllDocsQueryNodeBuilder>());
}

shared_ptr<Query> StandardQueryTreeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  return std::static_pointer_cast<Query>(QueryTreeBuilder::build(queryNode));
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders