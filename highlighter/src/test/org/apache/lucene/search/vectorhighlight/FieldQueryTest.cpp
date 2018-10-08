using namespace std;

#include "FieldQueryTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using QueryBitSetProducer =
    org::apache::lucene::search::join::QueryBitSetProducer;
using ScoreMode = org::apache::lucene::search::join::ScoreMode;
using ToParentBlockJoinQuery =
    org::apache::lucene::search::join::ToParentBlockJoinQuery;
using QueryPhraseMap =
    org::apache::lucene::search::vectorhighlight::FieldQuery::QueryPhraseMap;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;
using BytesRef = org::apache::lucene::util::BytesRef;

void FieldQueryTest::initBoost()
{
  boost = usually() ? 1.0F : random()->nextFloat() * 10000;
}

void FieldQueryTest::testFlattenBoolean() 
{
  initBoost();
  shared_ptr<BooleanQuery::Builder> booleanQueryB =
      make_shared<BooleanQuery::Builder>();
  booleanQueryB->add(tq(L"A"), Occur::MUST);
  booleanQueryB->add(tq(L"B"), Occur::MUST);
  booleanQueryB->add(tq(L"C"), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(tq(L"D"), Occur::MUST);
  innerQuery->add(tq(L"E"), Occur::MUST);
  booleanQueryB->add(innerQuery->build(), Occur::MUST_NOT);

  shared_ptr<Query> booleanQuery = booleanQueryB->build();
  booleanQuery = make_shared<BoostQuery>(booleanQuery, boost);

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(booleanQuery, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(booleanQuery, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries,
                          {tq(boost, L"A"), tq(boost, L"B"), tq(boost, L"C")});
}

void FieldQueryTest::testFlattenDisjunctionMaxQuery() 
{
  initBoost();
  shared_ptr<Query> query = dmq({tq(L"A"), tq(L"B"), pqF({L"C", L"D"})});
  query = make_shared<BoostQuery>(query, boost);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(query, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries, {tq(boost, L"A"), tq(boost, L"B"),
                                        pqF(boost, L"C", {L"D"})});
}

void FieldQueryTest::testFlattenTermAndPhrase() 
{
  initBoost();
  shared_ptr<BooleanQuery::Builder> booleanQueryB =
      make_shared<BooleanQuery::Builder>();
  booleanQueryB->add(tq(L"A"), Occur::MUST);
  booleanQueryB->add(pqF({L"B", L"C"}), Occur::MUST);
  shared_ptr<Query> booleanQuery = booleanQueryB->build();
  booleanQuery = make_shared<BoostQuery>(booleanQuery, boost);

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(booleanQuery, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(booleanQuery, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries,
                          {tq(boost, L"A"), pqF(boost, L"B", {L"C"})});
}

void FieldQueryTest::testFlattenTermAndPhrase2gram() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(F, L"AA")), Occur::MUST);
  query->add(toPhraseQuery(analyze(L"BCD", F, analyzerB), F), Occur::MUST);
  query->add(toPhraseQuery(analyze(L"EFGH", F, analyzerB), F), Occur::SHOULD);

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(query->build(), reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries, {tq(L"AA"), pqF({L"BC", L"CD"}),
                                        pqF({L"EF", L"FG", L"GH"})});
}

