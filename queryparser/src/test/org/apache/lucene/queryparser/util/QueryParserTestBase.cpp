using namespace std;

#include "QueryParserTestBase.h"

namespace org::apache::lucene::queryparser::util
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using DateTools = org::apache::lucene::document::DateTools;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using QueryParserBase =
    org::apache::lucene::queryparser::classic::QueryParserBase;
using TestQueryParser =
    org::apache::lucene::queryparser::classic::TestQueryParser;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using namespace org::apache::lucene::search;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<Analyzer> QueryParserTestBase::qpAnalyzer;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass()
void QueryParserTestBase::beforeClass()
{
  qpAnalyzer = make_shared<QPTestAnalyzer>();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void QueryParserTestBase::afterClass() { qpAnalyzer.reset(); }

QueryParserTestBase::QPTestFilter::QPTestFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
  termAtt = addAttribute(CharTermAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);
}

bool QueryParserTestBase::QPTestFilter::incrementToken() 
{
  if (inPhrase) {
    inPhrase = false;
    clearAttributes();
    termAtt->append(L"phrase2");
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

shared_ptr<TokenStreamComponents>
QueryParserTestBase::QPTestAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<QPTestFilter>(tokenizer));
}

wstring QueryParserTestBase::getDefaultField() { return defaultField; }

void QueryParserTestBase::setDefaultField(const wstring &defaultField)
{
  this->defaultField = defaultField;
}

void QueryParserTestBase::setUp() 
{
  LuceneTestCase::setUp();
  originalMaxClauses = BooleanQuery::getMaxClauseCount();
}

shared_ptr<Query>
QueryParserTestBase::getQuery(const wstring &query) 
{
  return getQuery(query, std::static_pointer_cast<Analyzer>(nullptr));
}

void QueryParserTestBase::assertQueryEquals(
    const wstring &query, shared_ptr<Analyzer> a,
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

void QueryParserTestBase::assertMatchNoDocsQuery(
    const wstring &queryString, shared_ptr<Analyzer> a) 
{
  assertMatchNoDocsQuery(getQuery(queryString, a));
}

void QueryParserTestBase::assertMatchNoDocsQuery(shared_ptr<Query> query) throw(
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

void QueryParserTestBase::assertQueryEquals(
    shared_ptr<CommonQueryParserConfiguration> cqpC, const wstring &field,
    const wstring &query, const wstring &result) 
{
  shared_ptr<Query> q = getQuery(query, cqpC);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(field);
  if (s != result) {
    fail(L"Query /" + query + L"/ yielded /" + s + L"/, expecting /" + result +
         L"/");
  }
}

void QueryParserTestBase::assertEscapedQueryEquals(
    const wstring &query, shared_ptr<Analyzer> a,
    const wstring &result) 
{
  wstring escapedQuery = QueryParserBase::escape(query);
  if (escapedQuery != result) {
    fail(L"Query /" + query + L"/ yielded /" + escapedQuery +
         L"/, expecting /" + result + L"/");
  }
}

void QueryParserTestBase::assertWildcardQueryEquals(
    const wstring &query, const wstring &result,
    bool allowLeadingWildcard) 
{
  shared_ptr<CommonQueryParserConfiguration> cqpC = getParserConfig(nullptr);
  cqpC->setAllowLeadingWildcard(allowLeadingWildcard);
  shared_ptr<Query> q = getQuery(query, cqpC);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = q->toString(L"field");
  if (s != result) {
    fail(L"WildcardQuery /" + query + L"/ yielded /" + s + L"/, expecting /" +
         result + L"/");
  }
}

void QueryParserTestBase::assertWildcardQueryEquals(
    const wstring &query, const wstring &result) 
{
  assertWildcardQueryEquals(query, result, false);
}

shared_ptr<Query>
QueryParserTestBase::getQueryDOA(const wstring &query,
                                 shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(a);
  setDefaultOperatorAND(qp);
  return getQuery(query, qp);
}

void QueryParserTestBase::assertQueryEqualsDOA(
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

void QueryParserTestBase::testCJK() 
{
  // Test Ideographic Space - As wide as a CJK character cell (fullwidth)
  // used google to translate the word "term" to japanese -> 用語
  assertQueryEquals(L"term\u3000term\u3000term", nullptr,
                    L"term\u0020term\u0020term");
  assertQueryEquals(L"用語\u3000用語\u3000用語", nullptr,
                    L"用語\u0020用語\u0020用語");
}

QueryParserTestBase::SimpleCJKTokenizer::SimpleCJKTokenizer() : Tokenizer() {}

bool QueryParserTestBase::SimpleCJKTokenizer::incrementToken() throw(
    IOException)
{
  int ch = input->read();
  if (ch < 0) {
    return false;
  }
  clearAttributes();
  termAtt->setEmpty()->append(static_cast<wchar_t>(ch));
  return true;
}

shared_ptr<TokenStreamComponents>
QueryParserTestBase::SimpleCJKAnalyzer::createComponents(
    const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(make_shared<SimpleCJKTokenizer>());
}

void QueryParserTestBase::testCJKTerm() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
                BooleanClause::Occur::SHOULD);

  assertEquals(expected->build(), getQuery(L"中国", analyzer));
}

void QueryParserTestBase::testCJKBoostedTerm() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                 BooleanClause::Occur::SHOULD);
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
                 BooleanClause::Occur::SHOULD);
  shared_ptr<Query> expected = expectedB->build();
  expected = make_shared<BoostQuery>(expected, 0.5f);

  assertEquals(expected, getQuery(L"中国^0.5", analyzer));
}

