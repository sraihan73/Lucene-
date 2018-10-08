using namespace std;

#include "TestSpanQueryParser.h"

namespace org::apache::lucene::queryparser::flexible::spans
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
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
using WildcardQueryNodeProcessor = org::apache::lucene::queryparser::flexible::
    standard::processors::WildcardQueryNodeProcessor;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

TestSpanQueryParser::TestSpanQueryParser()
{
  // empty constructor
}

void TestSpanQueryParser::setUp() 
{
  LuceneTestCase::setUp();

  this->spanProcessorPipeline = make_shared<QueryNodeProcessorPipeline>();
  this->spanQueryConfigHandler = make_shared<SpansQueryConfigHandler>();
  this->spansQueryTreeBuilder = make_shared<SpansQueryTreeBuilder>();

  // set up the processor pipeline
  this->spanProcessorPipeline->setQueryConfigHandler(
      this->spanQueryConfigHandler);

  this->spanProcessorPipeline->add(make_shared<WildcardQueryNodeProcessor>());
  this->spanProcessorPipeline->add(
      make_shared<SpansValidatorQueryNodeProcessor>());
  this->spanProcessorPipeline->add(
      make_shared<UniqueFieldQueryNodeProcessor>());
}

shared_ptr<SpanQuery> TestSpanQueryParser::getSpanQuery(
    shared_ptr<std::wstring> query) 
{
  return getSpanQuery(L"", query);
}

shared_ptr<SpanQuery> TestSpanQueryParser::getSpanQuery(
    const wstring &uniqueField,
    shared_ptr<std::wstring> query) 
{

  this->spanQueryConfigHandler->set(SpansQueryConfigHandler::UNIQUE_FIELD,
                                    uniqueField);

  shared_ptr<QueryNode> queryTree =
      this->queryParser->parse(query, L"defaultField");
  queryTree = this->spanProcessorPipeline->process(queryTree);

  return this->spansQueryTreeBuilder->build(queryTree);
}

void TestSpanQueryParser::testTermSpans() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"field:term")->toString(), L"term");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"term")->toString(), L"term");

  assertTrue(std::dynamic_pointer_cast<SpanTermQuery>(
                 getSpanQuery(L"field:term")) != nullptr);
  assertTrue(std::dynamic_pointer_cast<SpanTermQuery>(getSpanQuery(L"term")) !=
             nullptr);
}

void TestSpanQueryParser::testUniqueField() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"field", L"term")->toString(), L"field:term");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"field", L"field:term")->toString(),
               L"field:term");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"field", L"anotherField:term")->toString(),
               L"field:term");
}

void TestSpanQueryParser::testOrSpans() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"term1 term2")->toString(),
               L"spanOr([term1, term2])");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(getSpanQuery(L"term1 OR term2")->toString(),
               L"spanOr([term1, term2])");

  assertTrue(std::dynamic_pointer_cast<SpanOrQuery>(
                 getSpanQuery(L"term1 term2")) != nullptr);
  assertTrue(std::dynamic_pointer_cast<SpanOrQuery>(
                 getSpanQuery(L"term1 term2")) != nullptr);
}

void TestSpanQueryParser::testQueryValidator() 
{

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"term*"); });

  expectThrows(QueryNodeException::typeid,
               [&]() { getSpanQuery(L"[a TO z]"); });

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"a~0.5"); });

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"a^0.5"); });

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"\"a b\""); });

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"(a b)"); });

  expectThrows(QueryNodeException::typeid, [&]() { getSpanQuery(L"a AND b"); });
}
} // namespace org::apache::lucene::queryparser::flexible::spans