void FieldQueryTest::testFlatten1TermPhrase() 
{
  shared_ptr<Query> query = pqF({L"A"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(query, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries, {tq(L"A")});
}

void FieldQueryTest::testExpand() 
{
  shared_ptr<Query> dummy = pqF({L"DUMMY"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(dummy, true, true);

  // "a b","b c" => "a b","b c","a b c"
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"b", L"c"}));
  assertCollectionQueries(
      fq->expand(flatQueries),
      {pqF({L"a", L"b"}), pqF({L"b", L"c"}), pqF({L"a", L"b", L"c"})});

  // "a b","b c d" => "a b","b c d","a b c d"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"b", L"c", L"d"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), pqF({L"b", L"c", L"d"}),
                           pqF({L"a", L"b", L"c", L"d"})});

  // "a b c","b c d" => "a b c","b c d","a b c d"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  flatQueries->add(pqF({L"b", L"c", L"d"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"c"}), pqF({L"b", L"c", L"d"}),
                           pqF({L"a", L"b", L"c", L"d"})});

  // "a b c","c d e" => "a b c","c d e","a b c d e"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  flatQueries->add(pqF({L"c", L"d", L"e"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"c"}), pqF({L"c", L"d", L"e"}),
                           pqF({L"a", L"b", L"c", L"d", L"e"})});

  // "a b c d","b c" => "a b c d","b c"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"c", L"d"}));
  flatQueries->add(pqF({L"b", L"c"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"c", L"d"}), pqF({L"b", L"c"})});

  // "a b b","b c" => "a b b","b c","a b b c"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"b"}));
  flatQueries->add(pqF({L"b", L"c"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"b"}), pqF({L"b", L"c"}),
                           pqF({L"a", L"b", L"b", L"c"})});

  // "a b","b a" => "a b","b a","a b a", "b a b"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"b", L"a"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), pqF({L"b", L"a"}),
                           pqF({L"a", L"b", L"a"}), pqF({L"b", L"a", L"b"})});

  // "a b","a b c" => "a b","a b c"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), pqF({L"a", L"b", L"c"})});
}

void FieldQueryTest::testNoExpand() 
{
  shared_ptr<Query> dummy = pqF({L"DUMMY"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(dummy, true, true);

  // "a b","c d" => "a b","c d"
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"c", L"d"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), pqF({L"c", L"d"})});

  // "a","a b" => "a", "a b"
  flatQueries = unordered_set<>();
  flatQueries->add(tq(L"a"));
  flatQueries->add(pqF({L"a", L"b"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {tq(L"a"), pqF({L"a", L"b"})});

  // "a b","b" => "a b", "b"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(tq(L"b"));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), tq(L"b")});

  // "a b c","b c" => "a b c","b c"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  flatQueries->add(pqF({L"b", L"c"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"c"}), pqF({L"b", L"c"})});

  // "a b","a b c" => "a b","a b c"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b"}));
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b"}), pqF({L"a", L"b", L"c"})});

  // "a b c","b d e" => "a b c","b d e"
  flatQueries = unordered_set<>();
  flatQueries->add(pqF({L"a", L"b", L"c"}));
  flatQueries->add(pqF({L"b", L"d", L"e"}));
  assertCollectionQueries(fq->expand(flatQueries),
                          {pqF({L"a", L"b", L"c"}), pqF({L"b", L"d", L"e"})});
}

void FieldQueryTest::testExpandNotFieldMatch() 
{
  shared_ptr<Query> dummy = pqF({L"DUMMY"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(dummy, true, false);

  // f1:"a b",f2:"b c" => f1:"a b",f2:"b c",f1:"a b c"
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  flatQueries->add(pq(F1, {L"a", L"b"}));
  flatQueries->add(pq(F2, {L"b", L"c"}));
  assertCollectionQueries(
      fq->expand(flatQueries),
      {pq(F1, {L"a", L"b"}), pq(F2, {L"b", L"c"}), pq(F1, {L"a", L"b", L"c"})});
}

void FieldQueryTest::testGetFieldTermMap() 
{
  shared_ptr<Query> query = tq(L"a");
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);

  shared_ptr<QueryPhraseMap> pqm = fq->getFieldTermMap(F, L"a");
  assertNotNull(pqm);
  assertTrue(pqm->isTerminal());

  pqm = fq->getFieldTermMap(F, L"b");
  assertNull(pqm);

  pqm = fq->getFieldTermMap(F1, L"a");
  assertNull(pqm);
}

void FieldQueryTest::testGetRootMap() 
{
  shared_ptr<Query> dummy = pqF({L"DUMMY"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(dummy, true, true);

  shared_ptr<QueryPhraseMap> rootMap1 = fq->getRootMap(tq(L"a"));
  shared_ptr<QueryPhraseMap> rootMap2 = fq->getRootMap(tq(L"a"));
  assertTrue(rootMap1 == rootMap2);
  shared_ptr<QueryPhraseMap> rootMap3 = fq->getRootMap(tq(L"b"));
  assertTrue(rootMap1 == rootMap3);
  shared_ptr<QueryPhraseMap> rootMap4 = fq->getRootMap(tq(F1, L"b"));
  assertFalse(rootMap4 == rootMap3);
}

void FieldQueryTest::testGetRootMapNotFieldMatch() 
{
  shared_ptr<Query> dummy = pqF({L"DUMMY"});
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(dummy, true, false);

  shared_ptr<QueryPhraseMap> rootMap1 = fq->getRootMap(tq(L"a"));
  shared_ptr<QueryPhraseMap> rootMap2 = fq->getRootMap(tq(L"a"));
  assertTrue(rootMap1 == rootMap2);
  shared_ptr<QueryPhraseMap> rootMap3 = fq->getRootMap(tq(L"b"));
  assertTrue(rootMap1 == rootMap3);
  shared_ptr<QueryPhraseMap> rootMap4 = fq->getRootMap(tq(F1, L"b"));
  assertTrue(rootMap4 == rootMap3);
}

void FieldQueryTest::testGetTermSet() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(F, L"A")), Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(F, L"B")), Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(L"x", L"C")),
             Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> innerQuery =
      make_shared<BooleanQuery::Builder>();
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"D")),
                  Occur::MUST);
  innerQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"E")),
                  Occur::MUST);
  query->add(innerQuery->build(), Occur::MUST_NOT);

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  assertEquals(2, fq->termSetMap.size());
  shared_ptr<Set<wstring>> termSet = fq->getTermSet(F);
  assertEquals(2, termSet->size());
  assertTrue(termSet->contains(L"A"));
  assertTrue(termSet->contains(L"B"));
  termSet = fq->getTermSet(L"x");
  assertEquals(1, termSet->size());
  assertTrue(termSet->contains(L"C"));
  termSet = fq->getTermSet(L"y");
  assertNull(termSet);
}

void FieldQueryTest::testQueryPhraseMap1Term() 
{
  shared_ptr<Query> query = tq(L"a");

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());
  assertTrue(qpm->subMap[L"a"] != nullptr);
  assertTrue(qpm->subMap[L"a"]->terminal);
  assertEquals(1.0F, qpm->subMap[L"a"]->boost, 0);

  // phraseHighlight = true, fieldMatch = false
  fq = make_shared<FieldQuery>(query, true, false);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[F]);
  assertNotNull(map_obj[nullptr]);
  qpm = map_obj[nullptr];
  assertEquals(1, qpm->subMap.size());
  assertTrue(qpm->subMap[L"a"] != nullptr);
  assertTrue(qpm->subMap[L"a"]->terminal);
  assertEquals(1.0F, qpm->subMap[L"a"]->boost, 0);

  // phraseHighlight = false, fieldMatch = true
  fq = make_shared<FieldQuery>(query, false, true);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());
  assertTrue(qpm->subMap[L"a"] != nullptr);
  assertTrue(qpm->subMap[L"a"]->terminal);
  assertEquals(1.0F, qpm->subMap[L"a"]->boost, 0);

  // phraseHighlight = false, fieldMatch = false
  fq = make_shared<FieldQuery>(query, false, false);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[F]);
  assertNotNull(map_obj[nullptr]);
  qpm = map_obj[nullptr];
  assertEquals(1, qpm->subMap.size());
  assertTrue(qpm->subMap[L"a"] != nullptr);
  assertTrue(qpm->subMap[L"a"]->terminal);
  assertEquals(1.0F, qpm->subMap[L"a"]->boost, 0);

  // boost != 1
  query = tq(2, L"a");
  fq = make_shared<FieldQuery>(query, true, true);
  map_obj = fq->rootMaps;
  qpm = map_obj[F];
  assertEquals(2.0F, qpm->subMap[L"a"]->boost, 0);
}

