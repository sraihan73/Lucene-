using namespace std;

#include "TestQPHelper.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using DateTools = org::apache::lucene::document::DateTools;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using QueryNodeProcessorPipeline = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorPipeline;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using StandardQueryConfigHandler = org::apache::lucene::queryparser::flexible::
    standard::config::StandardQueryConfigHandler;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Ignore;
shared_ptr<org::apache::lucene::analysis::Analyzer> TestQPHelper::qpAnalyzer;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void TestQPHelper::beforeClass() { qpAnalyzer = make_shared<QPTestAnalyzer>(); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestQPHelper::afterClass() { qpAnalyzer.reset(); }

TestQPHelper::QPTestFilter::QPTestFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TestQPHelper::QPTestFilter::incrementToken() 
{
  if (inPhrase) {
    inPhrase = false;
    clearAttributes();
    termAtt->setEmpty()->append(L"phrase2");
    offsetAtt->setOffset(savedStart, savedEnd);
    return true;
  } else {
    while (input->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (termAtt->toString()->equals(L"phrase")) {
        inPhrase = true;
        savedStart = offsetAtt->startOffset();
        savedEnd = offsetAtt->endOffset();
        termAtt->setEmpty()->append(L"phrase1");
        offsetAtt->setOffset(savedStart, savedEnd);
        return true;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      else if (!termAtt->toString()->equals(L"stop")) {
        return true;
      }
    }
  }
  return false;
}

void TestQPHelper::QPTestFilter::reset() 
{
  TokenFilter::reset();
  this->inPhrase = false;
  this->savedStart = 0;
  this->savedEnd = 0;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQPHelper::QPTestAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<QPTestFilter>(tokenizer));
}

TestQPHelper::QPTestParser::QPTestParser(shared_ptr<Analyzer> a)
{
  (std::static_pointer_cast<QueryNodeProcessorPipeline>(
       getQueryNodeProcessor()))
      ->push_back(make_shared<QPTestParserQueryNodeProcessor>());
  this->setAnalyzer(a);
}

shared_ptr<QueryNode>
TestQPHelper::QPTestParser::QPTestParserQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

shared_ptr<QueryNode>
TestQPHelper::QPTestParser::QPTestParserQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<WildcardQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<FuzzyQueryNode>(node) != nullptr) {

    throw make_shared<QueryNodeException>(
        make_shared<MessageImpl>(QueryParserMessages::EMPTY_MESSAGE));
  }

  return node;
}

deque<std::shared_ptr<QueryNode>>
TestQPHelper::QPTestParser::QPTestParserQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}

void TestQPHelper::setUp() 
{
  LuceneTestCase::setUp();
  originalMaxClauses = BooleanQuery::getMaxClauseCount();
}

shared_ptr<StandardQueryParser>
TestQPHelper::getParser(shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(a);

  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::OR);

  return qp;
}

shared_ptr<Query>
TestQPHelper::getQuery(const wstring &query,
                       shared_ptr<Analyzer> a) 
{
  return getParser(a)->parse(query, L"field");
}

shared_ptr<Query> TestQPHelper::getQueryAllowLeadingWildcard(
    const wstring &query, shared_ptr<Analyzer> a) 
{
  shared_ptr<StandardQueryParser> parser = getParser(a);
  parser->setAllowLeadingWildcard(true);
  return parser->parse(query, L"field");
}

void TestQPHelper::assertQueryEquals(const wstring &query,
                                     shared_ptr<Analyzer> a,
                                     const wstring &result) 
{
  shared_ptr<Query> q = getQuery(query, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestQPHelper::assertMatchNoDocsQuery(
    const wstring &queryString, shared_ptr<Analyzer> a) 
{
  assertMatchNoDocsQuery(getQuery(queryString, a));
}

void TestQPHelper::assertMatchNoDocsQuery(shared_ptr<Query> query) throw(
    runtime_error)
{
  if (std::dynamic_pointer_cast<MatchNoDocsQuery>(query) != nullptr) {
    // good
  } else if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr &&
             (std::static_pointer_cast<BooleanQuery>(query))
                 ->clauses()
                 .empty()) {
    // good
  } else {
    fail(L"expected MatchNoDocsQuery or an empty BooleanQuery but got: " +
         query);
  }
}

void TestQPHelper::assertQueryEqualsAllowLeadingWildcard(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  shared_ptr<Query> q = getQueryAllowLeadingWildcard(query, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestQPHelper::assertQueryEquals(shared_ptr<StandardQueryParser> qp,
                                     const wstring &field, const wstring &query,
                                     const wstring &result) 
{
  shared_ptr<Query> q = qp->parse(query, field);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(field);
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestQPHelper::assertEscapedQueryEquals(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  wstring escapedQuery = QueryParserUtil::escape(query);
  if (escapedQuery != result) {
    fail(L"Query /" + query + L"/ yielded /" + escapedQuery +
         L"/, expecting /" + result + L"/");
  }
}

void TestQPHelper::assertWildcardQueryEquals(
    const wstring &query, const wstring &result,
    bool allowLeadingWildcard) 
{
  shared_ptr<StandardQueryParser> qp = getParser(nullptr);
  qp->setAllowLeadingWildcard(allowLeadingWildcard);
  shared_ptr<Query> q = qp->parse(query, L"field");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"WildcardQuery /" + query + L"/ yielded /" + s + L"/, expecting /" +
         result + L"/");
  }
}

void TestQPHelper::assertWildcardQueryEquals(
    const wstring &query, const wstring &result) 
{
  assertWildcardQueryEquals(query, result, false);
}

shared_ptr<Query>
TestQPHelper::getQueryDOA(const wstring &query,
                          shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(a);
  qp->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);

  return qp->parse(query, L"field");
}

void TestQPHelper::assertQueryEqualsDOA(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  shared_ptr<Query> q = getQueryDOA(query, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void TestQPHelper::testConstantScoreAutoRewrite() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  shared_ptr<Query> q = qp->parse(L"foo*bar", L"field");
  assertTrue(std::dynamic_pointer_cast<WildcardQuery>(q) != nullptr);
  assertEquals(
      MultiTermQuery::CONSTANT_SCORE_REWRITE,
      (std::static_pointer_cast<MultiTermQuery>(q))->getRewriteMethod());

  q = qp->parse(L"foo*", L"field");
  assertTrue(std::dynamic_pointer_cast<PrefixQuery>(q) != nullptr);
  assertEquals(
      MultiTermQuery::CONSTANT_SCORE_REWRITE,
      (std::static_pointer_cast<MultiTermQuery>(q))->getRewriteMethod());

  q = qp->parse(L"[a TO z]", L"field");
  assertTrue(std::dynamic_pointer_cast<TermRangeQuery>(q) != nullptr);
  assertEquals(
      MultiTermQuery::CONSTANT_SCORE_REWRITE,
      (std::static_pointer_cast<MultiTermQuery>(q))->getRewriteMethod());
}

void TestQPHelper::testCJK() 
{
  // Test Ideographic Space - As wide as a CJK character cell (fullwidth)
  // used google to translate the word "term" to japanese -> ??
  assertQueryEquals(L"term\u3000term\u3000term", nullptr,
                    L"term\u0020term\u0020term");
  assertQueryEqualsAllowLeadingWildcard(L"??\u3000??\u3000??", nullptr,
                                        L"??\u0020??\u0020??");
}

TestQPHelper::SimpleCJKTokenizer::SimpleCJKTokenizer()
    : org::apache::lucene::analysis::Tokenizer()
{
}

bool TestQPHelper::SimpleCJKTokenizer::incrementToken() 
{
  int ch = input->read();
  if (ch < 0) {
    return false;
  }
  clearAttributes();
  termAtt->setEmpty()->append(static_cast<wchar_t>(ch));
  return true;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQPHelper::SimpleCJKAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<SimpleCJKTokenizer>());
}

void TestQPHelper::testCJKTerm() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
                Occur::SHOULD);
  assertEquals(expected->build(), getQuery(L"中国", analyzer));

  expected = make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                Occur::MUST);
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
             Occur::SHOULD);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
             Occur::SHOULD);
  expected->add(inner->build(), Occur::MUST);
  assertEquals(expected->build(),
               getQuery(L"中 AND 中国", make_shared<SimpleCJKAnalyzer>()));
}