void QueryParserTestBase::testCJKPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"中", L"国");

  assertEquals(expected, getQuery(L"\"中国\"", analyzer));
}

void QueryParserTestBase::testCJKBoostedPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<Query> expected = make_shared<PhraseQuery>(L"field", L"中", L"国");
  expected = make_shared<BoostQuery>(expected, 0.5f);

  assertEquals(expected, getQuery(L"\"中国\"^0.5", analyzer));
}

void QueryParserTestBase::testCJKSloppyPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(3, L"field", L"中", L"国");

  assertEquals(expected, getQuery(L"\"中国\"~3", analyzer));
}

void QueryParserTestBase::testAutoGeneratePhraseQueriesOn() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"中", L"国");
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(analyzer);
  if (std::dynamic_pointer_cast<QueryParser>(qp) !=
      nullptr) { // Always true, since TestStandardQP overrides this method
    (std::static_pointer_cast<QueryParser>(qp))
        ->setSplitOnWhitespace(true); // LUCENE-7533
  }
  setAutoGeneratePhraseQueries(qp, true);
  assertEquals(expected, getQuery(L"中国", qp));
}

void QueryParserTestBase::testSimple() 
{
  assertQueryEquals(L"term term term", nullptr, L"term term term");
  assertQueryEquals(L"türm term term", make_shared<MockAnalyzer>(random()),
                    L"türm term term");
  assertQueryEquals(L"ümlaut", make_shared<MockAnalyzer>(random()), L"ümlaut");

  // FIXME: enhance MockAnalyzer to be able to support this
  // it must no longer extend CharTokenizer
  // assertQueryEquals("\"\"", new KeywordAnalyzer(), "");
  // assertQueryEquals("foo:\"\"", new KeywordAnalyzer(), "foo:");

  assertQueryEquals(L"a AND b", nullptr, L"+a +b");
  assertQueryEquals(L"(a AND b)", nullptr, L"+a +b");
  assertQueryEquals(L"c OR (a AND b)", nullptr, L"c (+a +b)");
  assertQueryEquals(L"a AND NOT b", nullptr, L"+a -b");
  assertQueryEquals(L"a AND -b", nullptr, L"+a -b");
  assertQueryEquals(L"a AND !b", nullptr, L"+a -b");
  assertQueryEquals(L"a && b", nullptr, L"+a +b");
  //    assertQueryEquals("a && ! b", null, "+a -b");

  assertQueryEquals(L"a OR b", nullptr, L"a b");
  assertQueryEquals(L"a || b", nullptr, L"a b");
  assertQueryEquals(L"a OR !b", nullptr, L"a -b");
  //    assertQueryEquals("a OR ! b", null, "a -b");
  assertQueryEquals(L"a OR -b", nullptr, L"a -b");

  assertQueryEquals(L"+term -term term", nullptr, L"+term -term term");
  assertQueryEquals(L"foo:term AND field:anotherTerm", nullptr,
                    L"+foo:term +anotherterm");
  assertQueryEquals(L"term AND \"phrase phrase\"", nullptr,
                    L"+term +\"phrase phrase\"");
  assertQueryEquals(L"\"hello there\"", nullptr, L"\"hello there\"");
  assertTrue(std::dynamic_pointer_cast<BooleanQuery>(getQuery(L"a AND b")) !=
             nullptr);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(getQuery(L"hello")) !=
             nullptr);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(
                 getQuery(L"\"hello there\"")) != nullptr);

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

void QueryParserTestBase::testOperatorVsWhitespace() 
{ // LUCENE-2566
  // +,-,! should be directly adjacent to operand (i.e. not separated by
  // whitespace) to be treated as an operator
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  assertQueryEquals(L"a - b", a, L"a - b");
  assertQueryEquals(L"a + b", a, L"a + b");
  assertQueryEquals(L"a ! b", a, L"a ! b");
}

QueryParserTestBase::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<QueryParserTestBase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TokenStreamComponents>
QueryParserTestBase::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

void QueryParserTestBase::testPunct() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"a&b", a, L"a&b");
  assertQueryEquals(L"a&&b", a, L"a&&b");
  assertQueryEquals(L".NET", a, L".NET");
}

void QueryParserTestBase::testSlop() 
{
  assertQueryEquals(L"\"term germ\"~2", nullptr, L"\"term germ\"~2");
  assertQueryEquals(L"\"term germ\"~2 flork", nullptr,
                    L"\"term germ\"~2 flork");
  assertQueryEquals(L"\"term\"~2", nullptr, L"term");
  assertQueryEquals(L"\" \"~2 germ", nullptr, L"germ");
  assertQueryEquals(L"\"term germ\"~2^2", nullptr, L"(\"term germ\"~2)^2.0");
}

void QueryParserTestBase::testNumber() 
{
  // The numbers go away because SimpleAnalzyer ignores them
  assertMatchNoDocsQuery(L"3", nullptr);
  assertQueryEquals(L"term 1.0 1 2", nullptr, L"term");
  assertQueryEquals(L"term term1 term2", nullptr, L"term term term");

  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true);
  assertQueryEquals(L"3", a, L"3");
  assertQueryEquals(L"term 1.0 1 2", a, L"term 1.0 1 2");
  assertQueryEquals(L"term term1 term2", a, L"term term1 term2");
}