void FieldQueryTest::testQueryPhraseMap1Phrase() 
{
  shared_ptr<Query> query = pqF({L"a", L"b"});

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // phraseHighlight = true, fieldMatch = false
  fq = make_shared<FieldQuery>(query, true, false);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[F]);
  assertNotNull(map_obj[nullptr]);
  qpm = map_obj[nullptr];
  assertEquals(1, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"a"]);
  qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // phraseHighlight = false, fieldMatch = true
  fq = make_shared<FieldQuery>(query, false, true);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  qpm = map_obj[F];
  assertEquals(2, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"a"]);
  qpm2 = qpm->subMap[L"a"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  assertNotNull(qpm->subMap[L"b"]);
  qpm2 = qpm->subMap[L"b"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);

  // phraseHighlight = false, fieldMatch = false
  fq = make_shared<FieldQuery>(query, false, false);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[F]);
  assertNotNull(map_obj[nullptr]);
  qpm = map_obj[nullptr];
  assertEquals(2, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"a"]);
  qpm2 = qpm->subMap[L"a"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  assertNotNull(qpm->subMap[L"b"]);
  qpm2 = qpm->subMap[L"b"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);

  // boost != 1
  query = pqF(2, L"a", {L"b"});
  // phraseHighlight = false, fieldMatch = false
  fq = make_shared<FieldQuery>(query, false, false);
  map_obj = fq->rootMaps;
  qpm = map_obj[nullptr];
  qpm2 = qpm->subMap[L"a"];
  assertEquals(2.0F, qpm2->boost, 0);
  qpm3 = qpm2->subMap[L"b"];
  assertEquals(2.0F, qpm3->boost, 0);
  qpm2 = qpm->subMap[L"b"];
  assertEquals(2.0F, qpm2->boost, 0);
}

void FieldQueryTest::testQueryPhraseMap1PhraseAnother() 
{
  shared_ptr<Query> query = pqF({L"search", L"engines"});

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"search"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"search"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"engines"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"engines"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);
}

void FieldQueryTest::testQueryPhraseMap2Phrases() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"b"}), Occur::SHOULD);
  query->add(pqF(2, L"c", {L"d"}), Occur::SHOULD);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(2, qpm->subMap.size());

  // "a b"
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "c d"^2
  assertNotNull(qpm->subMap[L"c"]);
  qpm2 = qpm->subMap[L"c"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"d"]);
  qpm3 = qpm2->subMap[L"d"];
  assertTrue(qpm3->terminal);
  assertEquals(2.0F, qpm3->boost, 0);
}

void FieldQueryTest::testQueryPhraseMap2PhrasesFields() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pq(F1, {L"a", L"b"}), Occur::SHOULD);
  query->add(pq(2.0F, F2, L"c", {L"d"}), Occur::SHOULD);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(2, map_obj.size());
  assertNull(map_obj[nullptr]);

  // "a b"
  assertNotNull(map_obj[F1]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F1];
  assertEquals(1, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "c d"^2
  assertNotNull(map_obj[F2]);
  qpm = map_obj[F2];
  assertEquals(1, qpm->subMap.size());
  assertNotNull(qpm->subMap[L"c"]);
  qpm2 = qpm->subMap[L"c"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"d"]);
  qpm3 = qpm2->subMap[L"d"];
  assertTrue(qpm3->terminal);
  assertEquals(2.0F, qpm3->boost, 0);

  // phraseHighlight = true, fieldMatch = false
  fq = make_shared<FieldQuery>(query->build(), true, false);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[F1]);
  assertNull(map_obj[F2]);
  assertNotNull(map_obj[nullptr]);
  qpm = map_obj[nullptr];
  assertEquals(2, qpm->subMap.size());

  // "a b"
  assertNotNull(qpm->subMap[L"a"]);
  qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "c d"^2
  assertNotNull(qpm->subMap[L"c"]);
  qpm2 = qpm->subMap[L"c"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"d"]);
  qpm3 = qpm2->subMap[L"d"];
  assertTrue(qpm3->terminal);
  assertEquals(2.0F, qpm3->boost, 0);
}