void TestQPHelper::testCJKBoostedTerm() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                 Occur::SHOULD);
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
                 Occur::SHOULD);
  shared_ptr<Query> expected = expectedB->build();
  expected = make_shared<BoostQuery>(expected, 0.5f);
  assertEquals(expected, getQuery(L"中国^0.5", analyzer));
}

void TestQPHelper::testCJKPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"中", L"国");

  assertEquals(expected, getQuery(L"\"中国\"", analyzer));
}

void TestQPHelper::testCJKBoostedPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<Query> expected = make_shared<PhraseQuery>(L"field", L"中", L"国");
  expected = make_shared<BoostQuery>(expected, 0.5f);

  assertEquals(expected, getQuery(L"\"中国\"^0.5", analyzer));
}

void TestQPHelper::testCJKSloppyPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(3, L"field", L"中", L"国");

  assertEquals(expected, getQuery(L"\"中国\"~3", analyzer));
}

void TestQPHelper::testSimple() 
{
  assertQueryEquals(L"field=a", nullptr, L"a");
  assertQueryEquals(L"\"term germ\"~2", nullptr, L"\"term germ\"~2");
  assertQueryEquals(L"term term term", nullptr, L"term term term");
  assertQueryEquals(
      L"t�rm term term",
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      L"t�rm term term");
  assertQueryEquals(
      L"�mlaut",
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      L"�mlaut");

  // FIXME: change MockAnalyzer to not extend CharTokenizer for this test
  // assertQueryEquals("\"\"", new KeywordAnalyzer(), "");
  // assertQueryEquals("foo:\"\"", new KeywordAnalyzer(), "foo:");

  assertQueryEquals(L"a AND b", nullptr, L"+a +b");
  assertQueryEquals(L"(a AND b)", nullptr, L"+a +b");
  assertQueryEquals(L"c OR (a AND b)", nullptr, L"c (+a +b)");

  assertQueryEquals(L"a AND NOT b", nullptr, L"+a -b");

  assertQueryEquals(L"a AND -b", nullptr, L"+a -b");

  assertQueryEquals(L"a AND !b", nullptr, L"+a -b");

  assertQueryEquals(L"a && b", nullptr, L"+a +b");

  assertQueryEquals(L"a && ! b", nullptr, L"+a -b");

  assertQueryEquals(L"a OR b", nullptr, L"a b");
  assertQueryEquals(L"a || b", nullptr, L"a b");

  assertQueryEquals(L"a OR !b", nullptr, L"a -b");

  assertQueryEquals(L"a OR ! b", nullptr, L"a -b");

  assertQueryEquals(L"a OR -b", nullptr, L"a -b");

  assertQueryEquals(L"+term -term term", nullptr, L"+term -term term");
  assertQueryEquals(L"foo:term AND field:anotherTerm", nullptr,
                    L"+foo:term +anotherterm");
  assertQueryEquals(L"term AND \"phrase phrase\"", nullptr,
                    L"+term +\"phrase phrase\"");
  assertQueryEquals(L"\"hello there\"", nullptr, L"\"hello there\"");
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(
                 getQuery(L"a AND b", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(
                 getQuery(L"hello", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(
                 getQuery(L"\"hello there\"", nullptr)) != nullptr);

  assertQueryEquals(L"germ term^2.0", nullptr, L"germ (term)^2.0");
  assertQueryEquals(L"(term)^2.0", nullptr, L"(term)^2.0");
  assertQueryEquals(L"(germ term)^2.0", nullptr, L"(germ term)^2.0");
  assertQueryEquals(L"term^2.0", nullptr, L"(term)^2.0");
  assertQueryEquals(L"term^2", nullptr, L"(term)^2.0");
  assertQueryEquals(L"\"germ term\"^2.0", nullptr, L"(\"germ term\")^2.0");
  assertQueryEquals(L"\"term germ\"^2", nullptr, L"(\"term germ\")^2.0");

  assertQueryEquals(L"(foo OR bar) AND (baz OR boo)", nullptr,
                    L"+(foo bar) +(baz boo)");
  assertQueryEquals(L"((a OR b) AND NOT c) OR d", nullptr, L"(+(a b) -c) d");
  assertQueryEquals(L"+(apple \"steve jobs\") -(foo bar baz)", nullptr,
                    L"+(apple \"steve jobs\") -(foo bar baz)");
  assertQueryEquals(L"+title:(dog OR cat) -author:\"bob dole\"", nullptr,
                    L"+(title:dog title:cat) -author:\"bob dole\"");
}

void TestQPHelper::testPunct() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"a&b", a, L"a&b");
  assertQueryEquals(L"a&&b", a, L"a&&b");
  assertQueryEquals(L".NET", a, L".NET");
}

void TestQPHelper::testGroup() 
{
  assertQueryEquals(L"!(a AND b) OR c", nullptr, L"-(+a +b) c");
  assertQueryEquals(L"!(a AND b) AND c", nullptr, L"-(+a +b) +c");
  assertQueryEquals(L"((a AND b) AND c)", nullptr, L"+(+a +b) +c");
  assertQueryEquals(L"(a AND b) AND c", nullptr, L"+(+a +b) +c");
  assertQueryEquals(L"b !(a AND b)", nullptr, L"b -(+a +b)");
  assertQueryEquals(L"(a AND b)^4 OR c", nullptr, L"(+a +b)^4.0 c");
}

void TestQPHelper::testSlop() 
{

  assertQueryEquals(L"\"term germ\"~2", nullptr, L"\"term germ\"~2");
  assertQueryEquals(L"\"term germ\"~2 flork", nullptr,
                    L"\"term germ\"~2 flork");
  assertQueryEquals(L"\"term\"~2", nullptr, L"term");
  assertQueryEquals(L"\" \"~2 germ", nullptr, L"germ");
  assertQueryEquals(L"\"term germ\"~2^2", nullptr, L"(\"term germ\"~2)^2.0");
}

void TestQPHelper::testNumber() 
{
  // The numbers go away because SimpleAnalzyer ignores them
  assertMatchNoDocsQuery(L"3", nullptr);
  assertQueryEquals(L"term 1.0 1 2", nullptr, L"term");
  assertQueryEquals(L"term term1 term2", nullptr, L"term term term");

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"3", a, L"3");
  assertQueryEquals(L"term 1.0 1 2", a, L"term 1.0 1 2");
  assertQueryEquals(L"term term1 term2", a, L"term term1 term2");
}