void QueryParserTestBase::testWildcard() 
{
  assertQueryEquals(L"term*", nullptr, L"term*");
  assertQueryEquals(L"term*^2", nullptr, L"(term*)^2.0");
  assertQueryEquals(L"term~", nullptr, L"term~2");
  assertQueryEquals(L"term~1", nullptr, L"term~1");
  assertQueryEquals(L"term~0.7", nullptr, L"term~1");
  assertQueryEquals(L"term~^3", nullptr, L"(term~2)^3.0");
  assertQueryEquals(L"term^3~", nullptr, L"(term~2)^3.0");
  assertQueryEquals(L"term*germ", nullptr, L"term*germ");
  assertQueryEquals(L"term*germ^3", nullptr, L"(term*germ)^3.0");

  assertTrue(std::dynamic_pointer_cast<PrefixQuery>(getQuery(L"term*")) !=
             nullptr);
  assertTrue(std::dynamic_pointer_cast<BoostQuery>(getQuery(L"term*^2")) !=
             nullptr);
  assertTrue(std::dynamic_pointer_cast<PrefixQuery>(
                 (std::static_pointer_cast<BoostQuery>(getQuery(L"term*^2")))
                     ->getQuery()) != nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(getQuery(L"term~")) !=
             nullptr);
  assertTrue(std::dynamic_pointer_cast<FuzzyQuery>(getQuery(L"term~0.7")) !=
             nullptr);
  shared_ptr<FuzzyQuery> fq =
      std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~0.7"));
  assertEquals(1, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());
  fq = std::static_pointer_cast<FuzzyQuery>(getQuery(L"term~"));
  assertEquals(2, fq->getMaxEdits());
  assertEquals(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());

  assertParseException(L"term~1.1"); // value > 1, throws exception

  assertTrue(std::dynamic_pointer_cast<WildcardQuery>(getQuery(L"term*germ")) !=
             nullptr);

  /* Tests to see that wild card terms are (or are not) properly
   * lower-cased with propery parser configuration
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
  //  Fuzzy queries:
  assertWildcardQueryEquals(L"Term~", L"term~2");
  //  Range queries:
  assertWildcardQueryEquals(L"[A TO C]", L"[a TO c]");
  // Test suffix queries: first disallow
  try {
    assertWildcardQueryEquals(L"*Term", L"*term", false);
  } catch (const runtime_error &pe) {
    // expected exception
    if (!isQueryParserException(pe)) {
      fail();
    }
  }
  try {
    assertWildcardQueryEquals(L"?Term", L"?term");
    fail();
  } catch (const runtime_error &pe) {
    // expected exception
    if (!isQueryParserException(pe)) {
      fail();
    }
  }
  // Test suffix queries: then allow
  assertWildcardQueryEquals(L"*Term", L"*term", true);
  assertWildcardQueryEquals(L"?Term", L"?term", true);
}

void QueryParserTestBase::testLeadingWildcardType() 
{
  shared_ptr<CommonQueryParserConfiguration> cqpC = getParserConfig(nullptr);
  cqpC->setAllowLeadingWildcard(true);
  assertEquals(WildcardQuery::typeid, getQuery(L"t*erm*", cqpC)->getClass());
  assertEquals(WildcardQuery::typeid, getQuery(L"?term*", cqpC)->getClass());
  assertEquals(WildcardQuery::typeid, getQuery(L"*term*", cqpC)->getClass());
}

void QueryParserTestBase::testQPA() 
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

  // TODO: Re-enable once flexible standard parser gets multi-word synonym
  // support
  //    assertQueryEquals("term phrase term", qpAnalyzer,
  //                      "term phrase1 phrase2 term");
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

  shared_ptr<CommonQueryParserConfiguration> cqpc = getParserConfig(qpAnalyzer);
  setDefaultOperatorAND(cqpc);
  // TODO: Re-enable once flexible standard parser gets multi-word synonym
  // support
  //    assertQueryEquals(cqpc, "field", "term phrase term",
  //        "+term +phrase1 +phrase2 +term");
  assertQueryEquals(cqpc, L"field", L"phrase", L"+phrase1 +phrase2");
}

void QueryParserTestBase::testRange() 
{
  assertQueryEquals(L"[ a TO z]", nullptr, L"[a TO z]");
  assertQueryEquals(L"[ a TO z}", nullptr, L"[a TO z}");
  assertQueryEquals(L"{ a TO z]", nullptr, L"{a TO z]");

  assertEquals(
      MultiTermQuery::CONSTANT_SCORE_REWRITE,
      (std::static_pointer_cast<TermRangeQuery>(getQuery(L"[ a TO z]")))
          ->getRewriteMethod());

  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true));

  qp->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertEquals(
      MultiTermQuery::SCORING_BOOLEAN_REWRITE,
      (std::static_pointer_cast<TermRangeQuery>(getQuery(L"[ a TO z]", qp)))
          ->getRewriteMethod());

  // test open ranges
  assertQueryEquals(L"[ a TO * ]", nullptr, L"[a TO *]");
  assertQueryEquals(L"[ * TO z ]", nullptr, L"[* TO z]");
  assertQueryEquals(L"[ * TO * ]", nullptr, L"[* TO *]");

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

  assertQueryEquals(L"[* TO Z]", nullptr, L"[* TO z]");
  assertQueryEquals(L"[A TO *]", nullptr, L"[a TO *]");
  assertQueryEquals(L"[* TO *]", nullptr, L"[* TO *]");
}

void QueryParserTestBase::testRangeWithPhrase() 
{
  assertQueryEquals(L"[\\* TO \"*\"]", nullptr, L"[\\* TO \\*]");
  assertQueryEquals(L"[\"*\" TO *]", nullptr, L"[\\* TO *]");
}

void QueryParserTestBase::testRangeQueryEndpointTO() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());
  assertQueryEquals(L"[to TO to]", a, L"[to TO to]");
  assertQueryEquals(L"[to TO TO]", a, L"[to TO to]");
  assertQueryEquals(L"[TO TO to]", a, L"[to TO to]");
  assertQueryEquals(L"[TO TO TO]", a, L"[to TO to]");

  assertQueryEquals(L"[\"TO\" TO \"TO\"]", a, L"[to TO to]");
  assertQueryEquals(L"[\"TO\" TO TO]", a, L"[to TO to]");
  assertQueryEquals(L"[TO TO \"TO\"]", a, L"[to TO to]");

  assertQueryEquals(L"[to TO xx]", a, L"[to TO xx]");
  assertQueryEquals(L"[\"TO\" TO xx]", a, L"[to TO xx]");
  assertQueryEquals(L"[TO TO xx]", a, L"[to TO xx]");

  assertQueryEquals(L"[xx TO to]", a, L"[xx TO to]");
  assertQueryEquals(L"[xx TO \"TO\"]", a, L"[xx TO to]");
  assertQueryEquals(L"[xx TO TO]", a, L"[xx TO to]");
}

void QueryParserTestBase::testRangeQueryRequiresTO() 
{
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(random());

  assertQueryEquals(L"{A TO B}", a, L"{a TO b}");
  assertQueryEquals(L"[A TO B}", a, L"[a TO b}");
  assertQueryEquals(L"{A TO B]", a, L"{a TO b]");
  assertQueryEquals(L"[A TO B]", a, L"[a TO b]");

  // " TO " is required between range endpoints

  type_info exceptionClass =
      std::dynamic_pointer_cast<TestQueryParser>(shared_from_this()) != nullptr
          ? org::apache::lucene::queryparser::classic::ParseException::typeid
          : org::apache::lucene::queryparser::flexible::standard::parser::
                ParseException::typeid;

  expectThrows(exceptionClass, [&]() { getQuery(L"{A B}"); });
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"[A B}"));
      };
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"{A B]"));
      };
      expectThrows(exceptionClass, [&] ()
      {
    getQuery(L"[A B]");
      });

      expectThrows(exceptionClass, [&] ()
      {
    getQuery(L"{TO B}");
      });
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"[TO B}"));
      };
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"{TO B]"));
      };
      expectThrows(exceptionClass, [&] ()
      {
    getQuery(L"[TO B]");
      });

      expectThrows(exceptionClass, [&] ()
      {
    getQuery(L"{A TO}");
      });
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"[A TO}"));
      };
      expectThrows(exceptionClass, [&] ()
      {
      getQuery(L"{A TO]"));
      };
      expectThrows(exceptionClass, [&] ()
      {
    getQuery(L"[A TO]");
      });
}

wstring QueryParserTestBase::escapeDateString(const wstring &s)
{
  if (s.find(L" ") != wstring::npos) {
    return L"\"" + s + L"\"";
  } else {
    return s;
  }
}

wstring QueryParserTestBase::getDate(
    const wstring &s, DateTools::Resolution resolution) 
{
  // we use the default Locale since LuceneTestCase randomizes it
  shared_ptr<DateFormat> df =
      DateFormat::getDateInstance(DateFormat::SHORT, Locale::getDefault());
  return getDate(df->parse(s), resolution);
}

wstring QueryParserTestBase::getDate(Date d, DateTools::Resolution resolution)
{
  return DateTools::dateToString(d, resolution);
}

wstring QueryParserTestBase::getLocalizedDate(int year, int month, int day)
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

void QueryParserTestBase::testDateRange() 
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
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(a);

  // set a field specific date resolution
  setDateResolution(qp, monthField, DateTools::Resolution::MONTH);

  // set default date resolution to MILLISECOND
  qp->setDateResolution(DateTools::Resolution::MILLISECOND);

  // set second field specific date resolution
  setDateResolution(qp, hourField, DateTools::Resolution::HOUR);

  // for this field no field specific date resolution has been set,
  // so verify if the default resolution is used
  assertDateRangeQueryEquals(qp, defaultField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MILLISECOND);

  // verify if field specific date resolutions are used for these two fields
  assertDateRangeQueryEquals(qp, monthField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::MONTH);

  assertDateRangeQueryEquals(qp, hourField, startDate, endDate,
                             endDateExpected->getTime(),
                             DateTools::Resolution::HOUR);
}

void QueryParserTestBase::assertDateRangeQueryEquals(
    shared_ptr<CommonQueryParserConfiguration> cqpC, const wstring &field,
    const wstring &startDate, const wstring &endDate, Date endDateInclusive,
    DateTools::Resolution resolution) 
{
  assertQueryEquals(cqpC, field,
                    field + L":[" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"]",
                    L"[" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDateInclusive, resolution) + L"]");
  assertQueryEquals(cqpC, field,
                    field + L":{" + escapeDateString(startDate) + L" TO " +
                        escapeDateString(endDate) + L"}",
                    L"{" + getDate(startDate, resolution) + L" TO " +
                        getDate(endDate, resolution) + L"}");
}

void QueryParserTestBase::testEscaped() 
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);

  /*assertQueryEquals("\\[brackets", a, "\\[brackets");
  assertQueryEquals("\\[brackets", null, "brackets");
  assertQueryEquals("\\\\", a, "\\\\");
  assertQueryEquals("\\+blah", a, "\\+blah");
  assertQueryEquals("\\(blah", a, "\\(blah");

  assertQueryEquals("\\-blah", a, "\\-blah");
  assertQueryEquals("\\!blah", a, "\\!blah");
  assertQueryEquals("\\{blah", a, "\\{blah");
  assertQueryEquals("\\}blah", a, "\\}blah");
  assertQueryEquals("\\:blah", a, "\\:blah");
  assertQueryEquals("\\^blah", a, "\\^blah");
  assertQueryEquals("\\[blah", a, "\\[blah");
  assertQueryEquals("\\]blah", a, "\\]blah");
  assertQueryEquals("\\\"blah", a, "\\\"blah");
  assertQueryEquals("\\(blah", a, "\\(blah");
  assertQueryEquals("\\)blah", a, "\\)blah");
  assertQueryEquals("\\~blah", a, "\\~blah");
  assertQueryEquals("\\*blah", a, "\\*blah");
  assertQueryEquals("\\?blah", a, "\\?blah");
  //assertQueryEquals("foo \\&\\& bar", a, "foo \\&\\& bar");
  //assertQueryEquals("foo \\|| bar", a, "foo \\|| bar");
  //assertQueryEquals("foo \\AND bar", a, "foo \\AND bar");*/

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

  assertQueryEquals(L"a:b\\-c~", a, L"a:b-c~2");
  assertQueryEquals(L"a:b\\+c~", a, L"a:b+c~2");
  assertQueryEquals(L"a:b\\:c~", a, L"a:b:c~2");
  assertQueryEquals(L"a:b\\\\c~", a, L"a:b\\c~2");

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

  assertParseException(
      L"XY\\"); // there must be a character after the escape char

  // test unicode escaping
  assertQueryEquals(L"a\\u0062c", a, L"abc");
  assertQueryEquals(L"XY\\u005a", a, L"XYZ");
  assertQueryEquals(L"XY\\u005A", a, L"XYZ");
  assertQueryEquals(L"\"a \\\\\\u0028\\u0062\\\" c\"", a, L"\"a \\(b\" c\"");

  assertParseException(
      L"XY\\u005G"); // test non-hex character in escaped unicode sequence
  assertParseException(L"XY\\u005"); // test incomplete escaped unicode sequence

  // Tests bug LUCENE-800
  assertQueryEquals(L"(item:\\\\ item:ABCD\\\\)", a, L"item:\\ item:ABCD\\");
  assertParseException(
      L"(item:\\\\ item:ABCD\\\\))"); // unmatched closing paranthesis
  assertQueryEquals(L"\\*", a, L"*");
  assertQueryEquals(L"\\\\", a, L"\\"); // escaped backslash

  assertParseException(L"\\"); // a backslash must always be escaped

  // LUCENE-1189
  assertQueryEquals(L"(\"a\\\\\") or (\"b\")", a, L"a\\ or b");
}

void QueryParserTestBase::testEscapedVsQuestionMarkAsWildcard() throw(
    runtime_error)
{
  shared_ptr<Analyzer> a =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertQueryEquals(L"a:b\\-?c", a, L"a:b\\-?c");
  assertQueryEquals(L"a:b\\+?c", a, L"a:b\\+?c");
  assertQueryEquals(L"a:b\\:?c", a, L"a:b\\:?c");

  assertQueryEquals(L"a:b\\\\?c", a, L"a:b\\\\?c");
}

void QueryParserTestBase::testQueryStringEscaping() 
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

void QueryParserTestBase::testTabNewlineCarriageReturn() 
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

void QueryParserTestBase::testSimpleDAO() 
{
  assertQueryEqualsDOA(L"term term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"term +term +term", nullptr, L"+term +term +term");
  assertQueryEqualsDOA(L"-term term term", nullptr, L"-term +term +term");
}

void QueryParserTestBase::testBoost() 
{
  shared_ptr<CharacterRunAutomaton> stopWords =
      make_shared<CharacterRunAutomaton>(Automata::makeString(L"on"));
  shared_ptr<Analyzer> oneStopAnalyzer = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, stopWords);
  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(oneStopAnalyzer);
  shared_ptr<Query> q = getQuery(L"on^1.0", qp);
  assertNotNull(q);
  q = getQuery(L"\"hello\"^2.0", qp);
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = getQuery(L"hello^2.0", qp);
  assertNotNull(q);
  assertEquals((std::static_pointer_cast<BoostQuery>(q))->getBoost(),
               static_cast<float>(2.0), static_cast<float>(0.5));
  q = getQuery(L"\"on\"^1.0", qp);
  assertNotNull(q);

  shared_ptr<Analyzer> a2 = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<CommonQueryParserConfiguration> qp2 = getParserConfig(a2);
  q = getQuery(L"the^3", qp2);
  // "the" is a stop word so the result is an empty query:
  assertNotNull(q);
  assertMatchNoDocsQuery(q);
  assertFalse(std::dynamic_pointer_cast<BoostQuery>(q) != nullptr);
}

void QueryParserTestBase::assertParseException(
    const wstring &queryString) 
{
  try {
    getQuery(queryString);
  } catch (const runtime_error &expected) {
    if (isQueryParserException(expected)) {
      return;
    }
  }
  fail(L"ParseException expected, not thrown");
}

void QueryParserTestBase::assertParseException(
    const wstring &queryString, shared_ptr<Analyzer> a) 
{
  try {
    getQuery(queryString, a);
  } catch (const runtime_error &expected) {
    if (isQueryParserException(expected)) {
      return;
    }
  }
  fail(L"ParseException expected, not thrown");
}

void QueryParserTestBase::testException() 
{
  assertParseException(L"\"some phrase");
  assertParseException(L"(foo bar");
  assertParseException(L"foo bar))");
  assertParseException(L"field:term:with:colon some more terms");
  assertParseException(L"(sub query)^5.0^2.0 plus more");
  assertParseException(L"secret AND illegal) AND access:confidential");
}

void QueryParserTestBase::testBooleanQuery() 
{
  BooleanQuery::setMaxClauseCount(2);
  shared_ptr<Analyzer> purWhitespaceAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  assertParseException(L"one two three", purWhitespaceAnalyzer);
}

void QueryParserTestBase::testPrecedence() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  shared_ptr<Query> query1 = getQuery(L"A AND B OR C AND D", qp);
  shared_ptr<Query> query2 = getQuery(L"+A +B +C +D", qp);
  assertEquals(query1, query2);
}

void QueryParserTestBase::testEscapedWildcard() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  shared_ptr<WildcardQuery> q =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"foo\\?ba?r"));
  assertEquals(q, getQuery(L"foo\\?ba?r", qp));
}

void QueryParserTestBase::testRegexps() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true));
  shared_ptr<RegexpQuery> q =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z][123]"));
  assertEquals(q, getQuery(L"/[a-z][123]/", qp));
  assertEquals(q, getQuery(L"/[A-Z][123]/", qp));
  assertEquals(make_shared<BoostQuery>(q, 0.5f),
               getQuery(L"/[A-Z][123]/^0.5", qp));
  qp->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  q->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  assertTrue(std::dynamic_pointer_cast<BoostQuery>(
                 getQuery(L"/[A-Z][123]/^0.5", qp)) != nullptr);
  assertTrue(std::dynamic_pointer_cast<RegexpQuery>(
                 (std::static_pointer_cast<BoostQuery>(
                      getQuery(L"/[A-Z][123]/^0.5", qp)))
                     ->getQuery()) != nullptr);
  assertEquals(MultiTermQuery::SCORING_BOOLEAN_REWRITE,
               (std::static_pointer_cast<RegexpQuery>(
                    (std::static_pointer_cast<BoostQuery>(
                         getQuery(L"/[A-Z][123]/^0.5", qp)))
                        ->getQuery()))
                   ->getRewriteMethod());
  assertEquals(make_shared<BoostQuery>(q, 0.5f),
               getQuery(L"/[A-Z][123]/^0.5", qp));
  qp->setMultiTermRewriteMethod(MultiTermQuery::CONSTANT_SCORE_REWRITE);

  shared_ptr<Query> escaped =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z]\\/[123]"));
  assertEquals(escaped, getQuery(L"/[a-z]\\/[123]/", qp));
  shared_ptr<Query> escaped2 =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"[a-z]\\*[123]"));
  assertEquals(escaped2, getQuery(L"/[a-z]\\*[123]/", qp));

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
               getQuery(L"/[a-z]\\/[123]/ AND path:\"/etc/init.d/\" OR "
                        L"\"/etc\\/init\\[.\\]d/lucene/\" ",
                        qp));

  shared_ptr<Query> re =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"http.*"));
  assertEquals(re, getQuery(L"field:/http.*/", qp));
  assertEquals(re, getQuery(L"/http.*/", qp));

  re = make_shared<RegexpQuery>(make_shared<Term>(L"field", L"http~0.5"));
  assertEquals(re, getQuery(L"field:/http~0.5/", qp));
  assertEquals(re, getQuery(L"/http~0.5/", qp));

  re = make_shared<RegexpQuery>(make_shared<Term>(L"field", L"boo"));
  assertEquals(re, getQuery(L"field:/boo/", qp));
  assertEquals(re, getQuery(L"/boo/", qp));

  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"field", L"/boo/")),
               getQuery(L"\"/boo/\"", qp));
  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"field", L"/boo/")),
               getQuery(L"\\/boo\\/", qp));

  shared_ptr<BooleanQuery::Builder> two = make_shared<BooleanQuery::Builder>();
  two->add(make_shared<RegexpQuery>(make_shared<Term>(L"field", L"foo")),
           Occur::SHOULD);
  two->add(make_shared<RegexpQuery>(make_shared<Term>(L"field", L"bar")),
           Occur::SHOULD);
  assertEquals(two->build(), getQuery(L"field:/foo/ field:/bar/", qp));
  assertEquals(two->build(), getQuery(L"/foo/ /bar/", qp));
}