void FieldQueryTest::testQueryPhraseMapOverlapPhrases() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"b", L"c"}), Occur::SHOULD);
  query->add(pqF(2, L"b", {L"c", L"d"}), Occur::SHOULD);
  query->add(pqF(3, L"b", {L"d"}), Occur::SHOULD);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(2, qpm->subMap.size());

  // "a b c"
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"b"];
  assertFalse(qpm3->terminal);
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"c"]);
  shared_ptr<QueryPhraseMap> qpm4 = qpm3->subMap[L"c"];
  assertTrue(qpm4->terminal);
  assertEquals(1.0F, qpm4->boost, 0);
  assertNotNull(qpm4->subMap[L"d"]);
  shared_ptr<QueryPhraseMap> qpm5 = qpm4->subMap[L"d"];
  assertTrue(qpm5->terminal);
  assertEquals(1.0F, qpm5->boost, 0);

  // "b c d"^2, "b d"^3
  assertNotNull(qpm->subMap[L"b"]);
  qpm2 = qpm->subMap[L"b"];
  assertFalse(qpm2->terminal);
  assertEquals(2, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"c"]);
  qpm3 = qpm2->subMap[L"c"];
  assertFalse(qpm3->terminal);
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"d"]);
  qpm4 = qpm3->subMap[L"d"];
  assertTrue(qpm4->terminal);
  assertEquals(2.0F, qpm4->boost, 0);
  assertNotNull(qpm2->subMap[L"d"]);
  qpm3 = qpm2->subMap[L"d"];
  assertTrue(qpm3->terminal);
  assertEquals(3.0F, qpm3->boost, 0);
}

void FieldQueryTest::testQueryPhraseMapOverlapPhrases2() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"b"}), Occur::SHOULD);
  query->add(pqF(2, L"a", {L"b", L"c"}), Occur::SHOULD);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());

  // "a b"
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"b"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"b"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "a b c"^2
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"c"]);
  shared_ptr<QueryPhraseMap> qpm4 = qpm3->subMap[L"c"];
  assertTrue(qpm4->terminal);
  assertEquals(2.0F, qpm4->boost, 0);
}

void FieldQueryTest::testQueryPhraseMapOverlapPhrases3() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(pqF({L"a", L"a", L"a", L"a"}), Occur::SHOULD);
  query->add(pqF(2, L"a", {L"a", L"a"}), Occur::SHOULD);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(1, qpm->subMap.size());

  // "a a a"
  assertNotNull(qpm->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"a"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"a"];
  assertFalse(qpm3->terminal);
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm4 = qpm3->subMap[L"a"];
  assertTrue(qpm4->terminal);

  // "a a a a"
  assertEquals(1, qpm4->subMap.size());
  assertNotNull(qpm4->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm5 = qpm4->subMap[L"a"];
  assertTrue(qpm5->terminal);

  // "a a a a a"
  assertEquals(1, qpm5->subMap.size());
  assertNotNull(qpm5->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm6 = qpm5->subMap[L"a"];
  assertTrue(qpm6->terminal);

  // "a a a a a a"
  assertEquals(1, qpm6->subMap.size());
  assertNotNull(qpm6->subMap[L"a"]);
  shared_ptr<QueryPhraseMap> qpm7 = qpm6->subMap[L"a"];
  assertTrue(qpm7->terminal);
}

void FieldQueryTest::testQueryPhraseMapOverlap2gram() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(toPhraseQuery(analyze(L"abc", F, analyzerB), F), Occur::MUST);
  query->add(toPhraseQuery(analyze(L"bcd", F, analyzerB), F), Occur::MUST);

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query->build(), true, true);
  unordered_map<wstring, std::shared_ptr<QueryPhraseMap>> &map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  shared_ptr<QueryPhraseMap> qpm = map_obj[F];
  assertEquals(2, qpm->subMap.size());

  // "ab bc"
  assertNotNull(qpm->subMap[L"ab"]);
  shared_ptr<QueryPhraseMap> qpm2 = qpm->subMap[L"ab"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"bc"]);
  shared_ptr<QueryPhraseMap> qpm3 = qpm2->subMap[L"bc"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "ab bc cd"
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"cd"]);
  shared_ptr<QueryPhraseMap> qpm4 = qpm3->subMap[L"cd"];
  assertTrue(qpm4->terminal);
  assertEquals(1.0F, qpm4->boost, 0);

  // "bc cd"
  assertNotNull(qpm->subMap[L"bc"]);
  qpm2 = qpm->subMap[L"bc"];
  assertFalse(qpm2->terminal);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"cd"]);
  qpm3 = qpm2->subMap[L"cd"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // phraseHighlight = false, fieldMatch = true
  fq = make_shared<FieldQuery>(query->build(), false, true);
  map_obj = fq->rootMaps;
  assertEquals(1, map_obj.size());
  assertNull(map_obj[nullptr]);
  assertNotNull(map_obj[F]);
  qpm = map_obj[F];
  assertEquals(3, qpm->subMap.size());

  // "ab bc"
  assertNotNull(qpm->subMap[L"ab"]);
  qpm2 = qpm->subMap[L"ab"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"bc"]);
  qpm3 = qpm2->subMap[L"bc"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "ab bc cd"
  assertEquals(1, qpm3->subMap.size());
  assertNotNull(qpm3->subMap[L"cd"]);
  qpm4 = qpm3->subMap[L"cd"];
  assertTrue(qpm4->terminal);
  assertEquals(1.0F, qpm4->boost, 0);

  // "bc cd"
  assertNotNull(qpm->subMap[L"bc"]);
  qpm2 = qpm->subMap[L"bc"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);
  assertEquals(1, qpm2->subMap.size());
  assertNotNull(qpm2->subMap[L"cd"]);
  qpm3 = qpm2->subMap[L"cd"];
  assertTrue(qpm3->terminal);
  assertEquals(1.0F, qpm3->boost, 0);

  // "cd"
  assertNotNull(qpm->subMap[L"cd"]);
  qpm2 = qpm->subMap[L"cd"];
  assertTrue(qpm2->terminal);
  assertEquals(1.0F, qpm2->boost, 0);
  assertEquals(0, qpm2->subMap.size());
}

