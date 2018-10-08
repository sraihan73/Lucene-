using namespace std;

#include "TestSimpleExplanations.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using org::junit::Test;

void TestSimpleExplanations::testT1() 
{
  qtest(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
        std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testT2() 
{
  shared_ptr<TermQuery> termQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  qtest(make_shared<BoostQuery>(termQuery, 100), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMA1() 
{
  qtest(make_shared<MatchAllDocsQuery>(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMA2() 
{
  shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testP1() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD, L"w1", L"w2");
  qtest(phraseQuery, std::deque<int>{0});
}

void TestSimpleExplanations::testP2() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(FIELD, L"w1", L"w3");
  qtest(phraseQuery, std::deque<int>{1, 3});
}

void TestSimpleExplanations::testP3() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, FIELD, L"w1", L"w2");
  qtest(phraseQuery, std::deque<int>{0, 1, 2});
}

void TestSimpleExplanations::testP4() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, FIELD, L"w2", L"w3");
  qtest(phraseQuery, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testP5() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(1, FIELD, L"w3", L"w2");
  qtest(phraseQuery, std::deque<int>{1, 3});
}

void TestSimpleExplanations::testP6() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(2, FIELD, L"w3", L"w2");
  qtest(phraseQuery, std::deque<int>{0, 1, 3});
}

void TestSimpleExplanations::testP7() 
{
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(3, FIELD, L"w3", L"w2");
  qtest(phraseQuery, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testCSQ1() 
{
  shared_ptr<Query> q = make_shared<ConstantScoreQuery>(
      matchTheseItems(std::deque<int>{0, 1, 2, 3}));
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testCSQ2() 
{
  shared_ptr<Query> q =
      make_shared<ConstantScoreQuery>(matchTheseItems(std::deque<int>{1, 3}));
  qtest(q, std::deque<int>{1, 3});
}

void TestSimpleExplanations::testCSQ3() 
{
  shared_ptr<Query> q =
      make_shared<ConstantScoreQuery>(matchTheseItems(std::deque<int>{0, 2}));
  qtest(make_shared<BoostQuery>(q, 1000), std::deque<int>{0, 2});
}

void TestSimpleExplanations::testDMQ1() 
{
  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"))),
      0.0f);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testDMQ2() 
{
  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"))),
      0.5f);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testDMQ3() 
{
  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQ")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"))),
      0.5f);
  qtest(q, std::deque<int>{0});
}

void TestSimpleExplanations::testDMQ4() 
{
  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQ")),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"))),
      0.5f);
  qtest(q, std::deque<int>{2, 3});
}

void TestSimpleExplanations::testDMQ5() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
                    BooleanClause::Occur::SHOULD);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQ")),
                    BooleanClause::Occur::MUST_NOT);

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(booleanQuery->build(),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"))),
      0.5f);
  qtest(q, std::deque<int>{2, 3});
}

void TestSimpleExplanations::testDMQ6() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
                    BooleanClause::Occur::MUST_NOT);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(booleanQuery->build(),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"))),
      0.5f);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testDMQ7() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
                    BooleanClause::Occur::MUST_NOT);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(booleanQuery->build(),
                     make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2"))),
      0.5f);
  qtest(q, std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testDMQ8() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"));
  booleanQuery->add(make_shared<BoostQuery>(boostedQuery, 100),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<TermQuery> xxBoostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"));

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(booleanQuery->build(),
                     make_shared<BoostQuery>(xxBoostedQuery, 100000)),
      0.5f);
  qtest(q, std::deque<int>{0, 2, 3});
}

void TestSimpleExplanations::testDMQ9() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5"));
  booleanQuery->add(make_shared<BoostQuery>(boostedQuery, 100),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<TermQuery> xxBoostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx"));

  shared_ptr<DisjunctionMaxQuery> q = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(booleanQuery->build(),
                     make_shared<BoostQuery>(xxBoostedQuery, 0)),
      0.5f);

  qtest(q, std::deque<int>{0, 2, 3});
}

void TestSimpleExplanations::testMPQ1() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1"}));
  qb->add(ta(std::deque<wstring>{L"w2", L"w3", L"xx"}));
  qtest(qb->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMPQ2() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1"}));
  qb->add(ta(std::deque<wstring>{L"w2", L"w3"}));
  qtest(qb->build(), std::deque<int>{0, 1, 3});
}

void TestSimpleExplanations::testMPQ3() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1", L"xx"}));
  qb->add(ta(std::deque<wstring>{L"w2", L"w3"}));
  qtest(qb->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMPQ4() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1"}));
  qb->add(ta(std::deque<wstring>{L"w2"}));
  qtest(qb->build(), std::deque<int>{0});
}