void QueryParserTestBase::testStopwords() 
{
  shared_ptr<CharacterRunAutomaton> stopSet =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"the|foo"))->toAutomaton());
  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                                true, stopSet));
  shared_ptr<Query> result = getQuery(L"field:the OR field:foo", qp);
  assertNotNull(L"result is null and it shouldn't be", result);
  assertTrue(L"result is not a BooleanQuery",
             std::dynamic_pointer_cast<BooleanQuery>(result) != nullptr ||
                 std::dynamic_pointer_cast<MatchNoDocsQuery>(result) !=
                     nullptr);
  if (std::dynamic_pointer_cast<BooleanQuery>(result) != nullptr) {
    assertEquals(
        0, (std::static_pointer_cast<BooleanQuery>(result))->clauses().size());
  }
  result = getQuery(L"field:woo OR field:the", qp);
  assertNotNull(L"result is null and it shouldn't be", result);
  assertTrue(L"result is not a TermQuery",
             std::dynamic_pointer_cast<TermQuery>(result) != nullptr);
  result = getQuery(
      L"(fieldX:xxxxx OR fieldy:xxxxxxxx)^2 AND (fieldx:the OR fieldy:foo)",
      qp);
  assertNotNull(L"result is null and it shouldn't be", result);
  assertTrue(L"result is not a BoostQuery",
             std::dynamic_pointer_cast<BoostQuery>(result) != nullptr);
  result = (std::static_pointer_cast<BoostQuery>(result))->getQuery();
  assertTrue(L"result is not a BooleanQuery",
             std::dynamic_pointer_cast<BooleanQuery>(result) != nullptr);
  if (VERBOSE) {
    wcout << L"Result: " << result << endl;
  }
  assertTrue(
      (std::static_pointer_cast<BooleanQuery>(result))->clauses().size() +
          L" does not equal: " + to_wstring(2),
      (std::static_pointer_cast<BooleanQuery>(result))->clauses().size() == 2);
}

void QueryParserTestBase::testPositionIncrement() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true,
                                MockTokenFilter::ENGLISH_STOPSET));
  qp->setEnablePositionIncrements(true);
  wstring qtxt =
      L"\"the words in poisitions pos02578 are stopped in this phrasequery\"";
  //               0         2                      5           7  8
  std::deque<int> expectedPositions = {1, 3, 4, 6, 9};
  shared_ptr<PhraseQuery> pq =
      std::static_pointer_cast<PhraseQuery>(getQuery(qtxt, qp));
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

void QueryParserTestBase::testMatchAllDocs() 
{
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  assertEquals(make_shared<MatchAllDocsQuery>(), getQuery(L"*:*", qp));
  assertEquals(make_shared<MatchAllDocsQuery>(), getQuery(L"(*:*)", qp));
  shared_ptr<BooleanQuery> bq =
      std::static_pointer_cast<BooleanQuery>(getQuery(L"+*:* -*:*", qp));
  assertEquals(2, bq->clauses().size());
  for (auto clause : bq) {
    assertTrue(std::dynamic_pointer_cast<MatchAllDocsQuery>(
                   clause->getQuery()) != nullptr);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") private void assertHits(int
// expected, std::wstring query, IndexSearcher is) throws Exception
void QueryParserTestBase::assertHits(
    int expected, const wstring &query,
    shared_ptr<IndexSearcher> is) 
{
  wstring oldDefaultField = getDefaultField();
  setDefaultField(L"date");
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  qp->setLocale(Locale::ENGLISH);
  shared_ptr<Query> q = getQuery(query, qp);
  std::deque<std::shared_ptr<ScoreDoc>> hits = is->search(q, 1000)->scoreDocs;
  assertEquals(expected, hits.size());
  setDefaultField(oldDefaultField);
}

void QueryParserTestBase::tearDown() 
{
  BooleanQuery::setMaxClauseCount(originalMaxClauses);
  LuceneTestCase::tearDown();
}

void QueryParserTestBase::testPositionIncrements() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> a = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(a));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"the wizard of ozzy", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  delete w;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<Query> q = getQuery(L"\"wizard of ozzy\"", a);
  assertEquals(1, s->search(q, 1)->totalHits);
  delete r;
  delete dir;
}

QueryParserTestBase::Analyzer1::Analyzer1() : Analyzer() {}

shared_ptr<TokenStreamComponents>
QueryParserTestBase::Analyzer1::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<MockSynonymFilter>(tokenizer));
}

QueryParserTestBase::Analyzer2::Analyzer2() : Analyzer() {}

shared_ptr<TokenStreamComponents>
QueryParserTestBase::Analyzer2::createComponents(const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true));
}

QueryParserTestBase::MockCollationFilter::MockCollationFilter(
    shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
}

bool QueryParserTestBase::MockCollationFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring term = termAtt->toString();
    termAtt->setEmpty()->append(L"collated")->append(term);
    return true;
  } else {
    return false;
  }
}

shared_ptr<TokenStreamComponents>
QueryParserTestBase::MockCollationAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<MockCollationFilter>(tokenizer));
}

shared_ptr<TokenStream> QueryParserTestBase::MockCollationAnalyzer::normalize(
    const wstring &fieldName, shared_ptr<TokenStream> in_)
{
  return make_shared<MockCollationFilter>(make_shared<LowerCaseFilter>(in_));
}