void FieldQueryTest::testSearchPhrase() 
{
  shared_ptr<Query> query = pqF({L"a", L"b", L"c"});

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);

  // "a"
  deque<std::shared_ptr<TermInfo>> phraseCandidate =
      deque<std::shared_ptr<TermInfo>>();
  phraseCandidate.push_back(make_shared<TermInfo>(L"a", 0, 1, 0, 1));
  assertNull(fq->searchPhrase(F, phraseCandidate));
  // "a b"
  phraseCandidate.push_back(make_shared<TermInfo>(L"b", 2, 3, 1, 1));
  assertNull(fq->searchPhrase(F, phraseCandidate));
  // "a b c"
  phraseCandidate.push_back(make_shared<TermInfo>(L"c", 4, 5, 2, 1));
  assertNotNull(fq->searchPhrase(F, phraseCandidate));
  assertNull(fq->searchPhrase(L"x", phraseCandidate));

  // phraseHighlight = true, fieldMatch = false
  fq = make_shared<FieldQuery>(query, true, false);

  // "a b c"
  assertNotNull(fq->searchPhrase(F, phraseCandidate));
  assertNotNull(fq->searchPhrase(L"x", phraseCandidate));

  // phraseHighlight = false, fieldMatch = true
  fq = make_shared<FieldQuery>(query, false, true);

  // "a"
  phraseCandidate.clear();
  phraseCandidate.push_back(make_shared<TermInfo>(L"a", 0, 1, 0, 1));
  assertNotNull(fq->searchPhrase(F, phraseCandidate));
  // "a b"
  phraseCandidate.push_back(make_shared<TermInfo>(L"b", 2, 3, 1, 1));
  assertNull(fq->searchPhrase(F, phraseCandidate));
  // "a b c"
  phraseCandidate.push_back(make_shared<TermInfo>(L"c", 4, 5, 2, 1));
  assertNotNull(fq->searchPhrase(F, phraseCandidate));
  assertNull(fq->searchPhrase(L"x", phraseCandidate));
}

void FieldQueryTest::testSearchPhraseSlop() 
{
  // "a b c"~0
  shared_ptr<Query> query = pqF({L"a", L"b", L"c"});

  // phraseHighlight = true, fieldMatch = true
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);

  // "a b c" w/ position-gap = 2
  deque<std::shared_ptr<TermInfo>> phraseCandidate =
      deque<std::shared_ptr<TermInfo>>();
  phraseCandidate.push_back(make_shared<TermInfo>(L"a", 0, 1, 0, 1));
  phraseCandidate.push_back(make_shared<TermInfo>(L"b", 2, 3, 2, 1));
  phraseCandidate.push_back(make_shared<TermInfo>(L"c", 4, 5, 4, 1));
  assertNull(fq->searchPhrase(F, phraseCandidate));

  // "a b c"~1
  query = pqF(1.0F, 1, {L"a", L"b", L"c"});

  // phraseHighlight = true, fieldMatch = true
  fq = make_shared<FieldQuery>(query, true, true);

  // "a b c" w/ position-gap = 2
  assertNotNull(fq->searchPhrase(F, phraseCandidate));

  // "a b c" w/ position-gap = 3
  phraseCandidate.clear();
  phraseCandidate.push_back(make_shared<TermInfo>(L"a", 0, 1, 0, 1));
  phraseCandidate.push_back(make_shared<TermInfo>(L"b", 2, 3, 3, 1));
  phraseCandidate.push_back(make_shared<TermInfo>(L"c", 4, 5, 6, 1));
  assertNull(fq->searchPhrase(F, phraseCandidate));
}