void TestSimpleExplanations::testMPQ5() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1"}));
  qb->add(ta(std::deque<wstring>{L"w2"}));
  qb->setSlop(1);
  qtest(qb->build(), std::deque<int>{0, 1, 2});
}

void TestSimpleExplanations::testMPQ6() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(ta(std::deque<wstring>{L"w1", L"w3"}));
  qb->add(ta(std::deque<wstring>{L"w2"}));
  qb->setSlop(1);
  qtest(qb->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ1() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
             BooleanClause::Occur::MUST);
  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ2() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
             BooleanClause::Occur::MUST);
  qtest(query->build(), std::deque<int>{2, 3});
}

void TestSimpleExplanations::testBQ3() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
             BooleanClause::Occur::MUST);
  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ4() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                  BooleanClause::Occur::MUST_NOT);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ5() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ6() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::MUST_NOT);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST_NOT);

  qtest(outerQuery->build(), std::deque<int>{1, 2, 3});
}

void TestSimpleExplanations::testBQ7() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::SHOULD);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::MUST_NOT);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::SHOULD);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST);

  qtest(outerQuery->build(), std::deque<int>{0});
}

void TestSimpleExplanations::testBQ8() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::SHOULD);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::MUST_NOT);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::SHOULD);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ9() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::MUST_NOT);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::SHOULD);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::MUST_NOT);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ10() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::MUST_NOT);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::SHOULD);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::MUST_NOT);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST);

  qtest(outerQuery->build(), std::deque<int>{1});
}

void TestSimpleExplanations::testBQ11() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::SHOULD);
  shared_ptr<TermQuery> boostedQuery =
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1"));
  query->add(make_shared<BoostQuery>(boostedQuery, 1000),
             BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ14() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQQQQ")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
         BooleanClause::Occur::SHOULD);
  qtest(q->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ15() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQQQQ")),
         BooleanClause::Occur::MUST_NOT);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
         BooleanClause::Occur::SHOULD);
  qtest(q->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ16() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQQQQ")),
         BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                    BooleanClause::Occur::SHOULD);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                    BooleanClause::Occur::MUST_NOT);

  q->add(booleanQuery->build(), BooleanClause::Occur::SHOULD);
  qtest(q->build(), std::deque<int>{0, 1});
}

void TestSimpleExplanations::testBQ17() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
         BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                    BooleanClause::Occur::SHOULD);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                    BooleanClause::Occur::MUST_NOT);

  q->add(booleanQuery->build(), BooleanClause::Occur::SHOULD);
  qtest(q->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ19() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             BooleanClause::Occur::MUST_NOT);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w3")),
             BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1});
}

void TestSimpleExplanations::testBQ20() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->setMinimumNumberShouldMatch(2);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"QQQQQ")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w5")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
         BooleanClause::Occur::SHOULD);

  qtest(q->build(), std::deque<int>{0, 3});
}

void TestSimpleExplanations::testBQ21() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
         BooleanClause::Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
         BooleanClause::Occur::SHOULD);

  qtest(q->build(), std::deque<int>{1, 2, 3});
}

void TestSimpleExplanations::testBQ23() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::FILTER);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
             BooleanClause::Occur::FILTER);
  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ24() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::FILTER);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
             BooleanClause::Occur::SHOULD);
  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ25() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::FILTER);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
             BooleanClause::Occur::MUST);
  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testBQ26() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::FILTER);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
             BooleanClause::Occur::MUST_NOT);
  qtest(query->build(), std::deque<int>{0, 1});
}

void TestSimpleExplanations::testMultiFieldBQ1() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w2")),
             BooleanClause::Occur::MUST);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ2() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
             BooleanClause::Occur::MUST);

  qtest(query->build(), std::deque<int>{2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ3() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"yy")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
             BooleanClause::Occur::MUST);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ4() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                  BooleanClause::Occur::MUST_NOT);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w2")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ5() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"qq")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w2")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ6() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"qq")),
                  BooleanClause::Occur::MUST_NOT);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w5")),
                  BooleanClause::Occur::SHOULD);
  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST_NOT);

  qtest(outerQuery->build(), std::deque<int>{1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ7() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"xx")),
                 BooleanClause::Occur::SHOULD);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w2")),
                 BooleanClause::Occur::MUST_NOT);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::SHOULD);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST);

  qtest(outerQuery->build(), std::deque<int>{0});
}

