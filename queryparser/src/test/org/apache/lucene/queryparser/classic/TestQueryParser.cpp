using namespace std;

#include "TestQueryParser.h"

namespace org::apache::lucene::queryparser::classic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockBytesAnalyzer = org::apache::lucene::analysis::MockBytesAnalyzer;
using MockLowerCaseFilter = org::apache::lucene::analysis::MockLowerCaseFilter;
using MockSynonymAnalyzer = org::apache::lucene::analysis::MockSynonymAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Operator =
    org::apache::lucene::queryparser::classic::QueryParser::Operator;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using QueryParserTestBase =
    org::apache::lucene::queryparser::util::QueryParserTestBase;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using TooComplexToDeterminizeException =
    org::apache::lucene::util::automaton::TooComplexToDeterminizeException;
const wstring TestQueryParser::FIELD = L"field";

TestQueryParser::QPTestParser::QPTestParser(const wstring &f,
                                            shared_ptr<Analyzer> a)
    : QueryParser(f, a)
{
}

shared_ptr<Query> TestQueryParser::QPTestParser::getFuzzyQuery(
    const wstring &field, const wstring &termStr,
    float minSimilarity) 
{
  throw make_shared<ParseException>(L"Fuzzy queries not allowed");
}

shared_ptr<Query> TestQueryParser::QPTestParser::getWildcardQuery(
    const wstring &field, const wstring &termStr) 
{
  throw make_shared<ParseException>(L"Wildcard queries not allowed");
}

shared_ptr<QueryParser>
TestQueryParser::getParser(shared_ptr<Analyzer> a) 
{
  if (a == nullptr) {
    a = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  }
  shared_ptr<QueryParser> qp = make_shared<QueryParser>(getDefaultField(), a);
  qp->setDefaultOperator(QueryParserBase::OR_OPERATOR);
  qp->setSplitOnWhitespace(splitOnWhitespace);
  return qp;
}

shared_ptr<CommonQueryParserConfiguration>
TestQueryParser::getParserConfig(shared_ptr<Analyzer> a) 
{
  return getParser(a);
}

shared_ptr<Query> TestQueryParser::getQuery(
    const wstring &query,
    shared_ptr<CommonQueryParserConfiguration> cqpC) 
{
  assert((cqpC != nullptr, L"Parameter must not be null"));
  assert(std::dynamic_pointer_cast<QueryParser>(cqpC) != nullptr)
      : L"Parameter must be instance of QueryParser";
  shared_ptr<QueryParser> qp = std::static_pointer_cast<QueryParser>(cqpC);
  return qp->parse(query);
}

shared_ptr<Query>
TestQueryParser::getQuery(const wstring &query,
                          shared_ptr<Analyzer> a) 
{
  return getParser(a)->parse(query);
}

bool TestQueryParser::isQueryParserException(runtime_error exception)
{
  return std::dynamic_pointer_cast<ParseException>(exception) != nullptr;
}

void TestQueryParser::setDefaultOperatorOR(
    shared_ptr<CommonQueryParserConfiguration> cqpC)
{
  assert(std::dynamic_pointer_cast<QueryParser>(cqpC) != nullptr);
  shared_ptr<QueryParser> qp = std::static_pointer_cast<QueryParser>(cqpC);
  qp->setDefaultOperator(Operator::OR);
}

void TestQueryParser::setDefaultOperatorAND(
    shared_ptr<CommonQueryParserConfiguration> cqpC)
{
  assert(std::dynamic_pointer_cast<QueryParser>(cqpC) != nullptr);
  shared_ptr<QueryParser> qp = std::static_pointer_cast<QueryParser>(cqpC);
  qp->setDefaultOperator(Operator::AND);
}

void TestQueryParser::setAutoGeneratePhraseQueries(
    shared_ptr<CommonQueryParserConfiguration> cqpC, bool value)
{
  assert(std::dynamic_pointer_cast<QueryParser>(cqpC) != nullptr);
  shared_ptr<QueryParser> qp = std::static_pointer_cast<QueryParser>(cqpC);
  qp->setAutoGeneratePhraseQueries(value);
}

void TestQueryParser::setDateResolution(
    shared_ptr<CommonQueryParserConfiguration> cqpC,
    shared_ptr<std::wstring> field, Resolution value)
{
  assert(std::dynamic_pointer_cast<QueryParser>(cqpC) != nullptr);
  shared_ptr<QueryParser> qp = std::static_pointer_cast<QueryParser>(cqpC);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  qp->setDateResolution(field->toString(), value);
}

void TestQueryParser::testDefaultOperator() 
{
  shared_ptr<QueryParser> qp = getParser(make_shared<MockAnalyzer>(random()));
  // make sure OR is the default:
  assertEquals(QueryParserBase::OR_OPERATOR, qp->getDefaultOperator());
  setDefaultOperatorAND(qp);
  assertEquals(QueryParserBase::AND_OPERATOR, qp->getDefaultOperator());
  setDefaultOperatorOR(qp);
  assertEquals(QueryParserBase::OR_OPERATOR, qp->getDefaultOperator());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("rawtype") public void testProtectedCtors()
// throws Exception
void TestQueryParser::testProtectedCtors() 
{
  try {
    QueryParser::typeid->getConstructor(CharStream::typeid);
    fail(L"please switch public QueryParser(CharStream) to be protected");
  } catch (const NoSuchMethodException &nsme) {
    // expected
  }
  try {
    QueryParser::typeid->getConstructor(QueryParserTokenManager::typeid);
    fail(L"please switch public QueryParser(QueryParserTokenManager) to be "
         L"protected");
  } catch (const NoSuchMethodException &nsme) {
    // expected
  }
}

void TestQueryParser::testFuzzySlopeExtendability() 
{
  shared_ptr<QueryParser> qp = make_shared<QueryParserAnonymousInnerClass>(
      shared_from_this(),
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false));
  assertEquals(qp->parse(L"a:[11.95 TO 12.95]"), qp->parse(L"12.45~1€"));
}

TestQueryParser::QueryParserAnonymousInnerClass::QueryParserAnonymousInnerClass(
    shared_ptr<TestQueryParser> outerInstance, shared_ptr<MockAnalyzer> org)
    : QueryParser(L"a", MockAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Query>
TestQueryParser::QueryParserAnonymousInnerClass::handleBareFuzzy(
    const wstring &qfield, shared_ptr<Token> fuzzySlop,
    const wstring &termImage) 
{

  if (StringHelper::endsWith(fuzzySlop->image, L"€")) {
    float fms = fuzzyMinSim;
    try {
      fms =
          stof(fuzzySlop->image.substr(1, (fuzzySlop->image.length() - 1) - 1));
    } catch (const runtime_error &ignored) {
    }
    float value = stof(termImage);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return getRangeQuery(qfield, Float::toString(value - fms / 2.0f),
                         Float::toString(value + fms / 2.0f), true, true);
  }
  return outerInstance->super->handleBareFuzzy(qfield, fuzzySlop, termImage);
}

void TestQueryParser::testStarParsing() 
{
  const std::deque<int> type = std::deque<int>(1);
  shared_ptr<QueryParser> qp = make_shared<QueryParserAnonymousInnerClass2>(
      shared_from_this(), FIELD,
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
      type);

  shared_ptr<TermQuery> tq;

  tq = std::static_pointer_cast<TermQuery>(qp->parse(L"foo:zoo*"));
  assertEquals(L"zoo", tq->getTerm()->text());
  assertEquals(2, type[0]);

  shared_ptr<BoostQuery> bq =
      std::static_pointer_cast<BoostQuery>(qp->parse(L"foo:zoo*^2"));
  tq = std::static_pointer_cast<TermQuery>(bq->getQuery());
  assertEquals(L"zoo", tq->getTerm()->text());
  assertEquals(2, type[0]);
  assertEquals(bq->getBoost(), 2, 0);

  tq = std::static_pointer_cast<TermQuery>(qp->parse(L"foo:*"));
  assertEquals(L"*", tq->getTerm()->text());
  assertEquals(1, type[0]); // could be a valid prefix query in the future too

  bq = std::static_pointer_cast<BoostQuery>(qp->parse(L"foo:*^2"));
  tq = std::static_pointer_cast<TermQuery>(bq->getQuery());
  assertEquals(L"*", tq->getTerm()->text());
  assertEquals(1, type[0]);
  assertEquals(bq->getBoost(), 2, 0);

  tq = std::static_pointer_cast<TermQuery>(qp->parse(L"*:foo"));
  assertEquals(L"*", tq->getTerm()->field());
  assertEquals(L"foo", tq->getTerm()->text());
  assertEquals(3, type[0]);

  tq = std::static_pointer_cast<TermQuery>(qp->parse(L"*:*"));
  assertEquals(L"*", tq->getTerm()->field());
  assertEquals(L"*", tq->getTerm()->text());
  assertEquals(1, type[0]); // could be handled as a prefix query in the
                            // future

  tq = std::static_pointer_cast<TermQuery>(qp->parse(L"(*:*)"));
  assertEquals(L"*", tq->getTerm()->field());
  assertEquals(L"*", tq->getTerm()->text());
  assertEquals(1, type[0]);
}

TestQueryParser::QueryParserAnonymousInnerClass2::
    QueryParserAnonymousInnerClass2(shared_ptr<TestQueryParser> outerInstance,
                                    const wstring &FIELD,
                                    shared_ptr<MockAnalyzer> org,
                                    deque<int> &type)
    : QueryParser(FIELD, MockAnalyzer)
{
  this->outerInstance = outerInstance;
  this->type = type;
}

shared_ptr<Query>
TestQueryParser::QueryParserAnonymousInnerClass2::getWildcardQuery(
    const wstring &field, const wstring &termStr)
{
  // override error checking of superclass
  type[0] = 1;
  return make_shared<TermQuery>(make_shared<Term>(field, termStr));
}

shared_ptr<Query>
TestQueryParser::QueryParserAnonymousInnerClass2::getPrefixQuery(
    const wstring &field, const wstring &termStr)
{
  // override error checking of superclass
  type[0] = 2;
  return make_shared<TermQuery>(make_shared<Term>(field, termStr));
}

shared_ptr<Query>
TestQueryParser::QueryParserAnonymousInnerClass2::getFieldQuery(
    const wstring &field, const wstring &queryText,
    bool quoted) 
{
  type[0] = 3;
  return outerInstance->super->getFieldQuery(field, queryText, quoted);
}

void TestQueryParser::testCustomQueryParserWildcard()
{
  expectThrows(ParseException::typeid, [&]() {
    (make_shared<QPTestParser>(
         L"contents",
         make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false)))
        ->parse(L"a?t");
  });
}

void TestQueryParser::testCustomQueryParserFuzzy() 
{
  expectThrows(ParseException::typeid, [&]() {
    (make_shared<QPTestParser>(
         L"contents",
         make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false)))
        ->parse(L"xunit~");
  });
}

TestQueryParser::SmartQueryParser::SmartQueryParser(
    shared_ptr<TestQueryParser> outerInstance)
    : QueryParser(FIELD, new Analyzer1()), outerInstance(outerInstance)
{
}

shared_ptr<Query> TestQueryParser::SmartQueryParser::getFieldQuery(
    const wstring &field, const wstring &queryText,
    bool quoted) 
{
  if (quoted) {
    return newFieldQuery(morePrecise, field, queryText, quoted);
  } else {
    return QueryParser::getFieldQuery(field, queryText, quoted);
  }
}

void TestQueryParser::testNewFieldQuery() 
{
  /** ordinary behavior, synonyms form uncoordinated bool query */
  shared_ptr<QueryParser> dumb = make_shared<QueryParser>(
      FIELD, make_shared<QueryParserTestBase::Analyzer1>());
  shared_ptr<Query> expanded = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"dogs"), make_shared<Term>(FIELD, L"dog"));
  assertEquals(expanded, dumb->parse(L"\"dogs\""));
  /** even with the phrase operator the behavior is the same */
  assertEquals(expanded, dumb->parse(L"dogs"));

  /**
   * custom behavior, the synonyms are expanded, unless you use quote operator
   */
  shared_ptr<QueryParser> smart =
      make_shared<SmartQueryParser>(shared_from_this());
  assertEquals(expanded, smart->parse(L"dogs"));

  shared_ptr<Query> unexpanded =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"dogs"));
  assertEquals(unexpanded, smart->parse(L"\"dogs\""));
}

void TestQueryParser::testSynonyms() 
{
  shared_ptr<Query> expected = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"dogs"), make_shared<Term>(FIELD, L"dog"));
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockSynonymAnalyzer>());
  assertEquals(expected, qp->parse(L"dogs"));
  assertEquals(expected, qp->parse(L"\"dogs\""));
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expected, qp->parse(L"dogs"));
  assertEquals(expected, qp->parse(L"\"dogs\""));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"dogs^2"));
  assertEquals(expected, qp->parse(L"\"dogs\"^2"));
}

void TestQueryParser::testSynonymsPhrase() 
{
  shared_ptr<MultiPhraseQuery::Builder> expectedQBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  expectedQBuilder->add(make_shared<Term>(FIELD, L"old"));
  expectedQBuilder->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(FIELD, L"dogs"), make_shared<Term>(FIELD, L"dog")});
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockSynonymAnalyzer>());
  assertEquals(expectedQBuilder->build(), qp->parse(L"\"old dogs\""));
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expectedQBuilder->build(), qp->parse(L"\"old dogs\""));
  shared_ptr<BoostQuery> expected =
      make_shared<BoostQuery>(expectedQBuilder->build(), 2.0f);
  assertEquals(expected, qp->parse(L"\"old dogs\"^2"));
  expectedQBuilder->setSlop(3);
  expected = make_shared<BoostQuery>(expectedQBuilder->build(), 2.0f);
  assertEquals(expected, qp->parse(L"\"old dogs\"~3^2"));
}

TestQueryParser::MockCJKSynonymFilter::MockCJKSynonymFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestQueryParser::MockCJKSynonymFilter::incrementToken() 
{
  if (addSynonym) { // inject our synonym
    clearAttributes();
    termAtt->setEmpty()->append(L"國");
    posIncAtt->setPositionIncrement(0);
    addSynonym = false;
    return true;
  }

  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    addSynonym = termAtt->toString()->equals(L"国");
    return true;
  } else {
    return false;
  }
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQueryParser::MockCJKSynonymAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<QueryParserTestBase::SimpleCJKTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<MockCJKSynonymFilter>(tokenizer));
}

void TestQueryParser::testCJKSynonym() 
{
  shared_ptr<Query> expected = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected, qp->parse(L"国"));
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expected, qp->parse(L"国"));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"国^2"));
}

void TestQueryParser::testCJKSynonymsOR() 
{
  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"中")),
                 BooleanClause::Occur::SHOULD);
  shared_ptr<Query> inner = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner, BooleanClause::Occur::SHOULD);
  shared_ptr<Query> expected = expectedB->build();
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected, qp->parse(L"中国"));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"中国^2"));
}

void TestQueryParser::testCJKSynonymsOR2() 
{
  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"中")),
                 BooleanClause::Occur::SHOULD);
  shared_ptr<SynonymQuery> inner = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner, BooleanClause::Occur::SHOULD);
  shared_ptr<SynonymQuery> inner2 = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner2, BooleanClause::Occur::SHOULD);
  shared_ptr<Query> expected = expectedB->build();
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected, qp->parse(L"中国国"));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"中国国^2"));
}

void TestQueryParser::testCJKSynonymsAND() 
{
  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"中")),
                 BooleanClause::Occur::MUST);
  shared_ptr<Query> inner = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner, BooleanClause::Occur::MUST);
  shared_ptr<Query> expected = expectedB->build();
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expected, qp->parse(L"中国"));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"中国^2"));
}

void TestQueryParser::testCJKSynonymsAND2() 
{
  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"中")),
                 BooleanClause::Occur::MUST);
  shared_ptr<Query> inner = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner, BooleanClause::Occur::MUST);
  shared_ptr<Query> inner2 = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國"));
  expectedB->add(inner2, BooleanClause::Occur::MUST);
  shared_ptr<Query> expected = expectedB->build();
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expected, qp->parse(L"中国国"));
  expected = make_shared<BoostQuery>(expected, 2.0f);
  assertEquals(expected, qp->parse(L"中国国^2"));
}

void TestQueryParser::testCJKSynonymsPhrase() 
{
  shared_ptr<MultiPhraseQuery::Builder> expectedQBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  expectedQBuilder->add(make_shared<Term>(FIELD, L"中"));
  expectedQBuilder->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(FIELD, L"国"), make_shared<Term>(FIELD, L"國")});
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockCJKSynonymAnalyzer>());
  qp->setDefaultOperator(Operator::AND);
  assertEquals(expectedQBuilder->build(), qp->parse(L"\"中国\""));
  shared_ptr<Query> expected =
      make_shared<BoostQuery>(expectedQBuilder->build(), 2.0f);
  assertEquals(expected, qp->parse(L"\"中国\"^2"));
  expectedQBuilder->setSlop(3);
  expected = make_shared<BoostQuery>(expectedQBuilder->build(), 2.0f);
  assertEquals(expected, qp->parse(L"\"中国\"~3^2"));
}

void TestQueryParser::testWildcardMaxDeterminizedStates() 
{
  shared_ptr<QueryParser> qp =
      make_shared<QueryParser>(FIELD, make_shared<MockAnalyzer>(random()));
  qp->setMaxDeterminizedStates(10);
  expectThrows(TooComplexToDeterminizeException::typeid,
               [&]() { qp->parse(L"a*aaaaaaa"); });
}

void TestQueryParser::testQPA() 
{
  bool oldSplitOnWhitespace = splitOnWhitespace;
  splitOnWhitespace = false;

  assertQueryEquals(L"term phrase term", qpAnalyzer,
                    L"term phrase1 phrase2 term");

  shared_ptr<CommonQueryParserConfiguration> cqpc = getParserConfig(qpAnalyzer);
  setDefaultOperatorAND(cqpc);
  assertQueryEquals(cqpc, L"field", L"term phrase term",
                    L"+term +phrase1 +phrase2 +term");

  splitOnWhitespace = oldSplitOnWhitespace;
}

void TestQueryParser::testMultiWordSynonyms() 
{
  shared_ptr<QueryParser> dumb = make_shared<QueryParser>(
      L"field", make_shared<QueryParserTestBase::Analyzer1>());
  dumb->setSplitOnWhitespace(false);

  shared_ptr<TermQuery> guinea =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"guinea"));
  shared_ptr<TermQuery> pig =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"pig"));
  shared_ptr<TermQuery> cavy =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"cavy"));

  // A multi-word synonym source will form a graph query for synonyms that
  // formed the graph token stream
  shared_ptr<BooleanQuery::Builder> synonym =
      make_shared<BooleanQuery::Builder>();
  synonym->add(guinea, BooleanClause::Occur::MUST);
  synonym->add(pig, BooleanClause::Occur::MUST);
  shared_ptr<BooleanQuery> guineaPig = synonym->build();

  shared_ptr<PhraseQuery> phraseGuineaPig =
      (make_shared<PhraseQuery::Builder>())
          ->add(make_shared<Term>(L"field", L"guinea"))
          ->add(make_shared<Term>(L"field", L"pig"))
          ->build();

  shared_ptr<BooleanQuery> graphQuery =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BooleanQuery::Builder>()
                    .add(guineaPig, BooleanClause::Occur::SHOULD)
                    ->add(cavy, BooleanClause::Occur::SHOULD)
                    .build(),
                BooleanClause::Occur::SHOULD)
          .build();
  assertEquals(graphQuery, dumb->parse(L"guinea pig"));

  // With the phrase operator, a multi-word synonym source will form span near
  // queries.
  shared_ptr<SpanNearQuery> spanGuineaPig =
      SpanNearQuery::newOrderedNearQuery(L"field")
          ->addClause(make_shared<SpanTermQuery>(
              make_shared<Term>(L"field", L"guinea")))
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"pig")))
          ->setSlop(0)
          ->build();
  shared_ptr<SpanTermQuery> spanCavy =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"cavy"));
  shared_ptr<SpanOrQuery> spanPhrase = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanGuineaPig, spanCavy});
  assertEquals(spanPhrase, dumb->parse(L"\"guinea pig\""));

  // custom behavior, the synonyms are expanded, unless you use quote operator
  shared_ptr<QueryParser> smart =
      make_shared<SmartQueryParser>(shared_from_this());
  smart->setSplitOnWhitespace(false);
  graphQuery = (make_shared<BooleanQuery::Builder>())
                   ->add(make_shared<BooleanQuery::Builder>()
                             .add(guineaPig, BooleanClause::Occur::SHOULD)
                             ->add(cavy, BooleanClause::Occur::SHOULD)
                             .build(),
                         BooleanClause::Occur::SHOULD)
                   .build();
  assertEquals(graphQuery, smart->parse(L"guinea pig"));
  assertEquals(phraseGuineaPig, smart->parse(L"\"guinea pig\""));

  // with the AND operator
  dumb->setDefaultOperator(Operator::AND);
  shared_ptr<BooleanQuery> graphAndQuery =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BooleanQuery::Builder>()
                    .add(guineaPig, BooleanClause::Occur::SHOULD)
                    ->add(cavy, BooleanClause::Occur::SHOULD)
                    .build(),
                BooleanClause::Occur::MUST)
          .build();
  assertEquals(graphAndQuery, dumb->parse(L"guinea pig"));

  graphAndQuery = (make_shared<BooleanQuery::Builder>())
                      ->add(make_shared<BooleanQuery::Builder>()
                                .add(guineaPig, BooleanClause::Occur::SHOULD)
                                ->add(cavy, BooleanClause::Occur::SHOULD)
                                .build(),
                            BooleanClause::Occur::MUST)
                      .add(cavy, BooleanClause::Occur::MUST)
                      .build();
  assertEquals(graphAndQuery, dumb->parse(L"guinea pig cavy"));
}

void TestQueryParser::testEnableGraphQueries() 
{
  shared_ptr<QueryParser> dumb = make_shared<QueryParser>(
      L"field", make_shared<QueryParserTestBase::Analyzer1>());
  dumb->setSplitOnWhitespace(false);
  dumb->setEnableGraphQueries(false);

  shared_ptr<TermQuery> guinea =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"guinea"));
  shared_ptr<TermQuery> pig =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"pig"));
  shared_ptr<TermQuery> cavy =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"cavy"));

  // A multi-word synonym source will just form a bool query when graph
  // queries are disabled:
  shared_ptr<Query> inner =
      make_shared<SynonymQuery>(std::deque<std::shared_ptr<Term>>{
          make_shared<Term>(L"field", L"cavy"),
          make_shared<Term>(L"field", L"guinea")});
  shared_ptr<BooleanQuery::Builder> b = make_shared<BooleanQuery::Builder>();
  b->add(inner, BooleanClause::Occur::SHOULD);
  b->add(pig, BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery> query = b->build();
  assertEquals(query, dumb->parse(L"guinea pig"));
}

void TestQueryParser::testOperatorsAndMultiWordSynonyms() 
{
  shared_ptr<Analyzer> a = make_shared<MockSynonymAnalyzer>();

  bool oldSplitOnWhitespace = splitOnWhitespace;
  splitOnWhitespace = false;

  // Operators should interrupt multiword analysis of adjacent words if they
  // associate
  assertQueryEquals(L"+guinea pig", a, L"+guinea pig");
  assertQueryEquals(L"-guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"!guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"guinea* pig", a, L"guinea* pig");
  assertQueryEquals(L"guinea? pig", a, L"guinea? pig");
  assertQueryEquals(L"guinea~2 pig", a, L"guinea~2 pig");
  assertQueryEquals(L"guinea^2 pig", a, L"(guinea)^2.0 pig");

  assertQueryEquals(L"guinea +pig", a, L"guinea +pig");
  assertQueryEquals(L"guinea -pig", a, L"guinea -pig");
  assertQueryEquals(L"guinea !pig", a, L"guinea -pig");
  assertQueryEquals(L"guinea pig*", a, L"guinea pig*");
  assertQueryEquals(L"guinea pig?", a, L"guinea pig?");
  assertQueryEquals(L"guinea pig~2", a, L"guinea pig~2");
  assertQueryEquals(L"guinea pig^2", a, L"guinea (pig)^2.0");

  assertQueryEquals(L"field:guinea pig", a, L"guinea pig");
  assertQueryEquals(L"guinea field:pig", a, L"guinea pig");

  assertQueryEquals(L"NOT guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"guinea NOT pig", a, L"guinea -pig");

  assertQueryEquals(L"guinea pig AND dogs", a,
                    L"guinea +pig +Synonym(dog dogs)");
  assertQueryEquals(L"dogs AND guinea pig", a,
                    L"+Synonym(dog dogs) +guinea pig");
  assertQueryEquals(L"guinea pig && dogs", a,
                    L"guinea +pig +Synonym(dog dogs)");
  assertQueryEquals(L"dogs && guinea pig", a,
                    L"+Synonym(dog dogs) +guinea pig");

  assertQueryEquals(L"guinea pig OR dogs", a, L"guinea pig Synonym(dog dogs)");
  assertQueryEquals(L"dogs OR guinea pig", a, L"Synonym(dog dogs) guinea pig");
  assertQueryEquals(L"guinea pig || dogs", a, L"guinea pig Synonym(dog dogs)");
  assertQueryEquals(L"dogs || guinea pig", a, L"Synonym(dog dogs) guinea pig");

  assertQueryEquals(L"\"guinea\" pig", a, L"guinea pig");
  assertQueryEquals(L"guinea \"pig\"", a, L"guinea pig");

  assertQueryEquals(L"(guinea) pig", a, L"guinea pig");
  assertQueryEquals(L"guinea (pig)", a, L"guinea pig");

  assertQueryEquals(L"/guinea/ pig", a, L"/guinea/ pig");
  assertQueryEquals(L"guinea /pig/", a, L"guinea /pig/");

  // Operators should not interrupt multiword analysis if not don't associate
  assertQueryEquals(L"(guinea pig)", a, L"((+guinea +pig) cavy)");
  assertQueryEquals(L"+(guinea pig)", a, L"+(((+guinea +pig) cavy))");
  assertQueryEquals(L"-(guinea pig)", a, L"-(((+guinea +pig) cavy))");
  assertQueryEquals(L"!(guinea pig)", a, L"-(((+guinea +pig) cavy))");
  assertQueryEquals(L"NOT (guinea pig)", a, L"-(((+guinea +pig) cavy))");
  assertQueryEquals(L"(guinea pig)^2", a, L"(((+guinea +pig) cavy))^2.0");

  assertQueryEquals(L"field:(guinea pig)", a, L"((+guinea +pig) cavy)");

  assertQueryEquals(L"+small guinea pig", a, L"+small ((+guinea +pig) cavy)");
  assertQueryEquals(L"-small guinea pig", a, L"-small ((+guinea +pig) cavy)");
  assertQueryEquals(L"!small guinea pig", a, L"-small ((+guinea +pig) cavy)");
  assertQueryEquals(L"NOT small guinea pig", a,
                    L"-small ((+guinea +pig) cavy)");
  assertQueryEquals(L"small* guinea pig", a, L"small* ((+guinea +pig) cavy)");
  assertQueryEquals(L"small? guinea pig", a, L"small? ((+guinea +pig) cavy)");
  assertQueryEquals(L"\"small\" guinea pig", a, L"small ((+guinea +pig) cavy)");

  assertQueryEquals(L"guinea pig +running", a,
                    L"((+guinea +pig) cavy) +running");
  assertQueryEquals(L"guinea pig -running", a,
                    L"((+guinea +pig) cavy) -running");
  assertQueryEquals(L"guinea pig !running", a,
                    L"((+guinea +pig) cavy) -running");
  assertQueryEquals(L"guinea pig NOT running", a,
                    L"((+guinea +pig) cavy) -running");
  assertQueryEquals(L"guinea pig running*", a,
                    L"((+guinea +pig) cavy) running*");
  assertQueryEquals(L"guinea pig running?", a,
                    L"((+guinea +pig) cavy) running?");
  assertQueryEquals(L"guinea pig \"running\"", a,
                    L"((+guinea +pig) cavy) running");

  assertQueryEquals(L"\"guinea pig\"~2", a,
                    L"spanOr([spanNear([guinea, pig], 0, true), cavy])");

  assertQueryEquals(L"field:\"guinea pig\"", a,
                    L"spanOr([spanNear([guinea, pig], 0, true), cavy])");

  splitOnWhitespace = oldSplitOnWhitespace;
}

void TestQueryParser::
    testOperatorsAndMultiWordSynonymsSplitOnWhitespace() 
{
  shared_ptr<Analyzer> a = make_shared<MockSynonymAnalyzer>();

  bool oldSplitOnWhitespace = splitOnWhitespace;
  splitOnWhitespace = true;

  assertQueryEquals(L"+guinea pig", a, L"+guinea pig");
  assertQueryEquals(L"-guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"!guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"guinea* pig", a, L"guinea* pig");
  assertQueryEquals(L"guinea? pig", a, L"guinea? pig");
  assertQueryEquals(L"guinea~2 pig", a, L"guinea~2 pig");
  assertQueryEquals(L"guinea^2 pig", a, L"(guinea)^2.0 pig");

  assertQueryEquals(L"guinea +pig", a, L"guinea +pig");
  assertQueryEquals(L"guinea -pig", a, L"guinea -pig");
  assertQueryEquals(L"guinea !pig", a, L"guinea -pig");
  assertQueryEquals(L"guinea pig*", a, L"guinea pig*");
  assertQueryEquals(L"guinea pig?", a, L"guinea pig?");
  assertQueryEquals(L"guinea pig~2", a, L"guinea pig~2");
  assertQueryEquals(L"guinea pig^2", a, L"guinea (pig)^2.0");

  assertQueryEquals(L"field:guinea pig", a, L"guinea pig");
  assertQueryEquals(L"guinea field:pig", a, L"guinea pig");

  assertQueryEquals(L"NOT guinea pig", a, L"-guinea pig");
  assertQueryEquals(L"guinea NOT pig", a, L"guinea -pig");

  assertQueryEquals(L"guinea pig AND dogs", a,
                    L"guinea +pig +Synonym(dog dogs)");
  assertQueryEquals(L"dogs AND guinea pig", a,
                    L"+Synonym(dog dogs) +guinea pig");
  assertQueryEquals(L"guinea pig && dogs", a,
                    L"guinea +pig +Synonym(dog dogs)");
  assertQueryEquals(L"dogs && guinea pig", a,
                    L"+Synonym(dog dogs) +guinea pig");

  assertQueryEquals(L"guinea pig OR dogs", a, L"guinea pig Synonym(dog dogs)");
  assertQueryEquals(L"dogs OR guinea pig", a, L"Synonym(dog dogs) guinea pig");
  assertQueryEquals(L"guinea pig || dogs", a, L"guinea pig Synonym(dog dogs)");
  assertQueryEquals(L"dogs || guinea pig", a, L"Synonym(dog dogs) guinea pig");

  assertQueryEquals(L"\"guinea\" pig", a, L"guinea pig");
  assertQueryEquals(L"guinea \"pig\"", a, L"guinea pig");

  assertQueryEquals(L"(guinea) pig", a, L"guinea pig");
  assertQueryEquals(L"guinea (pig)", a, L"guinea pig");

  assertQueryEquals(L"/guinea/ pig", a, L"/guinea/ pig");
  assertQueryEquals(L"guinea /pig/", a, L"guinea /pig/");

  assertQueryEquals(L"(guinea pig)", a, L"guinea pig");
  assertQueryEquals(L"+(guinea pig)", a, L"+(guinea pig)");
  assertQueryEquals(L"-(guinea pig)", a, L"-(guinea pig)");
  assertQueryEquals(L"!(guinea pig)", a, L"-(guinea pig)");
  assertQueryEquals(L"NOT (guinea pig)", a, L"-(guinea pig)");
  assertQueryEquals(L"(guinea pig)^2", a, L"(guinea pig)^2.0");

  assertQueryEquals(L"field:(guinea pig)", a, L"guinea pig");

  assertQueryEquals(L"+small guinea pig", a, L"+small guinea pig");
  assertQueryEquals(L"-small guinea pig", a, L"-small guinea pig");
  assertQueryEquals(L"!small guinea pig", a, L"-small guinea pig");
  assertQueryEquals(L"NOT small guinea pig", a, L"-small guinea pig");
  assertQueryEquals(L"small* guinea pig", a, L"small* guinea pig");
  assertQueryEquals(L"small? guinea pig", a, L"small? guinea pig");
  assertQueryEquals(L"\"small\" guinea pig", a, L"small guinea pig");

  assertQueryEquals(L"guinea pig +running", a, L"guinea pig +running");
  assertQueryEquals(L"guinea pig -running", a, L"guinea pig -running");
  assertQueryEquals(L"guinea pig !running", a, L"guinea pig -running");
  assertQueryEquals(L"guinea pig NOT running", a, L"guinea pig -running");
  assertQueryEquals(L"guinea pig running*", a, L"guinea pig running*");
  assertQueryEquals(L"guinea pig running?", a, L"guinea pig running?");
  assertQueryEquals(L"guinea pig \"running\"", a, L"guinea pig running");

  assertQueryEquals(L"\"guinea pig\"~2", a,
                    L"spanOr([spanNear([guinea, pig], 0, true), cavy])");

  assertQueryEquals(L"field:\"guinea pig\"", a,
                    L"spanOr([spanNear([guinea, pig], 0, true), cavy])");

  splitOnWhitespace = oldSplitOnWhitespace;
}

void TestQueryParser::testDefaultSplitOnWhitespace() 
{
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(
      L"field", make_shared<QueryParserTestBase::Analyzer1>());

  assertFalse(parser->getSplitOnWhitespace()); // default is false

  // A multi-word synonym source will form a synonym query for the
  // same-starting-position tokens
  shared_ptr<TermQuery> guinea =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"guinea"));
  shared_ptr<TermQuery> pig =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"pig"));
  shared_ptr<TermQuery> cavy =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"cavy"));

  // A multi-word synonym source will form a graph query for synonyms that
  // formed the graph token stream
  shared_ptr<BooleanQuery::Builder> synonym =
      make_shared<BooleanQuery::Builder>();
  synonym->add(guinea, BooleanClause::Occur::MUST);
  synonym->add(pig, BooleanClause::Occur::MUST);
  shared_ptr<BooleanQuery> guineaPig = synonym->build();

  shared_ptr<BooleanQuery> graphQuery =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BooleanQuery::Builder>()
                    .add(guineaPig, BooleanClause::Occur::SHOULD)
                    ->add(cavy, BooleanClause::Occur::SHOULD)
                    .build(),
                BooleanClause::Occur::SHOULD)
          .build();
  assertEquals(graphQuery, parser->parse(L"guinea pig"));

  bool oldSplitOnWhitespace = splitOnWhitespace;
  splitOnWhitespace = QueryParser::DEFAULT_SPLIT_ON_WHITESPACE;
  assertQueryEquals(L"guinea pig", make_shared<MockSynonymAnalyzer>(),
                    L"((+guinea +pig) cavy)");
  splitOnWhitespace = oldSplitOnWhitespace;
}

void TestQueryParser::testWildcardAlone() 
{
  // seems like crazy edge case, but can be useful in concordance
  shared_ptr<QueryParser> parser =
      make_shared<QueryParser>(FIELD, make_shared<ASCIIAnalyzer>());
  parser->setAllowLeadingWildcard(false);
  expectThrows(ParseException::typeid, [&]() { parser->parse(L"*"); });

  shared_ptr<QueryParser> parser2 =
      make_shared<QueryParser>(L"*", make_shared<ASCIIAnalyzer>());
  parser2->setAllowLeadingWildcard(false);
  assertEquals(make_shared<MatchAllDocsQuery>(), parser2->parse(L"*"));
}

void TestQueryParser::testWildCardEscapes() 
{
  shared_ptr<Analyzer> a = make_shared<ASCIIAnalyzer>();
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(FIELD, a);
  assertTrue(isAHit(parser->parse(L"mö*tley"), L"moatley", a));
  // need to have at least one genuine wildcard to trigger the wildcard analysis
  // hence the * before the y
  assertTrue(isAHit(parser->parse(L"mö\\*tl*y"), L"mo*tley", a));
  // escaped backslash then true wildcard
  assertTrue(isAHit(parser->parse(L"mö\\\\*tley"), L"mo\\atley", a));
  // escaped wildcard then true wildcard
  assertTrue(isAHit(parser->parse(L"mö\\??ley"), L"mo?tley", a));

  // the first is an escaped * which should yield a miss
  assertFalse(isAHit(parser->parse(L"mö\\*tl*y"), L"moatley", a));
}

void TestQueryParser::testWildcardDoesNotNormalizeEscapedChars() throw(
    runtime_error)
{
  shared_ptr<Analyzer> asciiAnalyzer = make_shared<ASCIIAnalyzer>();
  shared_ptr<Analyzer> keywordAnalyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<QueryParser> parser =
      make_shared<QueryParser>(FIELD, asciiAnalyzer);

  assertTrue(isAHit(parser->parse(L"e*e"), L"étude", asciiAnalyzer));
  assertTrue(isAHit(parser->parse(L"é*e"), L"etude", asciiAnalyzer));
  assertFalse(isAHit(parser->parse(L"\\é*e"), L"etude", asciiAnalyzer));
  assertTrue(isAHit(parser->parse(L"\\é*e"), L"étude", keywordAnalyzer));
}

void TestQueryParser::testWildCardQuery() 
{
  shared_ptr<Analyzer> a = make_shared<ASCIIAnalyzer>();
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(FIELD, a);
  parser->setAllowLeadingWildcard(true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"*bersetzung uber*ung",
               parser->parse(L"*bersetzung über*ung")->toString(FIELD));
  parser->setAllowLeadingWildcard(false);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"motley crue motl?* cru?",
               parser->parse(L"Mötley Cr\u00fce Mötl?* Crü?")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"renee zellweger ren?? zellw?ger",
      parser->parse(L"Renée Zellweger Ren?? Zellw?ger")->toString(FIELD));
}

void TestQueryParser::testPrefixQuery() 
{
  shared_ptr<Analyzer> a = make_shared<ASCIIAnalyzer>();
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(FIELD, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"ubersetzung ubersetz*",
               parser->parse(L"übersetzung übersetz*")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"motley crue motl* cru*",
               parser->parse(L"Mötley Crüe Mötl* crü*")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"rene? zellw*",
               parser->parse(L"René? Zellw*")->toString(FIELD));
}

void TestQueryParser::testRangeQuery() 
{
  shared_ptr<Analyzer> a = make_shared<ASCIIAnalyzer>();
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(FIELD, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"[aa TO bb]", parser->parse(L"[aa TO bb]")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"{anais TO zoe}",
               parser->parse(L"{Anaïs TO Zoé}")->toString(FIELD));
}

void TestQueryParser::testFuzzyQuery() 
{
  shared_ptr<Analyzer> a = make_shared<ASCIIAnalyzer>();
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(FIELD, a);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"ubersetzung ubersetzung~1",
               parser->parse(L"Übersetzung Übersetzung~0.9")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"motley crue motley~1 crue~2",
      parser->parse(L"Mötley Crüe Mötley~0.75 Crüe~0.5")->toString(FIELD));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"renee zellweger renee~0 zellweger~2",
      parser->parse(L"Renée Zellweger Renée~0.9 Zellweger~")->toString(FIELD));
}

TestQueryParser::FoldingFilter::FoldingFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestQueryParser::FoldingFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> term = termAtt->buffer();
    for (int i = 0; i < term.size(); i++) {
      switch (term[i]) {
      case L'ü':
        term[i] = L'u';
        break;
      case L'ö':
        term[i] = L'o';
        break;
      case L'é':
        term[i] = L'e';
        break;
      case L'ï':
        term[i] = L'i';
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQueryParser::ASCIIAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<FoldingFilter>(result));
}

shared_ptr<TokenStream>
TestQueryParser::ASCIIAnalyzer::normalize(const wstring &fieldName,
                                          shared_ptr<TokenStream> in_)
{
  return make_shared<FoldingFilter>(make_shared<MockLowerCaseFilter>(in_));
}

void TestQueryParser::testByteTerms() 
{
  wstring s = L"เข";
  shared_ptr<Analyzer> analyzer = make_shared<MockBytesAnalyzer>();
  shared_ptr<QueryParser> qp = make_shared<QueryParser>(FIELD, analyzer);

  assertTrue(isAHit(qp->parse(L"[เข TO เข]"), s, analyzer));
  assertTrue(isAHit(qp->parse(L"เข~1"), s, analyzer));
  assertTrue(isAHit(qp->parse(L"เข*"), s, analyzer));
  assertTrue(isAHit(qp->parse(L"เ*"), s, analyzer));
  assertTrue(isAHit(qp->parse(L"เ??"), s, analyzer));
}

void TestQueryParser::test_splitOnWhitespace_with_autoGeneratePhraseQueries()
{
  shared_ptr<QueryParser> *const qp =
      make_shared<QueryParser>(FIELD, make_shared<MockAnalyzer>(random()));
  expectThrows(invalid_argument::typeid, [&]() {
    qp->setSplitOnWhitespace(false);
    qp->setAutoGeneratePhraseQueries(true);
  });
  shared_ptr<QueryParser> *const qp2 =
      make_shared<QueryParser>(FIELD, make_shared<MockAnalyzer>(random()));
  expectThrows(invalid_argument::typeid, [&]() {
    qp2->setSplitOnWhitespace(true);
    qp2->setAutoGeneratePhraseQueries(true);
    qp2->setSplitOnWhitespace(false);
  });
}

bool TestQueryParser::isAHit(shared_ptr<Query> q, const wstring &content,
                             shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<Directory> ramDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), ramDir, analyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType = make_shared<FieldType>();
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  fieldType->setTokenized(true);
  fieldType->setStored(true);
  shared_ptr<Field> field = make_shared<Field>(FIELD, content, fieldType);
  doc->push_back(field);
  writer->addDocument(doc);
  delete writer;
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(ramDir);
  shared_ptr<IndexSearcher> is = make_shared<IndexSearcher>(ir);

  int64_t hits = is->search(q, 10)->totalHits;
  ir->close();
  delete ramDir;
  if (hits == 1) {
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::queryparser::classic