void FieldQueryTest::testHighlightQuery() 
{
  makeIndexStrMV();
  defgMultiTermQueryTest(
      make_shared<WildcardQuery>(make_shared<Term>(F, L"d*g")));
}

void FieldQueryTest::testPrefixQuery() 
{
  makeIndexStrMV();
  defgMultiTermQueryTest(make_shared<PrefixQuery>(make_shared<Term>(F, L"de")));
}

void FieldQueryTest::testRegexpQuery() 
{
  makeIndexStrMV();
  shared_ptr<Term> term = make_shared<Term>(F, L"d[a-z].g");
  defgMultiTermQueryTest(make_shared<RegexpQuery>(term));
}

void FieldQueryTest::testRangeQuery() 
{
  makeIndexStrMV();
  defgMultiTermQueryTest(make_shared<TermRangeQuery>(
      F, make_shared<BytesRef>(L"d"), make_shared<BytesRef>(L"e"), true, true));
}

void FieldQueryTest::defgMultiTermQueryTest(shared_ptr<Query> query) throw(
    IOException)
{
  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(query, reader, true, true);
  shared_ptr<QueryPhraseMap> qpm = fq->getFieldTermMap(F, L"defg");
  assertNotNull(qpm);
  assertNull(fq->getFieldTermMap(F, L"dog"));
  deque<std::shared_ptr<TermInfo>> phraseCandidate =
      deque<std::shared_ptr<TermInfo>>();
  phraseCandidate.push_back(make_shared<TermInfo>(L"defg", 0, 12, 0, 1));
  assertNotNull(fq->searchPhrase(F, phraseCandidate));
}

void FieldQueryTest::testStopRewrite() 
{
  shared_ptr<Query> q =
      make_shared<QueryAnonymousInnerClass>(shared_from_this());
  make1d1fIndex(L"a");
  assertNotNull(reader);
  make_shared<FieldQuery>(q, reader, true, true);
}

FieldQueryTest::QueryAnonymousInnerClass::QueryAnonymousInnerClass(
    shared_ptr<FieldQueryTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

// C++ TODO: There is no native C++ equivalent to 'toString':
wstring
FieldQueryTest::QueryAnonymousInnerClass::Term::toString(const wstring &field)
{
  return L"DummyQuery";
}

bool FieldQueryTest::QueryAnonymousInnerClass::equals(any o)
{
  throw make_shared<AssertionError>();
}

int FieldQueryTest::QueryAnonymousInnerClass::hashCode()
{
  throw make_shared<AssertionError>();
}

void FieldQueryTest::testFlattenConstantScoreQuery() 
{
  initBoost();
  shared_ptr<Query> query = make_shared<ConstantScoreQuery>(pqF({L"A"}));
  query = make_shared<BoostQuery>(query, boost);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(query, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries, {tq(boost, L"A")});
}

void FieldQueryTest::testFlattenToParentBlockJoinQuery() 
{
  initBoost();
  shared_ptr<Query> childQuery = tq(boost, L"a");
  shared_ptr<Query> query = make_shared<ToParentBlockJoinQuery>(
      childQuery,
      make_shared<QueryBitSetProducer>(make_shared<MatchAllDocsQuery>()),
      ScoreMode::None);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<Set<std::shared_ptr<Query>>> flatQueries =
      unordered_set<std::shared_ptr<Query>>();
  fq->flatten(query, reader, flatQueries, 1.0f);
  assertCollectionQueries(flatQueries, {tq(boost, L"a")});
}
} // namespace org::apache::lucene::search::vectorhighlight