void TestSimpleExplanations::testMultiFieldBQ8() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"xx")),
                 BooleanClause::Occur::SHOULD);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::MUST_NOT);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::SHOULD);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ9() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::MUST_NOT);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w2")),
                 BooleanClause::Occur::SHOULD);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::MUST_NOT);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::SHOULD);

  qtest(outerQuery->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQ10() 
{
  shared_ptr<BooleanQuery::Builder> outerQuery =
      make_shared<BooleanQuery::Builder>();
  outerQuery->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
                  BooleanClause::Occur::MUST);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"qq")),
                  BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childLeft =
      make_shared<BooleanQuery::Builder>();
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"xx")),
                 BooleanClause::Occur::MUST_NOT);
  childLeft->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w2")),
                 BooleanClause::Occur::SHOULD);
  innerQuery->add(childLeft->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> childRight =
      make_shared<BooleanQuery::Builder>();
  childRight->add(make_shared<TermQuery>(make_shared<Term>(ALTFIELD, L"w3")),
                  BooleanClause::Occur::MUST);
  childRight->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w4")),
                  BooleanClause::Occur::MUST);
  innerQuery->add(childRight->build(), BooleanClause::Occur::MUST_NOT);

  outerQuery->add(innerQuery->build(), BooleanClause::Occur::MUST);

  qtest(outerQuery->build(), std::deque<int>{1});
}

void TestSimpleExplanations::testMultiFieldBQofPQ1() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(FIELD, L"w1", L"w2");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(ALTFIELD, L"w1", L"w2");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0});
}

void TestSimpleExplanations::testMultiFieldBQofPQ2() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(FIELD, L"w1", L"w3");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(ALTFIELD, L"w1", L"w3");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{1, 3});
}

void TestSimpleExplanations::testMultiFieldBQofPQ3() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(1, FIELD, L"w1", L"w2");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(1, ALTFIELD, L"w1", L"w2");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2});
}

void TestSimpleExplanations::testMultiFieldBQofPQ4() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(1, FIELD, L"w2", L"w3");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(1, ALTFIELD, L"w2", L"w3");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testMultiFieldBQofPQ5() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(1, FIELD, L"w3", L"w2");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(1, ALTFIELD, L"w3", L"w2");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{1, 3});
}

void TestSimpleExplanations::testMultiFieldBQofPQ6() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(2, FIELD, L"w3", L"w2");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(2, ALTFIELD, L"w3", L"w2");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 3});
}

void TestSimpleExplanations::testMultiFieldBQofPQ7() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();

  shared_ptr<PhraseQuery> leftChild =
      make_shared<PhraseQuery>(3, FIELD, L"w3", L"w2");
  query->add(leftChild, BooleanClause::Occur::SHOULD);

  shared_ptr<PhraseQuery> rightChild =
      make_shared<PhraseQuery>(1, ALTFIELD, L"w3", L"w2");
  query->add(rightChild, BooleanClause::Occur::SHOULD);

  qtest(query->build(), std::deque<int>{0, 1, 2, 3});
}

void TestSimpleExplanations::testSynonymQuery() 
{
  shared_ptr<SynonymQuery> query = make_shared<SynonymQuery>(
      make_shared<Term>(FIELD, L"w1"), make_shared<Term>(FIELD, L"w2"));
  qtest(query, std::deque<int>{0, 1, 2, 3});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEquality()
void TestSimpleExplanations::testEquality()
{

  shared_ptr<Explanation> e1 = Explanation::match(1.0f, L"an explanation");
  shared_ptr<Explanation> e2 = Explanation::match(
      1.0f, L"an explanation", Explanation::match(1.0f, L"a subexplanation"));
  shared_ptr<Explanation> e25 = Explanation::match(
      1.0f, L"an explanation",
      Explanation::match(1.0f, L"a subexplanation",
                         Explanation::match(1.0f, L"a subsubexplanation")));
  shared_ptr<Explanation> e3 = Explanation::match(1.0f, L"an explanation");
  shared_ptr<Explanation> e4 = Explanation::match(2.0f, L"an explanation");
  shared_ptr<Explanation> e5 = Explanation::noMatch(L"an explanation");
  shared_ptr<Explanation> e6 = Explanation::noMatch(
      L"an explanation", Explanation::match(1.0f, L"a subexplanation"));
  shared_ptr<Explanation> e7 = Explanation::noMatch(L"an explanation");
  shared_ptr<Explanation> e8 = Explanation::match(1.0f, L"another explanation");

  assertEquals(e1, e3);
  assertFalse(e1->equals(e2));
  assertFalse(e2->equals(e25));
  assertFalse(e1->equals(e4));
  assertFalse(e1->equals(e5));
  assertEquals(e5, e7);
  assertFalse(e5->equals(e6));
  assertFalse(e1->equals(e8));

  assertEquals(e1->hashCode(), e3->hashCode());
  assertEquals(e5->hashCode(), e7->hashCode());
}
} // namespace org::apache::lucene::search