void TestQPHelper::testLeadingNegation() 
{
  assertQueryEquals(L"-term", nullptr, L"-term");
  assertQueryEquals(L"!term", nullptr, L"-term");
  assertQueryEquals(L"NOT term", nullptr, L"-term");
}

void TestQPHelper::testNegationInParentheses() 
{
  assertQueryEquals(L"(-a)", nullptr, L"-a");
  assertQueryEquals(L"(!a)", nullptr, L"-a");
  assertQueryEquals(L"(NOT a)", nullptr, L"-a");
  assertQueryEquals(L"a (!b)", nullptr, L"a (-b)");
  assertQueryEquals(L"+a +(!b)", nullptr, L"+a +(-b)");
  assertQueryEquals(L"a AND (!b)", nullptr, L"+a +(-b)");
  assertQueryEquals(L"a (NOT b)", nullptr, L"a (-b)");
  assertQueryEquals(L"a AND (NOT b)", nullptr, L"+a +(-b)");
}

void TestQPHelper::testWildcard() 
{
  assertQueryEquals(L"term*", nullptr, L"term*");
  assertQueryEquals(L"term*^2", nullptr, L"(term*)^2.0");
  assertQueryEquals(L"term~", nullptr, L"term~2");
  assertQueryEquals(L"term~0.7", nullptr, L"term~1");

  assertQueryEquals(L"term~^3", nullptr, L"(term~2)^3.0");

  assertQueryEquals(L"term^3~", nullptr, L"(term~2)^3.0");
  assertQueryEquals(L"term*germ", nullptr, L"term*germ");
  assertQueryEquals(L"term*germ^3", nullptr, L"(term*germ)^3.0");

  assertTrue(std::dynamic_pointer_cast<PrefixQuery>(
                 getQuery(L"term*", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<BoostQuery>(
                 getQuery(L"term*^2", nullptr)) != nullptr);
  assertTrue(
      std::dynamic_pointer_cast<PrefixQuery>(
          (std::static_pointer_cast<BoostQuery>(getQuery(L"term*^2", nullptr)))
              ->getQuery()) != nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(
                 getQuery(L"term~", nullptr)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(
                 getQuery(L"term~0.7", nullptr)) != nullptr);
  shared_ptr<FuzzyQuery> fq =
      std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~0.7", nullptr));
  assertEquals(1, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());
  fq = std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~", nullptr));
  assertEquals(2, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());

  assertQueryNodeException(L"term~1.1"); // value > 1, throws exception

  assertTrue(std::dynamic_pointer_cast<WildcardQuery>(
                 getQuery(L"term*germ", nullptr)) != nullptr);

  /*
   * Tests to see that wild card terms are (or are not) properly lower-cased
   * with propery parser configuration
   */
  // First prefix queries:
  // by default, convert to lowercase:
  assertWildcardQueryEquals(L"Term*", L"term*");
  // explicitly set lowercase:
  assertWildcardQueryEquals(L"term*", L"term*");
  assertWildcardQueryEquals(L"Term*", L"term*");
  assertWildcardQueryEquals(L"TERM*", L"term*");
  // Then 'full' wildcard queries:
  // by default, convert to lowercase:
  assertWildcardQueryEquals(L"Te?m", L"te?m");
  // explicitly set lowercase:
  assertWildcardQueryEquals(L"te?m", L"te?m");
  assertWildcardQueryEquals(L"Te?m", L"te?m");
  assertWildcardQueryEquals(L"TE?M", L"te?m");
  assertWildcardQueryEquals(L"Te?m*gerM", L"te?m*germ");
  // Fuzzy queries:
  assertWildcardQueryEquals(L"Term~", L"term~2");
  // Range queries:

  // TODO: implement this on QueryParser
  // Q0002E_INVALID_SYNTAX_CANNOT_PARSE: Syntax Error, cannot parse '[A TO
  // C]': Lexical error at line 1, column 1. Encountered: "[" (91), after
  // : ""
  assertWildcardQueryEquals(L"[A TO C]", L"[a TO c]");
  // Test suffix queries: first disallow
  expectThrows(QueryNodeException::typeid,
               [&]() { assertWildcardQueryEquals(L"*Term", L"*term"); });

  expectThrows(QueryNodeException::typeid,
               [&]() { assertWildcardQueryEquals(L"?Term", L"?term"); });

  // Test suffix queries: then allow
  assertWildcardQueryEquals(L"*Term", L"*term", true);
  assertWildcardQueryEquals(L"?Term", L"?term", true);
}

void TestQPHelper::testLeadingWildcardType() 
{
  shared_ptr<StandardQueryParser> qp = getParser(nullptr);
  qp->setAllowLeadingWildcard(true);
  assertEquals(WildcardQuery::typeid,
               qp->parse(L"t*erm*", L"field")->getClass());
  assertEquals(WildcardQuery::typeid,
               qp->parse(L"?term*", L"field")->getClass());
  assertEquals(WildcardQuery::typeid,
               qp->parse(L"*term*", L"field")->getClass());
}

void TestQPHelper::testQPA() 
{
  assertQueryEquals(L"term term^3.0 term", qpAnalyzer, L"term (term)^3.0 term");
  assertQueryEquals(L"term stop^3.0 term", qpAnalyzer, L"term term");

  assertQueryEquals(L"term term term", qpAnalyzer, L"term term term");
  assertQueryEquals(L"term +stop term", qpAnalyzer, L"term term");
  assertQueryEquals(L"term -stop term", qpAnalyzer, L"term term");

  assertQueryEquals(L"drop AND (stop) AND roll", qpAnalyzer, L"+drop +roll");
  assertQueryEquals(L"term +(stop) term", qpAnalyzer, L"term term");
  assertQueryEquals(L"term -(stop) term", qpAnalyzer, L"term term");

  assertQueryEquals(L"drop AND stop AND roll", qpAnalyzer, L"+drop +roll");
  assertQueryEquals(L"term phrase term", qpAnalyzer,
                    L"term (phrase1 phrase2) term");

  assertQueryEquals(L"term AND NOT phrase term", qpAnalyzer,
                    L"+term -(phrase1 phrase2) term");

  assertMatchNoDocsQuery(L"stop^3", qpAnalyzer);
  assertMatchNoDocsQuery(L"stop", qpAnalyzer);
  assertMatchNoDocsQuery(L"(stop)^3", qpAnalyzer);
  assertMatchNoDocsQuery(L"((stop))^3", qpAnalyzer);
  assertMatchNoDocsQuery(L"(stop^3)", qpAnalyzer);
  assertMatchNoDocsQuery(L"((stop)^3)", qpAnalyzer);
  assertMatchNoDocsQuery(L"(stop)", qpAnalyzer);
  assertMatchNoDocsQuery(L"((stop))", qpAnalyzer);
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(
                 getQuery(L"term term term", qpAnalyzer)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(
                 getQuery(L"term +stop", qpAnalyzer)) != nullptr);
}

void TestQPHelper::testRange() 
{
  assertQueryEquals(L"[ a TO z]", nullptr, L"[a TO z]");
  assertEquals(MultiTermQuery::CONSTANT_SCORE_REWRITE,
               (std::static_pointer_cast<TermRangeQuery>(
                    getQuery(L"[ a TO z]", nullptr)))
                   ->getRewriteMethod());

  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();

  qp->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertEquals(MultiTermQuery::SCORING_BOOLEAN_REWRITE,
               (std::static_pointer_cast<TermRangeQuery>(
                    qp->parse(L"[ a TO z]", L"field")))
                   ->getRewriteMethod());

  // test open ranges
  assertQueryEquals(L"[ a TO * ]", nullptr, L"[a TO *]");
  assertQueryEquals(L"[ * TO z ]", nullptr, L"[* TO z]");
  assertQueryEquals(L"[ * TO * ]", nullptr, L"[* TO *]");

  assertQueryEquals(L"field>=a", nullptr, L"[a TO *]");
  assertQueryEquals(L"field>a", nullptr, L"{a TO *]");
  assertQueryEquals(L"field<=a", nullptr, L"[* TO a]");
  assertQueryEquals(L"field<a", nullptr, L"[* TO a}");

  // mixing exclude and include bounds
  assertQueryEquals(L"{ a TO z ]", nullptr, L"{a TO z]");
  assertQueryEquals(L"[ a TO z }", nullptr, L"[a TO z}");
  assertQueryEquals(L"{ a TO * ]", nullptr, L"{a TO *]");
  assertQueryEquals(L"[ * TO z }", nullptr, L"[* TO z}");

  assertQueryEquals(L"[ a TO z ]", nullptr, L"[a TO z]");
  assertQueryEquals(L"{ a TO z}", nullptr, L"{a TO z}");
  assertQueryEquals(L"{ a TO z }", nullptr, L"{a TO z}");
  assertQueryEquals(L"{ a TO z }^2.0", nullptr, L"({a TO z})^2.0");
  assertQueryEquals(L"[ a TO z] OR bar", nullptr, L"[a TO z] bar");
  assertQueryEquals(L"[ a TO z] AND bar", nullptr, L"+[a TO z] +bar");
  assertQueryEquals(L"( bar blar { a TO z}) ", nullptr, L"bar blar {a TO z}");
  assertQueryEquals(L"gack ( bar blar { a TO z}) ", nullptr,
                    L"gack (bar blar {a TO z})");
}

wstring
TestQPHelper::getDate(const wstring &s,
                      DateTools::Resolution resolution) 
{
  // we use the default Locale since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  return getDate(df->parse(s), resolution);
}

wstring TestQPHelper::getDate(Date d, DateTools::Resolution resolution)
{
  return DateTools::dateToString(d, resolution);
}

wstring TestQPHelper::escapeDateString(const wstring &s)
{
  if (s.find(L" ") != wstring::npos) {
    return L"\"" + s + L"\"";
  } else {
    return s;
  }
}

wstring TestQPHelper::getLocalizedDate(int year, int month, int day)
{
  // we use the default Locale/TZ since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  shared_ptr<Calendar> calendar = make_shared<GregorianCalendar>(
      TimeZone::getDefault(), Locale::getDefault());
  calendar->clear();
  calendar->set(year, month, day);
  calendar->set(Calendar::HOUR_OF_DAY, 23);
  calendar->set(Calendar::MINUTE, 59);
  calendar->set(Calendar::SECOND, 59);
  calendar->set(Calendar::MILLISECOND, 999);
  return df->format(calendar->getTime());
}

void TestQPHelper::testDateRange() 
{
  wstring startDate = getLocalizedDate(2002, 1, 1);
  wstring endDate = getLocalizedDate(2002, 1, 4);
  // we use the default Locale/TZ since LuceneTestCase randomizes it
  shared_ptr<Calendar> endDateExpected = make_shared<GregorianCalendar>(
      TimeZone::getDefault(), Locale::getDefault());
  endDateExpected->clear();
  endDateExpected->set(2002, 1, 4, 23, 59, 59);
  endDateExpected->set(Calendar::MILLISECOND, 999);
  const wstring defaultField = L"default";
  const wstring monthField = L"month";
  const wstring hourField = L"hour";
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();

  unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution> dateRes =
      unordered_map<std::shared_ptr<std::wstring>, DateTools::Resolution>();

  // set a field specific date resolution
  dateRes.emplace(monthField, DateTools::Resolution::MONTH);
  qp->setDateResolutionMap(dateRes);

  // set default date resolution to MILLISECOND
  qp->setDateResolution(DateTools::Resolution::MILLISECOND);

  // set second field specific date resolution
  dateRes.emplace(hourField, DateTools::Resolution::HOUR);
  qp->setDateResolutionMap(dateRes);

  // for this field no field specific date resolution has been set,
  // so verify if the default resolution is used
  assertDateRangeQueryEquals(qp, defaultField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MILLISECOND);

  // verify if field specific date resolutions are used for these two
  // fields
  assertDateRangeQueryEquals(qp, monthField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MONTH);

  assertDateRangeQueryEquals(qp, hourField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::HOUR);
}

void TestQPHelper::assertDateRangeQueryEquals(
    shared_ptr<StandardQueryParser> qp, const wstring &field,
    const wstring &startDate, const wstring &endDate, Date endDateInclusive,
    DateTools::Resolution resolution) 
{
  assertQueryEquals(qp, field,
                    field + L":[" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"]",
                    L"[" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDateInclusive, resolution) + L"]");
  assertQueryEquals(qp, field,
                    field + L":{" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"}",
                    L"{" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDate, resolution) + L"}");
}