void QueryParserTestBase::testCollatedRange() 
{
  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(make_shared<MockCollationAnalyzer>());
  shared_ptr<Query> expected = TermRangeQuery::newStringRange(
      getDefaultField(), L"collatedabc", L"collateddef", true, true);
  shared_ptr<Query> actual = getQuery(L"[abc TO def]", qp);
  assertEquals(expected, actual);
}

void QueryParserTestBase::testDistanceAsEditsParsing() 
{
  shared_ptr<FuzzyQuery> q = std::static_pointer_cast<FuzzyQuery>(
      getQuery(L"foobar~2", make_shared<MockAnalyzer>(random())));
  assertEquals(2, q->getMaxEdits());
}

void QueryParserTestBase::testPhraseQueryToString() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE, true, MockTokenFilter::ENGLISH_STOPSET);
  shared_ptr<CommonQueryParserConfiguration> qp = getParserConfig(analyzer);
  qp->setEnablePositionIncrements(true);
  shared_ptr<PhraseQuery> q = std::static_pointer_cast<PhraseQuery>(
      getQuery(L"\"this hi this is a test is\"", qp));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:\"? hi ? ? ? test\"", q->toString());
}

void QueryParserTestBase::testParseWildcardAndPhraseQueries() throw(
    runtime_error)
{
  wstring field = L"content";
  wstring oldDefaultField = getDefaultField();
  setDefaultField(field);
  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(make_shared<MockAnalyzer>(random()));
  qp->setAllowLeadingWildcard(true);

  std::deque<std::deque<wstring>> prefixQueries = {
      std::deque<wstring>{L"a*", L"ab*", L"abc*"},
      std::deque<wstring>{L"h*", L"hi*", L"hij*", L"\\\\7*"},
      std::deque<wstring>{L"o*", L"op*", L"opq*", L"\\\\\\\\*"}};

  std::deque<std::deque<wstring>> wildcardQueries = {
      std::deque<wstring>{L"*a*", L"*ab*", L"*abc**", L"ab*e*", L"*g?",
                           L"*f?1", L"abc**"},
      std::deque<wstring>{L"*h*", L"*hi*", L"*hij**", L"hi*k*", L"*n?",
                           L"*m?1", L"hij**"},
      std::deque<wstring>{L"*o*", L"*op*", L"*opq**", L"op*q*", L"*u?",
                           L"*t?1", L"opq**"}};

  // test queries that must be prefix queries
  for (int i = 0; i < prefixQueries.size(); i++) {
    for (int j = 0; j < prefixQueries[i].size(); j++) {
      wstring queryString = prefixQueries[i][j];
      shared_ptr<Query> q = getQuery(queryString, qp);
      assertEquals(PrefixQuery::typeid, q->getClass());
    }
  }

  // test queries that must be wildcard queries
  for (int i = 0; i < wildcardQueries.size(); i++) {
    for (int j = 0; j < wildcardQueries[i].size(); j++) {
      wstring qtxt = wildcardQueries[i][j];
      shared_ptr<Query> q = getQuery(qtxt, qp);
      assertEquals(WildcardQuery::typeid, q->getClass());
    }
  }
  setDefaultField(oldDefaultField);
}

void QueryParserTestBase::testPhraseQueryPositionIncrements() throw(
    runtime_error)
{
  shared_ptr<CharacterRunAutomaton> stopStopList =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"[sS][tT][oO][pP]"))->toAutomaton());

  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false, stopStopList));
  qp->setEnablePositionIncrements(true);

  shared_ptr<PhraseQuery::Builder> phraseQuery =
      make_shared<PhraseQuery::Builder>();
  phraseQuery->add(make_shared<Term>(L"field", L"1"));
  phraseQuery->add(make_shared<Term>(L"field", L"2"), 2);
  assertEquals(phraseQuery->build(), getQuery(L"\"1 stop 2\"", qp));
}

void QueryParserTestBase::testMatchAllQueryParsing() 
{
  // test simple parsing of MatchAllDocsQuery
  wstring oldDefaultField = getDefaultField();
  setDefaultField(L"key");
  shared_ptr<CommonQueryParserConfiguration> qp =
      getParserConfig(make_shared<MockAnalyzer>(random()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(make_shared<MatchAllDocsQuery>(),
               getQuery((make_shared<MatchAllDocsQuery>())->toString(), qp));

  // test parsing with non-default boost
  shared_ptr<Query> query = make_shared<MatchAllDocsQuery>();
  query = make_shared<BoostQuery>(query, 2.3f);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(query, getQuery(query->toString(), qp));
  setDefaultField(oldDefaultField);
}

void QueryParserTestBase::testNestedAndClausesFoo() 
{
  wstring query = L"(field1:[1 TO *] AND field1:[* TO 2]) AND field2:(z)";
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(TermRangeQuery::newStringRange(L"field1", L"1", L"", true, true),
          BooleanClause::Occur::MUST);
  bq->add(TermRangeQuery::newStringRange(L"field1", L"", L"2", true, true),
          BooleanClause::Occur::MUST);
  q->add(bq->build(), BooleanClause::Occur::MUST);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field2", L"z")),
         BooleanClause::Occur::MUST);
  assertEquals(q->build(),
               getQuery(query, make_shared<MockAnalyzer>(random())));
}
} // namespace org::apache::lucene::queryparser::util