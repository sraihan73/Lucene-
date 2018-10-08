using namespace std;

#include "TestStandardQP.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using QueryParserTestBase =
    org::apache::lucene::queryparser::util::QueryParserTestBase;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;

shared_ptr<StandardQueryParser>
TestStandardQP::getParser(shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>(a);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::OR);

  return qp;
}

shared_ptr<Query>
TestStandardQP::parse(const wstring &query,
                      shared_ptr<StandardQueryParser> qp) 
{
  return qp->parse(query, getDefaultField());
}

shared_ptr<CommonQueryParserConfiguration>
TestStandardQP::getParserConfig(shared_ptr<Analyzer> a) 
{
  return getParser(a);
}

shared_ptr<Query> TestStandardQP::getQuery(
    const wstring &query,
    shared_ptr<CommonQueryParserConfiguration> cqpC) 
{
  assert((cqpC != nullptr, L"Parameter must not be null"));
  assert(std::dynamic_pointer_cast<StandardQueryParser>(cqpC) != nullptr)
      : L"Parameter must be instance of StandardQueryParser";
  shared_ptr<StandardQueryParser> qp =
      std::static_pointer_cast<StandardQueryParser>(cqpC);
  return parse(query, qp);
}

shared_ptr<Query>
TestStandardQP::getQuery(const wstring &query,
                         shared_ptr<Analyzer> a) 
{
  return parse(query, getParser(a));
}

bool TestStandardQP::isQueryParserException(runtime_error exception)
{
  return std::dynamic_pointer_cast<QueryNodeException>(exception) != nullptr;
}

void TestStandardQP::setDefaultOperatorOR(
    shared_ptr<CommonQueryParserConfiguration> cqpC)
{
  assert(std::dynamic_pointer_cast<StandardQueryParser>(cqpC) != nullptr);
  shared_ptr<StandardQueryParser> qp =
      std::static_pointer_cast<StandardQueryParser>(cqpC);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::OR);
}

void TestStandardQP::setDefaultOperatorAND(
    shared_ptr<CommonQueryParserConfiguration> cqpC)
{
  assert(std::dynamic_pointer_cast<StandardQueryParser>(cqpC) != nullptr);
  shared_ptr<StandardQueryParser> qp =
      std::static_pointer_cast<StandardQueryParser>(cqpC);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
}

void TestStandardQP::setAutoGeneratePhraseQueries(
    shared_ptr<CommonQueryParserConfiguration> cqpC, bool value)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestStandardQP::setDateResolution(
    shared_ptr<CommonQueryParserConfiguration> cqpC,
    shared_ptr<std::wstring> field, Resolution value)
{
  assert(std::dynamic_pointer_cast<StandardQueryParser>(cqpC) != nullptr);
  shared_ptr<StandardQueryParser> qp =
      std::static_pointer_cast<StandardQueryParser>(cqpC);
  qp->getDateResolutionMap().emplace(field, value);
}

void TestStandardQP::testOperatorVsWhitespace() 
{
  // LUCENE-2566 is not implemented for StandardQueryParser
  // TODO implement LUCENE-2566 and remove this (override)method
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  assertQueryEquals(L"a - b", a, L"a -b");
  assertQueryEquals(L"a + b", a, L"a +b");
  assertQueryEquals(L"a ! b", a, L"a -b");
}

TestStandardQP::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestStandardQP> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestStandardQP::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

void TestStandardQP::testRangeWithPhrase() 
{
  // StandardSyntaxParser does not differentiate between a term and a
  // one-term-phrase in a range query.
  // Is this an issue? Should StandardSyntaxParser mark the text as
  // wasEscaped=true ?
  assertQueryEquals(L"[\\* TO \"*\"]", nullptr, L"[\\* TO *]");
}

void TestStandardQP::testEscapedVsQuestionMarkAsWildcard() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"a:b\\-?c", a, L"a:b-?c");
  assertQueryEquals(L"a:b\\+?c", a, L"a:b+?c");
  assertQueryEquals(L"a:b\\:?c", a, L"a:b:?c");

  assertQueryEquals(L"a:b\\\\?c", a, L"a:b\\?c");
}

void TestStandardQP::testEscapedWildcard() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  shared_ptr<WildcardQuery> q = make_shared<WildcardQuery>(
      make_shared<Term>(L"field", L"foo?ba?r")); // TODO not correct!!
  assertEquals(q, getQuery(L"foo\\?ba?r", qp));
}

void TestStandardQP::testAutoGeneratePhraseQueriesOn() 
{
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    setAutoGeneratePhraseQueries(getParser(nullptr), true);
    org::apache::lucene::queryparser::util::QueryParserTestBase::
        testAutoGeneratePhraseQueriesOn();
  });
}

void TestStandardQP::testStarParsing()  {}

void TestStandardQP::testDefaultOperator() 
{
  shared_ptr<StandardQueryParser> qp =
      getParser(make_shared<MockAnalyzer>(random()));
  // make sure OR is the default:
  assertEquals(StandardQueryConfigHandler::Operator::OR,
               qp->getDefaultOperator());
  setDefaultOperatorAND(qp);
  assertEquals(StandardQueryConfigHandler::Operator::AND,
               qp->getDefaultOperator());
  setDefaultOperatorOR(qp);
  assertEquals(StandardQueryConfigHandler::Operator::OR,
               qp->getDefaultOperator());
}

void TestStandardQP::testNewFieldQuery() 
{
  /** ordinary behavior, synonyms form uncoordinated bool query */
  shared_ptr<StandardQueryParser> dumb =
      getParser(make_shared<QueryParserTestBase::Analyzer1>());
  shared_ptr<BooleanQuery::Builder> expanded =
      make_shared<BooleanQuery::Builder>();
  expanded->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"dogs")),
                BooleanClause::Occur::SHOULD);
  expanded->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"dog")),
                BooleanClause::Occur::SHOULD);
  assertEquals(expanded->build(), dumb->parse(L"\"dogs\"", L"field"));
  /** even with the phrase operator the behavior is the same */
  assertEquals(expanded->build(), dumb->parse(L"dogs", L"field"));

  /**
   * custom behavior, the synonyms are expanded, unless you use quote operator
   */
  // TODO test something like "SmartQueryParser()"
}

void TestStandardQP::testQPA() 
{
  QueryParserTestBase::testQPA();

  assertQueryEquals(L"term phrase term", qpAnalyzer,
                    L"term (phrase1 phrase2) term");

  shared_ptr<CommonQueryParserConfiguration> cqpc = getParserConfig(qpAnalyzer);
  setDefaultOperatorAND(cqpc);
  assertQueryEquals(cqpc, L"field", L"term phrase term",
                    L"+term +(+phrase1 +phrase2) +term");
}
} // namespace org::apache::lucene::queryparser::flexible::standard