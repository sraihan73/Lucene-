using namespace std;

#include "TestQueryBuilder.h"

namespace org::apache::lucene::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockSynonymFilter = org::apache::lucene::analysis::MockSynonymFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void TestQueryBuilder::testTerm()
{
  shared_ptr<TermQuery> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"test"));
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertEquals(expected, builder->createBooleanQuery(L"field", L"test"));
}

void TestQueryBuilder::testBoolean()
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                BooleanClause::Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                BooleanClause::Occur::SHOULD);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"foo bar"));
}

void TestQueryBuilder::testBooleanMust()
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                BooleanClause::Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                BooleanClause::Occur::MUST);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"foo bar",
                                           BooleanClause::Occur::MUST));
}

void TestQueryBuilder::testMinShouldMatchNone()
{
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertEquals(builder->createBooleanQuery(L"field", L"one two three four"),
               builder->createMinShouldMatchQuery(L"field",
                                                  L"one two three four", 0.0f));
}

void TestQueryBuilder::testMinShouldMatchAll()
{
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertEquals(builder->createBooleanQuery(L"field", L"one two three four",
                                           BooleanClause::Occur::MUST),
               builder->createMinShouldMatchQuery(L"field",
                                                  L"one two three four", 1.0f));
}

void TestQueryBuilder::testMinShouldMatch()
{
  shared_ptr<BooleanQuery::Builder> expectedB =
      make_shared<BooleanQuery::Builder>();
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"one")),
                 BooleanClause::Occur::SHOULD);
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"two")),
                 BooleanClause::Occur::SHOULD);
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"three")),
                 BooleanClause::Occur::SHOULD);
  expectedB->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"four")),
                 BooleanClause::Occur::SHOULD);
  expectedB->setMinimumNumberShouldMatch(0);
  shared_ptr<Query> expected = expectedB->build();

  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  // assertEquals(expected, builder.createMinShouldMatchQuery("field", "one two
  // three four", 0.1f)); assertEquals(expected,
  // builder.createMinShouldMatchQuery("field", "one two three four", 0.24f));

  expectedB->setMinimumNumberShouldMatch(1);
  expected = expectedB->build();
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.25f));
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.49f));

  expectedB->setMinimumNumberShouldMatch(2);
  expected = expectedB->build();
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.5f));
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.74f));

  expectedB->setMinimumNumberShouldMatch(3);
  expected = expectedB->build();
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.75f));
  assertEquals(expected, builder->createMinShouldMatchQuery(
                             L"field", L"one two three four", 0.99f));
}

void TestQueryBuilder::testPhraseQueryPositionIncrements() 
{
  shared_ptr<PhraseQuery::Builder> pqBuilder =
      make_shared<PhraseQuery::Builder>();
  pqBuilder->add(make_shared<Term>(L"field", L"1"), 0);
  pqBuilder->add(make_shared<Term>(L"field", L"2"), 2);
  shared_ptr<PhraseQuery> expected = pqBuilder->build();
  shared_ptr<CharacterRunAutomaton> stopList =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"[sS][tT][oO][pP]"))->toAutomaton());

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(
      random(), MockTokenizer::WHITESPACE, false, stopList);

  shared_ptr<QueryBuilder> builder = make_shared<QueryBuilder>(analyzer);
  assertEquals(expected, builder->createPhraseQuery(L"field", L"1 stop 2"));
}

void TestQueryBuilder::testEmpty()
{
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockAnalyzer>(random()));
  assertNull(builder->createBooleanQuery(L"field", L""));
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQueryBuilder::MockSynonymAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<MockSynonymFilter>(tokenizer));
}

void TestQueryBuilder::testSynonyms() 
{
  shared_ptr<SynonymQuery> expected =
      make_shared<SynonymQuery>(make_shared<Term>(L"field", L"dogs"),
                                make_shared<Term>(L"field", L"dog"));
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockSynonymAnalyzer>());
  assertEquals(expected, builder->createBooleanQuery(L"field", L"dogs"));
  assertEquals(expected, builder->createPhraseQuery(L"field", L"dogs"));
  assertEquals(expected, builder->createBooleanQuery(
                             L"field", L"dogs", BooleanClause::Occur::MUST));
  assertEquals(expected, builder->createPhraseQuery(L"field", L"dogs"));
}

void TestQueryBuilder::testSynonymsPhrase() 
{
  shared_ptr<MultiPhraseQuery::Builder> expectedBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  expectedBuilder->add(make_shared<Term>(L"field", L"old"));
  expectedBuilder->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"dogs"),
                                         make_shared<Term>(L"field", L"dog")});
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockSynonymAnalyzer>());
  assertEquals(expectedBuilder->build(),
               builder->createPhraseQuery(L"field", L"old dogs"));
}

void TestQueryBuilder::testMultiWordSynonymsPhrase() 
{
  shared_ptr<SpanNearQuery> expectedNear =
      SpanNearQuery::newOrderedNearQuery(L"field")
          ->addClause(make_shared<SpanTermQuery>(
              make_shared<Term>(L"field", L"guinea")))
          ->addClause(
              make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"pig")))
          ->setSlop(0)
          ->build();

  shared_ptr<SpanTermQuery> expectedTerm =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"cavy"));

  shared_ptr<QueryBuilder> queryBuilder =
      make_shared<QueryBuilder>(make_shared<MockSynonymAnalyzer>());
  assertEquals(make_shared<SpanOrQuery>(std::deque<std::shared_ptr<SpanQuery>>{
                   expectedNear, expectedTerm}),
               queryBuilder->createPhraseQuery(L"field", L"guinea pig"));
}

void TestQueryBuilder::testMultiWordSynonymsBoolean() 
{
  for (auto occur : std::deque<BooleanClause::Occur>{
           BooleanClause::Occur::SHOULD, BooleanClause::Occur::MUST}) {
    shared_ptr<Query> syn1 =
        (make_shared<BooleanQuery::Builder>())
            ->add(
                make_shared<TermQuery>(make_shared<Term>(L"field", L"guinea")),
                BooleanClause::Occur::MUST)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"pig")),
                  BooleanClause::Occur::MUST)
            ->build();
    shared_ptr<Query> syn2 =
        make_shared<TermQuery>(make_shared<Term>(L"field", L"cavy"));

    shared_ptr<BooleanQuery> synQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(syn1, BooleanClause::Occur::SHOULD)
            ->add(syn2, BooleanClause::Occur::SHOULD)
            ->build();

    shared_ptr<BooleanQuery> expectedGraphQuery =
        (make_shared<BooleanQuery::Builder>())->add(synQuery, occur)->build();

    shared_ptr<QueryBuilder> queryBuilder =
        make_shared<QueryBuilder>(make_shared<MockSynonymAnalyzer>());
    assertEquals(expectedGraphQuery, queryBuilder->createBooleanQuery(
                                         L"field", L"guinea pig", occur));

    shared_ptr<BooleanQuery> expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->build();
    assertEquals(
        expectedBooleanQuery,
        queryBuilder->createBooleanQuery(L"field", L"guinea pig story", occur));

    expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"the")),
                  occur)
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->build();
    assertEquals(expectedBooleanQuery,
                 queryBuilder->createBooleanQuery(
                     L"field", L"the guinea pig story", occur));

    expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"the")),
                  occur)
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->add(synQuery, occur)
            ->build();
    assertEquals(expectedBooleanQuery,
                 queryBuilder->createBooleanQuery(
                     L"field", L"the guinea pig story guinea pig", occur));
  }
}

void TestQueryBuilder::testMultiWordPhraseSynonymsBoolean() 
{
  for (auto occur : std::deque<BooleanClause::Occur>{
           BooleanClause::Occur::SHOULD, BooleanClause::Occur::MUST}) {
    shared_ptr<Query> syn1 = (make_shared<PhraseQuery::Builder>())
                                 ->add(make_shared<Term>(L"field", L"guinea"))
                                 ->add(make_shared<Term>(L"field", L"pig"))
                                 ->build();
    shared_ptr<Query> syn2 =
        make_shared<TermQuery>(make_shared<Term>(L"field", L"cavy"));

    shared_ptr<BooleanQuery> synQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(syn1, BooleanClause::Occur::SHOULD)
            ->add(syn2, BooleanClause::Occur::SHOULD)
            ->build();
    shared_ptr<BooleanQuery> expectedGraphQuery =
        (make_shared<BooleanQuery::Builder>())->add(synQuery, occur)->build();
    shared_ptr<QueryBuilder> queryBuilder =
        make_shared<QueryBuilder>(make_shared<MockSynonymAnalyzer>());
    queryBuilder->setAutoGenerateMultiTermSynonymsPhraseQuery(true);
    assertEquals(expectedGraphQuery, queryBuilder->createBooleanQuery(
                                         L"field", L"guinea pig", occur));

    shared_ptr<BooleanQuery> expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->build();
    assertEquals(
        expectedBooleanQuery,
        queryBuilder->createBooleanQuery(L"field", L"guinea pig story", occur));

    expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"the")),
                  occur)
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->build();
    assertEquals(expectedBooleanQuery,
                 queryBuilder->createBooleanQuery(
                     L"field", L"the guinea pig story", occur));

    expectedBooleanQuery =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"the")),
                  occur)
            ->add(synQuery, occur)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"story")),
                  occur)
            ->add(synQuery, occur)
            ->build();
    assertEquals(expectedBooleanQuery,
                 queryBuilder->createBooleanQuery(
                     L"field", L"the guinea pig story guinea pig", occur));
  }
}

TestQueryBuilder::SimpleCJKTokenizer::SimpleCJKTokenizer()
    : org::apache::lucene::analysis::Tokenizer()
{
}

bool TestQueryBuilder::SimpleCJKTokenizer::incrementToken() 
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
TestQueryBuilder::SimpleCJKAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<SimpleCJKTokenizer>());
}

void TestQueryBuilder::testCJKTerm() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"国")),
                BooleanClause::Occur::SHOULD);

  shared_ptr<QueryBuilder> builder = make_shared<QueryBuilder>(analyzer);
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"中国"));
}

void TestQueryBuilder::testCJKPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"中", L"国");

  shared_ptr<QueryBuilder> builder = make_shared<QueryBuilder>(analyzer);
  assertEquals(expected, builder->createPhraseQuery(L"field", L"中国"));
}

void TestQueryBuilder::testCJKSloppyPhrase() 
{
  // individual CJK chars as terms
  shared_ptr<SimpleCJKAnalyzer> analyzer = make_shared<SimpleCJKAnalyzer>();

  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(3, L"field", L"中", L"国");

  shared_ptr<QueryBuilder> builder = make_shared<QueryBuilder>(analyzer);
  assertEquals(expected, builder->createPhraseQuery(L"field", L"中国", 3));
}

TestQueryBuilder::MockCJKSynonymFilter::MockCJKSynonymFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TestQueryBuilder::MockCJKSynonymFilter::incrementToken() 
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
TestQueryBuilder::MockCJKSynonymAnalyzer::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<SimpleCJKTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<MockCJKSynonymFilter>(tokenizer));
}

void TestQueryBuilder::testCJKSynonym() 
{
  shared_ptr<SynonymQuery> expected = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected, builder->createBooleanQuery(L"field", L"国"));
  assertEquals(expected, builder->createPhraseQuery(L"field", L"国"));
  assertEquals(expected, builder->createBooleanQuery(
                             L"field", L"国", BooleanClause::Occur::MUST));
}

void TestQueryBuilder::testCJKSynonymsOR() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::SHOULD);
  shared_ptr<SynonymQuery> inner = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner, BooleanClause::Occur::SHOULD);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"中国"));
}

void TestQueryBuilder::testCJKSynonymsOR2() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::SHOULD);
  shared_ptr<SynonymQuery> inner = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner, BooleanClause::Occur::SHOULD);
  shared_ptr<SynonymQuery> inner2 = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner2, BooleanClause::Occur::SHOULD);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"中国国"));
}

void TestQueryBuilder::testCJKSynonymsAND() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::MUST);
  shared_ptr<SynonymQuery> inner = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner, BooleanClause::Occur::MUST);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"中国",
                                           BooleanClause::Occur::MUST));
}

void TestQueryBuilder::testCJKSynonymsAND2() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"中")),
                BooleanClause::Occur::MUST);
  shared_ptr<SynonymQuery> inner = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner, BooleanClause::Occur::MUST);
  shared_ptr<SynonymQuery> inner2 = make_shared<SynonymQuery>(
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國"));
  expected->add(inner2, BooleanClause::Occur::MUST);
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expected->build(),
               builder->createBooleanQuery(L"field", L"中国国",
                                           BooleanClause::Occur::MUST));
}

void TestQueryBuilder::testCJKSynonymsPhrase() 
{
  shared_ptr<MultiPhraseQuery::Builder> expectedBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  expectedBuilder->add(make_shared<Term>(L"field", L"中"));
  expectedBuilder->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(L"field", L"国"), make_shared<Term>(L"field", L"國")});
  shared_ptr<QueryBuilder> builder =
      make_shared<QueryBuilder>(make_shared<MockCJKSynonymAnalyzer>());
  assertEquals(expectedBuilder->build(),
               builder->createPhraseQuery(L"field", L"中国"));
  expectedBuilder->setSlop(3);
  assertEquals(expectedBuilder->build(),
               builder->createPhraseQuery(L"field", L"中国", 3));
}

void TestQueryBuilder::testNoTermAttribute()
{
  // Can't use MockTokenizer because it adds TermAttribute and we don't want
  // that
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<QueryBuilder> builder = make_shared<QueryBuilder>(analyzer);
  assertNull(builder->createBooleanQuery(L"field", L"whatever"));
}

TestQueryBuilder::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestQueryBuilder> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestQueryBuilder::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<TokenizerAnonymousInnerClass>(shared_from_this()));
}

TestQueryBuilder::AnalyzerAnonymousInnerClass::TokenizerAnonymousInnerClass::
    TokenizerAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  wasReset = false;
}

void TestQueryBuilder::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::reset() 
{
  outerInstance->outerInstance.super.reset();
  assertFalse(wasReset);
  wasReset = true;
}

bool TestQueryBuilder::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::incrementToken() 
{
  assertTrue(wasReset);
  return false;
}
} // namespace org::apache::lucene::util