void TestQPHelper::testEscaped() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  /*
   * assertQueryEquals("\\[brackets", a, "\\[brackets");
   * assertQueryEquals("\\[brackets", null, "brackets");
   * assertQueryEquals("\\\\", a, "\\\\"); assertQueryEquals("\\+blah", a,
   * "\\+blah"); assertQueryEquals("\\(blah", a, "\\(blah");
   *
   * assertQueryEquals("\\-blah", a, "\\-blah"); assertQueryEquals("\\!blah",
   * a, "\\!blah"); assertQueryEquals("\\{blah", a, "\\{blah");
   * assertQueryEquals("\\}blah", a, "\\}blah"); assertQueryEquals("\\:blah",
   * a, "\\:blah"); assertQueryEquals("\\^blah", a, "\\^blah");
   * assertQueryEquals("\\[blah", a, "\\[blah"); assertQueryEquals("\\]blah",
   * a, "\\]blah"); assertQueryEquals("\\\"blah", a, "\\\"blah");
   * assertQueryEquals("\\(blah", a, "\\(blah"); assertQueryEquals("\\)blah",
   * a, "\\)blah"); assertQueryEquals("\\~blah", a, "\\~blah");
   * assertQueryEquals("\\*blah", a, "\\*blah"); assertQueryEquals("\\?blah",
   * a, "\\?blah"); //assertQueryEquals("foo \\&\\& bar", a,
   * "foo \\&\\& bar"); //assertQueryEquals("foo \\|| bar", a,
   * "foo \\|| bar"); //assertQueryEquals("foo \\AND bar", a,
   * "foo \\AND bar");
   */

  assertQueryEquals(L"\\*", a, L"*");

  assertQueryEquals(L"\\a", a, L"a");

  assertQueryEquals(L"a\\-b:c", a, L"a-b:c");
  assertQueryEquals(L"a\\+b:c", a, L"a+b:c");
  assertQueryEquals(L"a\\:b:c", a, L"a:b:c");
  assertQueryEquals(L"a\\\\b:c", a, L"a\\b:c");

  assertQueryEquals(L"a:b\\-c", a, L"a:b-c");
  assertQueryEquals(L"a:b\\+c", a, L"a:b+c");
  assertQueryEquals(L"a:b\\:c", a, L"a:b:c");
  assertQueryEquals(L"a:b\\\\c", a, L"a:b\\c");

  assertQueryEquals(L"a:b\\-c*", a, L"a:b-c*");
  assertQueryEquals(L"a:b\\+c*", a, L"a:b+c*");
  assertQueryEquals(L"a:b\\:c*", a, L"a:b:c*");

  assertQueryEquals(L"a:b\\\\c*", a, L"a:b\\c*");

  assertQueryEquals(L"a:b\\-?c", a, L"a:b-?c");
  assertQueryEquals(L"a:b\\+?c", a, L"a:b+?c");
  assertQueryEquals(L"a:b\\:?c", a, L"a:b:?c");

  assertQueryEquals(L"a:b\\\\?c", a, L"a:b\\?c");

  assertQueryEquals(L"a:b\\-c~", a, L"a:b-c~2");
  assertQueryEquals(L"a:b\\+c~", a, L"a:b+c~2");
  assertQueryEquals(L"a:b\\:c~", a, L"a:b:c~2");
  assertQueryEquals(L"a:b\\\\c~", a, L"a:b\\c~2");

  // TODO: implement Range queries on QueryParser
  assertQueryEquals(L"[ a\\- TO a\\+ ]", nullptr, L"[a- TO a+]");
  assertQueryEquals(L"[ a\\: TO a\\~ ]", nullptr, L"[a: TO a~]");
  assertQueryEquals(L"[ a\\\\ TO a\\* ]", nullptr, L"[a\\ TO a*]");

  assertQueryEquals(
      L"[\"c\\:\\\\temp\\\\\\~foo0.txt\" TO \"c\\:\\\\temp\\\\\\~foo9.txt\"]",
      a, L"[c:\\temp\\~foo0.txt TO c:\\temp\\~foo9.txt]");

  assertQueryEquals(L"a\\\\\\+b", a, L"a\\+b");

  assertQueryEquals(L"a \\\"b c\\\" d", a, L"a \"b c\" d");
  assertQueryEquals(L"\"a \\\"b c\\\" d\"", a, L"\"a \"b c\" d\"");
  assertQueryEquals(L"\"a \\+b c d\"", a, L"\"a +b c d\"");

  assertQueryEquals(L"c\\:\\\\temp\\\\\\~foo.txt", a, L"c:\\temp\\~foo.txt");

  assertQueryNodeException(L"XY\\"); // there must be a character after the
  // escape char

  // test unicode escaping
  assertQueryEquals(L"a\\u0062c", a, L"abc");
  assertQueryEquals(L"XY\\u005a", a, L"XYZ");
  assertQueryEquals(L"XY\\u005A", a, L"XYZ");
  assertQueryEquals(L"\"a \\\\\\u0028\\u0062\\\" c\"", a, L"\"a \\(b\" c\"");

  assertQueryNodeException(L"XY\\u005G"); // test non-hex character in escaped
  // unicode sequence
  assertQueryNodeException(L"XY\\u005"); // test incomplete escaped unicode
  // sequence

  // Tests bug LUCENE-800
  assertQueryEquals(L"(item:\\\\ item:ABCD\\\\)", a, L"item:\\ item:ABCD\\");
  assertQueryNodeException(L"(item:\\\\ item:ABCD\\\\))"); // unmatched closing
  // paranthesis
  assertQueryEquals(L"\\*", a, L"*");
  assertQueryEquals(L"\\\\", a, L"\\"); // escaped backslash

  assertQueryNodeException(L"\\"); // a backslash must always be escaped

  // LUCENE-1189
  assertQueryEquals(L"(\"a\\\\\") or (\"b\")", a, L"a\\ or b");
}

void TestQPHelper::testQueryStringEscaping() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  assertEscapedQueryEquals(L"a-b:c", a, L"a\\-b\\:c");
  assertEscapedQueryEquals(L"a+b:c", a, L"a\\+b\\:c");
  assertEscapedQueryEquals(L"a:b:c", a, L"a\\:b\\:c");
  assertEscapedQueryEquals(L"a\\b:c", a, L"a\\\\b\\:c");

  assertEscapedQueryEquals(L"a:b-c", a, L"a\\:b\\-c");
  assertEscapedQueryEquals(L"a:b+c", a, L"a\\:b\\+c");
  assertEscapedQueryEquals(L"a:b:c", a, L"a\\:b\\:c");
  assertEscapedQueryEquals(L"a:b\\c", a, L"a\\:b\\\\c");

  assertEscapedQueryEquals(L"a:b-c*", a, L"a\\:b\\-c\\*");
  assertEscapedQueryEquals(L"a:b+c*", a, L"a\\:b\\+c\\*");
  assertEscapedQueryEquals(L"a:b:c*", a, L"a\\:b\\:c\\*");

  assertEscapedQueryEquals(L"a:b\\\\c*", a, L"a\\:b\\\\\\\\c\\*");

  assertEscapedQueryEquals(L"a:b-?c", a, L"a\\:b\\-\\?c");
  assertEscapedQueryEquals(L"a:b+?c", a, L"a\\:b\\+\\?c");
  assertEscapedQueryEquals(L"a:b:?c", a, L"a\\:b\\:\\?c");

  assertEscapedQueryEquals(L"a:b?c", a, L"a\\:b\\?c");

  assertEscapedQueryEquals(L"a:b-c~", a, L"a\\:b\\-c\\~");
  assertEscapedQueryEquals(L"a:b+c~", a, L"a\\:b\\+c\\~");
  assertEscapedQueryEquals(L"a:b:c~", a, L"a\\:b\\:c\\~");
  assertEscapedQueryEquals(L"a:b\\c~", a, L"a\\:b\\\\c\\~");

  assertEscapedQueryEquals(L"[ a - TO a+ ]", nullptr, L"\\[ a \\- TO a\\+ \\]");
  assertEscapedQueryEquals(L"[ a : TO a~ ]", nullptr, L"\\[ a \\: TO a\\~ \\]");
  assertEscapedQueryEquals(L"[ a\\ TO a* ]", nullptr, L"\\[ a\\\\ TO a\\* \\]");

  // LUCENE-881
  assertEscapedQueryEquals(L"|| abc ||", a, L"\\|\\| abc \\|\\|");
  assertEscapedQueryEquals(L"&& abc &&", a, L"\\&\\& abc \\&\\&");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("flexible queryparser shouldn't escape wildcard
// terms") public void testEscapedWildcard() throws Exception
void TestQPHelper::testEscapedWildcard() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));

  shared_ptr<WildcardQuery> q =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo\\?ba?r"));
  assertEquals(q, qp->parse(L"foo\\?ba?r", L"field"));
}

void TestQPHelper::testTabNewlineCarriageReturn() 
{
  assertQueryEqualsDOA(L"+weltbank +worlbank", nullptr, L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \n +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\r+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\r\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r\n+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r\n +worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \r \n +worlbank", nullptr,
                       L"+weltbank +worlbank");

  assertQueryEqualsDOA(L"+weltbank\t+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \t+worlbank", nullptr,
                       L"+weltbank +worlbank");
  assertQueryEqualsDOA(L"weltbank \t +worlbank", nullptr,
                       L"+weltbank +worlbank");
}

void TestQPHelper::testSimpleDAO() 
{
  assertQueryEqualsDOA(L"term term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"-term term term", nullptr, L"-term +term +term");
}

void TestQPHelper::testBoost() 
{
  shared_ptr<CharacterRunAutomaton> stopSet =
      make_shared<CharacterRunAutomaton>(Automata::makeString(L"on"));
  shared_ptr<Analyzer> oneStopAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true, stopSet);
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(oneStopAnalyzer);

  shared_ptr<Query> q = qp->parse(L"on^1.0", L"field");
  assertNotNull(q);
  q = qp->parse(L"\"hello\"^2.0", L"field");
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = qp->parse(L"hello^2.0", L"field");
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = qp->parse(L"\"on\"^1.0", L"field");
  assertNotNull(q);

  shared_ptr<StandardQueryParser> qp2 = make_shared<StandardQueryParser>();
  qp2->setAnalyzer(make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET));

  q = qp2->parse(L"the^3", L"field");
  // "the" is a stop word so the result is an empty query:
  assertNotNull(q);
  assertMatchNoDocsQuery(q);
  assertFalse(std::dynamic_pointer_cast<BoostQuery>(q) != nullptr);
}

void TestQPHelper::assertQueryNodeException(const wstring &queryString) throw(
    runtime_error)
{
  expectThrows(QueryNodeException::typeid,
               [&]() { getQuery(queryString, nullptr); });
}

void TestQPHelper::testException() 
{
  assertQueryNodeException(L"*leadingWildcard"); // disallowed by default
  assertQueryNodeException(L"\"some phrase");
  assertQueryNodeException(L"(foo bar");
  assertQueryNodeException(L"foo bar))");
  assertQueryNodeException(L"field:term:with:colon some more terms");
  assertQueryNodeException(L"(sub query)^5.0^2.0 plus more");
  assertQueryNodeException(L"secret AND illegal) AND access:confidential");
}

void TestQPHelper::testCustomQueryParserWildcard()
{
  expectThrows(QueryNodeException::typeid, [&]() {
    (make_shared<QPTestParser>(
         make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false)))
        ->parse(L"a?t", L"contents");
  });
}

void TestQPHelper::testCustomQueryParserFuzzy() 
{
  expectThrows(QueryNodeException::typeid, [&]() {
    (make_shared<QPTestParser>(
         make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false)))
        ->parse(L"xunit~", L"contents");
  });
}

void TestQPHelper::testBooleanQuery() 
{
  BooleanQuery::setMaxClauseCount(2);
  expectThrows(QueryNodeException::typeid, [&]() {
    shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
    qp->setAnalyzer(
        make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
    qp->parse(L"one two three", L"field");
  });
}

void TestQPHelper::testPrecedence() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));

  shared_ptr<Query> query1 = qp->parse(L"A AND B OR C AND D", L"field");
  shared_ptr<Query> query2 = qp->parse(L"+A +B +C +D", L"field");

  assertEquals(query1, query2);
}

void TestQPHelper::testStarParsing() 
{
  // final int[] type = new int[1];
  // StandardQueryParser qp = new StandardQueryParser("field", new
  // WhitespaceAnalyzer()) {
  // protected Query getWildcardQuery(std::wstring field, std::wstring termStr) throws
  // ParseException {
  // // override error checking of superclass
  // type[0]=1;
  // return new TermQuery(new Term(field,termStr));
  // }
  // protected Query getPrefixQuery(std::wstring field, std::wstring termStr) throws
  // ParseException {
  // // override error checking of superclass
  // type[0]=2;
  // return new TermQuery(new Term(field,termStr));
  // }
  //
  // protected Query getFieldQuery(std::wstring field, std::wstring queryText) throws
  // ParseException {
  // type[0]=3;
  // return super.getFieldQuery(field, queryText);
  // }
  // };
  //
  // TermQuery tq;
  //
  // tq = (TermQuery)qp.parse("foo:zoo*");
  // assertEquals("zoo",tq.getTerm().text());
  // assertEquals(2,type[0]);
  //
  // tq = (TermQuery)qp.parse("foo:zoo*^2");
  // assertEquals("zoo",tq.getTerm().text());
  // assertEquals(2,type[0]);
  // assertEquals(tq.getBoost(),2,0);
  //
  // tq = (TermQuery)qp.parse("foo:*");
  // assertEquals("*",tq.getTerm().text());
  // assertEquals(1,type[0]); // could be a valid prefix query in the
  // future too
  //
  // tq = (TermQuery)qp.parse("foo:*^2");
  // assertEquals("*",tq.getTerm().text());
  // assertEquals(1,type[0]);
  // assertEquals(tq.getBoost(),2,0);
  //
  // tq = (TermQuery)qp.parse("*:foo");
  // assertEquals("*",tq.getTerm().field());
  // assertEquals("foo",tq.getTerm().text());
  // assertEquals(3,type[0]);
  //
  // tq = (TermQuery)qp.parse("*:*");
  // assertEquals("*",tq.getTerm().field());
  // assertEquals("*",tq.getTerm().text());
  // assertEquals(1,type[0]); // could be handled as a prefix query in the
  // future
  //
  // tq = (TermQuery)qp.parse("(*:*)");
  // assertEquals("*",tq.getTerm().field());
  // assertEquals("*",tq.getTerm().text());
  // assertEquals(1,type[0]);
}

void TestQPHelper::testRegexps() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true));
  const wstring df = L"field";
  shared_ptr<RegexpQuery> q =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z][123]"));
  assertEquals(q, qp->parse(L"/[a-z][123]/", df));
  assertEquals(q, qp->parse(L"/[A-Z][123]/", df));
  assertEquals(make_shared<BoostQuery>(q, 0.5f),
               qp->parse(L"/[A-Z][123]/^0.5", df));
  qp->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  q->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertEquals(make_shared<BoostQuery>(q, 0.5f),
               qp->parse(L"/[A-Z][123]/^0.5", df));
  assertEquals(MultiTermQuery::SCORING_BOOLEAN_REWRITE,
               (std::static_pointer_cast<RegexpQuery>(
                    (std::static_pointer_cast<BoostQuery>(
                         qp->parse(L"/[A-Z][123]/^0.5", df)))
                        ->getQuery()))
                   ->getRewriteMethod());
  qp->setMultiTermRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);

  shared_ptr<Query> escaped =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z]\\/[123]"));
  assertEquals(escaped, qp->parse(L"/[a-z]\\/[123]/", df));
  shared_ptr<Query> escaped2 =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z]\\*[123]"));
  assertEquals(escaped2, qp->parse(L"/[a-z]\\*[123]/", df));

  shared_ptr<BooleanQuery::Builder> complex =
      make_shared<BooleanQuery::Builder>();
  complex->add(
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z]\\/[123]")),
      Occur::MUST);
  complex->add(
      make_shared<TermQuery>(make_shared<Term>(L"path", L"/etc/init.d/")),
      Occur::MUST);
  complex->add(make_shared<TermQuery>(
                   make_shared<Term>(L"field", L"/etc/init[.]d/lucene/")),
               Occur::SHOULD);
  assertEquals(complex->build(),
               qp->parse(L"/[a-z]\\/[123]/ AND path:\"/etc/init.d/\" OR "
                         L"\"/etc\\/init\\[.\\]d/lucene/\" ",
                         df));

  shared_ptr<Query> re =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"http.*"));
  assertEquals(re, qp->parse(L"field:/http.*/", df));
  assertEquals(re, qp->parse(L"/http.*/", df));

  re = make_shared<RegexpQuery>(make_shared<Term>(L"field", L"http~0.5"));
  assertEquals(re, qp->parse(L"field:/http~0.5/", df));
  assertEquals(re, qp->parse(L"/http~0.5/", df));

  re = make_shared<RegexpQuery>(make_shared<Term>(L"field", L"boo"));
  assertEquals(re, qp->parse(L"field:/boo/", df));
  assertEquals(re, qp->parse(L"/boo/", df));

  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"field", L"/boo/")),
               qp->parse(L"\"/boo/\"", df));
  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"field", L"/boo/")),
               qp->parse(L"\\/boo\\/", df));

  shared_ptr<BooleanQuery::Builder> two = make_shared<BooleanQuery::Builder>();
  two->add(make_shared<RegexpQuery>(make_shared<Term>(L"field", L"foo")),
           Occur::SHOULD);
  two->add(make_shared<RegexpQuery>(make_shared<Term>(L"field", L"bar")),
           Occur::SHOULD);
  assertEquals(two->build(), qp->parse(L"field:/foo/ field:/bar/", df));
  assertEquals(two->build(), qp->parse(L"/foo/ /bar/", df));
}

void TestQPHelper::testStopwords() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  shared_ptr<CharacterRunAutomaton> stopSet =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"the|foo"))->toAutomaton());
  qp->setAnalyzer(make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                            true, stopSet));

  shared_ptr<Query> result = qp->parse(L"a:the OR a:foo", L"a");
  assertNotNull(L"result is null and it shouldn't be", result);
  assertTrue(L"result is not a MatchNoDocsQuery",
             std::dynamic_pointer_cast<MatchNoDocsQuery>(result) != nullptr);
  result = qp->parse(L"a:woo OR a:the", L"a");
  assertNotNull(L"result is null and it shouldn't be", result);
  assertTrue(L"result is not a TermQuery",
             std::dynamic_pointer_cast<TermQuery>(result) != nullptr);
  result = qp->parse(
      L"(fieldX:xxxxx OR fieldy:xxxxxxxx)^2 AND (fieldx:the OR fieldy:foo)",
      L"a");
  shared_ptr<Query> expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"fieldX", L"xxxxx")),
                Occur::SHOULD)
          ->add(
              make_shared<TermQuery>(make_shared<Term>(L"fieldy", L"xxxxxxxx")),
              Occur::SHOULD)
          ->build();
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, result);
}

void TestQPHelper::testPositionIncrement() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET));

  qp->setEnablePositionIncrements(true);

  wstring qtxt =
      L"\"the words in poisitions pos02578 are stopped in this phrasequery\"";
  // 0 2 5 7 8
  std::deque<int> expectedPositions = {1, 3, 4, 6, 9};
  shared_ptr<PhraseQuery> pq =
      std::static_pointer_cast<PhraseQuery>(qp->parse(qtxt, L"a"));
  // System.out.println("Query text: "+qtxt);
  // System.out.println("Result: "+pq);
  std::deque<std::shared_ptr<Term>> t = pq->getTerms();
  std::deque<int> pos = pq->getPositions();
  for (int i = 0; i < t.size(); i++) {
    // System.out.println(i+". "+t[i]+"  pos: "+pos[i]);
    assertEquals(L"term " + to_wstring(i) + L" = " + t[i] +
                     L" has wrong term-position!",
                 expectedPositions[i], pos[i]);
  }
}

void TestQPHelper::testMatchAllDocs() 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));

  assertEquals(make_shared<MatchAllDocsQuery>(), qp->parse(L"*:*", L"field"));
  assertEquals(make_shared<MatchAllDocsQuery>(), qp->parse(L"(*:*)", L"field"));
  shared_ptr<BooleanQuery> bq =
      std::static_pointer_cast<BooleanQuery>(qp->parse(L"+*:* -*:*", L"field"));
  for (auto c : bq) {
    assertTrue(c->getQuery()->getClass() == MatchAllDocsQuery::typeid);
  }
}

void TestQPHelper::assertHits(
    int expected, const wstring &query,
    shared_ptr<IndexSearcher> is) 
{
  shared_ptr<StandardQueryParser> qp = make_shared<StandardQueryParser>();
  qp->setAnalyzer(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  qp->setLocale(Locale::ENGLISH);

  shared_ptr<Query> q = qp->parse(query, L"date");
  std::deque<std::shared_ptr<ScoreDoc>> hits = is->search(q, 1000)->scoreDocs;
  assertEquals(expected, hits.size());
}

void TestQPHelper::tearDown() 
{
  BooleanQuery::setMaxClauseCount(originalMaxClauses);
  LuceneTestCase::tearDown();
}

TestQPHelper::CannedTokenizer::CannedTokenizer()
    : org::apache::lucene::analysis::Tokenizer()
{
}

bool TestQPHelper::CannedTokenizer::incrementToken()
{
  clearAttributes();
  if (upto == 4) {
    return false;
  }
  if (upto == 0) {
    posIncr->setPositionIncrement(1);
    term->setEmpty()->append(L"a");
  } else if (upto == 1) {
    posIncr->setPositionIncrement(1);
    term->setEmpty()->append(L"b");
  } else if (upto == 2) {
    posIncr->setPositionIncrement(0);
    term->setEmpty()->append(L"c");
  } else {
    posIncr->setPositionIncrement(0);
    term->setEmpty()->append(L"d");
  }
  upto++;
  return true;
}

void TestQPHelper::CannedTokenizer::reset() 
{
  Tokenizer::reset();
  this->upto = 0;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQPHelper::CannedAnalyzer::createComponents(const wstring &ignored)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<CannedTokenizer>());
}

void TestQPHelper::testMultiPhraseQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<CannedAnalyzer>()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<Query> q =
      (make_shared<StandardQueryParser>(make_shared<CannedAnalyzer>()))
          ->parse(L"\"a\"", L"field");
  assertTrue(std::dynamic_pointer_cast<MultiPhraseQuery>(q) != nullptr);
  assertEquals(1, s->search(q, 10)->totalHits);
  delete r;
  delete w;
  delete dir;
}

void TestQPHelper::testRegexQueryParsing() 
{
  const std::deque<wstring> fields = {L"b", L"t"};

  shared_ptr<StandardQueryParser> *const parser =
      make_shared<StandardQueryParser>();
  parser->setMultiFields(fields);
  parser->setDefaultOperator(StandardQueryConfigHandler::Operator::AND);
  parser->setAnalyzer(make_shared<MockAnalyzer>(random()));

  shared_ptr<BooleanQuery::Builder> exp = make_shared<BooleanQuery::Builder>();
  exp->add(make_shared<BooleanClause>(
      make_shared<RegexpQuery>(make_shared<Term>(L"b", L"ab.+")),
      Occur::SHOULD)); // TODO spezification? was "MUST"
  exp->add(make_shared<BooleanClause>(
      make_shared<RegexpQuery>(make_shared<Term>(L"t", L"ab.+")),
      Occur::SHOULD)); // TODO spezification? was "MUST"

  assertEquals(exp->build(), parser->parse(L"/ab.+/", L""));

  shared_ptr<RegexpQuery> regexpQueryexp =
      make_shared<RegexpQuery>(make_shared<Term>(L"test", L"[abc]?[0-9]"));

  assertEquals(regexpQueryexp, parser->parse(L"test:/[abc]?[0-9]/", L""));
}
} // namespace org::apache::lucene::queryparser::flexible::standard