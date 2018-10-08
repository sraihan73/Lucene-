using namespace std;

#include "TestSpanQueryParserSimpleSample.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using OrQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::OrQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SyntaxParser =
    org::apache::lucene::queryparser::flexible::core::parser::SyntaxParser;
using QueryNodeProcessorPipeline = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorPipeline;
using StandardSyntaxParser = org::apache::lucene::queryparser::flexible::
    standard::parser::StandardSyntaxParser;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSpanQueryParserSimpleSample::testBasicDemo() 
{
  shared_ptr<SyntaxParser> queryParser = make_shared<StandardSyntaxParser>();

  // convert the std::wstring into a QueryNode tree
  shared_ptr<QueryNode> queryTree = queryParser->parse(L"body:text", nullptr);

  // create a config handler with a attribute used in
  // UniqueFieldQueryNodeProcessor
  shared_ptr<QueryConfigHandler> spanQueryConfigHandler =
      make_shared<SpansQueryConfigHandler>();
  spanQueryConfigHandler->set(SpansQueryConfigHandler::UNIQUE_FIELD, L"index");

  // set up the processor pipeline with the ConfigHandler
  // and create the pipeline for this simple demo
  shared_ptr<QueryNodeProcessorPipeline> spanProcessorPipeline =
      make_shared<QueryNodeProcessorPipeline>(spanQueryConfigHandler);
  // @see SpansValidatorQueryNodeProcessor
  spanProcessorPipeline->push_back(
      make_shared<SpansValidatorQueryNodeProcessor>());
  // @see UniqueFieldQueryNodeProcessor
  spanProcessorPipeline->push_back(
      make_shared<UniqueFieldQueryNodeProcessor>());

  // print to show out the QueryNode tree before being processed
  if (VERBOSE) {
    wcout << queryTree << endl;
  }

  // Process the QueryTree using our new Processors
  queryTree = spanProcessorPipeline->process(queryTree);

  // print to show out the QueryNode tree after being processed
  if (VERBOSE) {
    wcout << queryTree << endl;
  }

  // create a instance off the Builder
  shared_ptr<SpansQueryTreeBuilder> spansQueryTreeBuilder =
      make_shared<SpansQueryTreeBuilder>();

  // convert QueryNode tree to span query Objects
  shared_ptr<SpanQuery> spanquery = spansQueryTreeBuilder->build(queryTree);

  assertTrue(std::dynamic_pointer_cast<SpanTermQuery>(spanquery) != nullptr);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(spanquery->toString(), L"index:text");
}
} // namespace org::apache::lucene::queryparser::flexible::spans