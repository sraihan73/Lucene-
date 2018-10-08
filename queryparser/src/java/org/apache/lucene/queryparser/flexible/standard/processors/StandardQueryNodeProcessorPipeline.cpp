using namespace std;

#include "StandardQueryNodeProcessorPipeline.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using NoChildOptimizationQueryNodeProcessor = org::apache::lucene::queryparser::
    flexible::core::processors::NoChildOptimizationQueryNodeProcessor;
using QueryNodeProcessorPipeline = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorPipeline;
using RemoveDeletedQueryNodesProcessor = org::apache::lucene::queryparser::
    flexible::core::processors::RemoveDeletedQueryNodesProcessor;
using StandardQueryTreeBuilder = org::apache::lucene::queryparser::flexible::
    standard::builders::StandardQueryTreeBuilder;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using StandardSyntaxParser = org::apache::lucene::queryparser::flexible::
    standard::parser::StandardSyntaxParser;
using Query = org::apache::lucene::search::Query;

StandardQueryNodeProcessorPipeline::StandardQueryNodeProcessorPipeline(
    shared_ptr<QueryConfigHandler> queryConfig)
    : org::apache::lucene::queryparser::flexible::core::processors::
          QueryNodeProcessorPipeline(queryConfig)
{

  add(make_shared<WildcardQueryNodeProcessor>());
  add(make_shared<MultiFieldQueryNodeProcessor>());
  add(make_shared<FuzzyQueryNodeProcessor>());
  add(make_shared<RegexpQueryNodeProcessor>());
  add(make_shared<MatchAllDocsQueryNodeProcessor>());
  add(make_shared<OpenRangeQueryNodeProcessor>());
  add(make_shared<PointQueryNodeProcessor>());
  add(make_shared<PointRangeQueryNodeProcessor>());
  add(make_shared<TermRangeQueryNodeProcessor>());
  add(make_shared<AllowLeadingWildcardProcessor>());
  add(make_shared<AnalyzerQueryNodeProcessor>());
  add(make_shared<PhraseSlopQueryNodeProcessor>());
  // add(new GroupQueryNodeProcessor());
  add(make_shared<BooleanQuery2ModifierNodeProcessor>());
  add(make_shared<NoChildOptimizationQueryNodeProcessor>());
  add(make_shared<RemoveDeletedQueryNodesProcessor>());
  add(make_shared<RemoveEmptyNonLeafQueryNodeProcessor>());
  add(make_shared<BooleanSingleChildOptimizationQueryNodeProcessor>());
  add(make_shared<DefaultPhraseSlopQueryNodeProcessor>());
  add(make_shared<BoostQueryNodeProcessor>());
  add(make_shared<MultiTermRewriteMethodProcessor>());
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors