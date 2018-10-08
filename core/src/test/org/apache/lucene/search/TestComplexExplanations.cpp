using namespace std;

#include "TestComplexExplanations.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using namespace org::apache::lucene::search::spans;

void TestComplexExplanations::setUp() 
{
  BaseExplanationTestCase::setUp();
  // TODO: switch to BM25?
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
}

void TestComplexExplanations::tearDown() 
{
  searcher->setSimilarity(IndexSearcher::getDefaultSimilarity());
  BaseExplanationTestCase::tearDown();
}

void TestComplexExplanations::test1() 
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, FIELD, L"w1", L"w2");
  q->add(phraseQuery, Occur::MUST);
  q->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true), Occur::SHOULD);
  q->add(snear(sf(L"w3", 2), st(L"w2"), st(L"w3"), 5, true), Occur::SHOULD);

  shared_ptr<Query> t =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                Occur::MUST)
          ->add(matchTheseItems(std::deque<int>{1, 3}), Occur::FILTER)
          ->build();
  q->add(make_shared<BoostQuery>(t, 1000), Occur::SHOULD);

  t = make_shared<ConstantScoreQuery>(matchTheseItems(std::deque<int>{0, 2}));
  q->add(make_shared<BoostQuery>(t, 30), Occur::SHOULD);

  deque<std::shared_ptr<Query>> disjuncts = deque<std::shared_ptr<Query>>();
  disjuncts.push_back(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true));
  disjuncts.push_back(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQ")));

  shared_ptr<BooleanQuery::Builder> xxYYZZ =
      make_shared<BooleanQuery::Builder>();
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
              Occur::SHOULD);
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
              Occur::SHOULD);
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
              Occur::MUST_NOT);

  disjuncts.push_back(xxYYZZ->build());

  shared_ptr<BooleanQuery::Builder> xxW1 = make_shared<BooleanQuery::Builder>();
  xxW1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
            Occur::MUST_NOT);
  xxW1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
            Occur::MUST_NOT);

  disjuncts.push_back(xxW1->build());

  deque<std::shared_ptr<Query>> disjuncts2 = deque<std::shared_ptr<Query>>();
  disjuncts2.push_back(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")));
  disjuncts2.push_back(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")));
  disjuncts2.push_back(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")));
  disjuncts.push_back(make_shared<DisjunctionMaxQuery>(disjuncts2, 0.5f));

  q->add(make_shared<DisjunctionMaxQuery>(disjuncts, 0.2f), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> b = make_shared<BooleanQuery::Builder>();
  b->setMinimumNumberShouldMatch(2);
  b->add(snear(L"w1", L"w2", 1, true), Occur::SHOULD);
  b->add(snear(L"w2", L"w3", 1, true), Occur::SHOULD);
  b->add(snear(L"w1", L"w3", 3, true), Occur::SHOULD);

  q->add(b->build(), Occur::SHOULD);

  qtest(q->build(), std::deque<int>{0, 1, 2});
}

void TestComplexExplanations::test2() 
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, FIELD, L"w1", L"w2");
  q->add(phraseQuery, Occur::MUST);
  q->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true), Occur::SHOULD);
  q->add(snear(sf(L"w3", 2), st(L"w2"), st(L"w3"), 5, true), Occur::SHOULD);

  shared_ptr<Query> t =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                Occur::MUST)
          ->add(matchTheseItems(std::deque<int>{1, 3}), Occur::FILTER)
          ->build();
  q->add(make_shared<BoostQuery>(t, 1000), Occur::SHOULD);

  t = make_shared<ConstantScoreQuery>(matchTheseItems(std::deque<int>{0, 2}));
  q->add(make_shared<BoostQuery>(t, -20), Occur::SHOULD);

  deque<std::shared_ptr<Query>> disjuncts = deque<std::shared_ptr<Query>>();
  disjuncts.push_back(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true));
  disjuncts.push_back(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQ")));

  shared_ptr<BooleanQuery::Builder> xxYYZZ =
      make_shared<BooleanQuery::Builder>();
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
              Occur::SHOULD);
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
              Occur::SHOULD);
  xxYYZZ->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
              Occur::MUST_NOT);

  disjuncts.push_back(xxYYZZ->build());

  shared_ptr<BooleanQuery::Builder> xxW1 = make_shared<BooleanQuery::Builder>();
  xxW1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
            Occur::MUST_NOT);
  xxW1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
            Occur::MUST_NOT);

  disjuncts.push_back(xxW1->build());

  shared_ptr<DisjunctionMaxQuery> dm2 = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3"))),
      0.5f);
  disjuncts.push_back(dm2);

  q->add(make_shared<DisjunctionMaxQuery>(disjuncts, 0.2f), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  builder->setMinimumNumberShouldMatch(2);
  builder->add(snear(L"w1", L"w2", 1, true), Occur::SHOULD);
  builder->add(snear(L"w2", L"w3", 1, true), Occur::SHOULD);
  builder->add(snear(L"w1", L"w3", 3, true), Occur::SHOULD);
  shared_ptr<BooleanQuery> b = builder->build();

  q->add(make_shared<BoostQuery>(b, 0), Occur::SHOULD);

  qtest(q->build(), std::deque<int>{0, 1, 2});
}

void TestComplexExplanations::testT3() 
{
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  bqtest(make_shared<BoostQuery>(query, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testMA3() 
{
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  bqtest(make_shared<BoostQuery>(q, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testFQ5() 
{
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"));
  shared_ptr<Query> filtered =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BoostQuery>(query, 0), Occur::MUST)
          ->add(matchTheseItems(std::deque<int>{1, 3}), Occur::FILTER)
          ->build();
  bqtest(filtered, std::deque<int>{3});
}

void TestComplexExplanations::testCSQ4() 
{
  shared_ptr<Query> q =
      make_shared<ConstantScoreQuery>(matchTheseItems(std::deque<int>{3}));
  bqtest(make_shared<BoostQuery>(q, 0), std::deque<int>{3});
}

void TestComplexExplanations::testDMQ10() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             Occur::SHOULD);
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"));
  query->add(make_shared<BoostQuery>(boostedQuery, 100), Occur::SHOULD);

  shared_ptr<TermQuery> xxBoostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"));

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(query->build(),
                     make_shared<BoostQuery>(xxBoostedQuery, 0)),
      0.5f);
  bqtest(make_shared<BoostQuery>(q, 0), std::deque<int>{0, 2, 3});
}

void TestComplexExplanations::testMPQ7() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1"}));
  qb->add(ta(std::deque<wstring>{L"w2"}));
  qb->setSlop(1);
  bqtest(make_shared<BoostQuery>(qb->build(), 0), std::deque<int>{0, 1, 2});
}

void TestComplexExplanations::testBQ12() 
{
  // NOTE: using qtest not bqtest
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             Occur::SHOULD);
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2"));
  query->add(make_shared<BoostQuery>(boostedQuery, 0), Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testBQ13() 
{
  // NOTE: using qtest not bqtest
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             Occur::SHOULD);
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"));
  query->add(make_shared<BoostQuery>(boostedQuery, 0), Occur::MUST_NOT);

  qtest(query->build(), std::deque<int>{1, 2, 3});
}

void TestComplexExplanations::testBQ18() 
{
  // NOTE: using qtest not bqtest
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  query->add(make_shared<BoostQuery>(boostedQuery, 0), Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
             Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testBQ21() 
{
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  builder->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
               Occur::MUST);
  builder->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
               Occur::SHOULD);

  shared_ptr<Query> query = builder->build();

  bqtest(make_shared<BoostQuery>(query, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testBQ22() 
{
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  builder->add(make_shared<BoostQuery>(boostedQuery, 0), Occur::MUST);
  builder->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
               Occur::SHOULD);
  shared_ptr<BooleanQuery> query = builder->build();

  bqtest(make_shared<BoostQuery>(query, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testST3() 
{
  shared_ptr<SpanQuery> q = st(L"w1");
  bqtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testST6() 
{
  shared_ptr<SpanQuery> q = st(L"xx");
  qtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{2, 3});
}

void TestComplexExplanations::testSF3() 
{
  shared_ptr<SpanQuery> q = sf((L"w1"), 1);
  bqtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testSF7() 
{
  shared_ptr<SpanQuery> q = sf((L"xx"), 3);
  bqtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{2, 3});
}

void TestComplexExplanations::testSNot3() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"QQ"));
  bqtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testSNot6() 
{
  shared_ptr<SpanQuery> q = snot(sf(L"w1", 10), st(L"xx"));
  bqtest(make_shared<SpanBoostQuery>(q, 0), std::deque<int>{0, 1, 2, 3});
}

void TestComplexExplanations::testSNot8() 
{
  // NOTE: using qtest not bqtest
  shared_ptr<SpanQuery> f = snear(L"w1", L"w3", 10, true);
  f = make_shared<SpanBoostQuery>(f, 0);
  shared_ptr<SpanQuery> q = snot(f, st(L"xx"));
  qtest(q, std::deque<int>{0, 1, 3});
}

void TestComplexExplanations::testSNot9() 
{
  // NOTE: using qtest not bqtest
  shared_ptr<SpanQuery> t = st(L"xx");
  t = make_shared<SpanBoostQuery>(t, 0);
  shared_ptr<SpanQuery> q = snot(snear(L"w1", L"w3", 10, true), t);
  qtest(q, std::deque<int>{0, 1, 3});
}
} // namespace org::apache::lucene::search