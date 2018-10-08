using namespace std;

#include "TestQueryRescorer.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<IndexSearcher>
TestQueryRescorer::getSearcher(shared_ptr<IndexReader> r)
{
  shared_ptr<IndexSearcher> searcher = newSearcher(r);

  // We rely on more tokens = lower score:
  searcher->setSimilarity(make_shared<ClassicSimilarity>());

  return searcher;
}

shared_ptr<IndexWriterConfig> TestQueryRescorer::newIndexWriterConfig()
{
  // We rely on more tokens = lower score:
  return LuceneTestCase::newIndexWriterConfig()->setSimilarity(
      make_shared<ClassicSimilarity>());
}

void TestQueryRescorer::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  doc->push_back(newTextField(L"field", L"wizard the the the the the oz",
                              Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  // 1 extra token, but wizard and oz are close;
  doc->push_back(newTextField(L"field", L"wizard oz the the the the the the",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Do ordinary BooleanQuery:
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wizard")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"oz")),
          Occur::SHOULD);
  shared_ptr<IndexSearcher> searcher = getSearcher(r);
  searcher->setSimilarity(make_shared<ClassicSimilarity>());

  shared_ptr<TopDocs> hits = searcher->search(bq->build(), 10);
  TestUtil::assertEquals(2, hits->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits->scoreDocs[1]->doc)[L"id"]);

  // Now, resort using PhraseQuery:
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(5, L"field", L"wizard", L"oz");

  shared_ptr<TopDocs> hits2 =
      QueryRescorer::rescore(searcher, hits, pq, 2.0, 10);

  // Resorting changed the order:
  TestUtil::assertEquals(2, hits2->totalHits);
  TestUtil::assertEquals(L"1", searcher->doc(hits2->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"0", searcher->doc(hits2->scoreDocs[1]->doc)[L"id"]);

  // Resort using SpanNearQuery:
  shared_ptr<SpanTermQuery> t1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"wizard"));
  shared_ptr<SpanTermQuery> t2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"oz"));
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{t1, t2}, 0, true);

  shared_ptr<TopDocs> hits3 =
      QueryRescorer::rescore(searcher, hits, snq, 2.0, 10);

  // Resorting changed the order:
  TestUtil::assertEquals(2, hits3->totalHits);
  TestUtil::assertEquals(L"1", searcher->doc(hits3->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"0", searcher->doc(hits3->scoreDocs[1]->doc)[L"id"]);

  delete r;
  delete dir;
}

void TestQueryRescorer::testNullScorerTermQuery() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  doc->push_back(newTextField(L"field", L"wizard the the the the the oz",
                              Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  // 1 extra token, but wizard and oz are close;
  doc->push_back(newTextField(L"field", L"wizard oz the the the the the the",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Do ordinary BooleanQuery:
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wizard")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"oz")),
          Occur::SHOULD);
  shared_ptr<IndexSearcher> searcher = getSearcher(r);
  searcher->setSimilarity(make_shared<ClassicSimilarity>());

  shared_ptr<TopDocs> hits = searcher->search(bq->build(), 10);
  TestUtil::assertEquals(2, hits->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits->scoreDocs[1]->doc)[L"id"]);

  // Now, resort using TermQuery on term that does not exist.
  shared_ptr<TermQuery> tq =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"gold"));
  shared_ptr<TopDocs> hits2 =
      QueryRescorer::rescore(searcher, hits, tq, 2.0, 10);

  // Just testing that null scorer is handled.
  TestUtil::assertEquals(2, hits2->totalHits);

  delete r;
  delete dir;
}

void TestQueryRescorer::testCustomCombine() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  doc->push_back(newTextField(L"field", L"wizard the the the the the oz",
                              Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  // 1 extra token, but wizard and oz are close;
  doc->push_back(newTextField(L"field", L"wizard oz the the the the the the",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Do ordinary BooleanQuery:
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wizard")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"oz")),
          Occur::SHOULD);
  shared_ptr<IndexSearcher> searcher = getSearcher(r);

  shared_ptr<TopDocs> hits = searcher->search(bq->build(), 10);
  TestUtil::assertEquals(2, hits->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits->scoreDocs[1]->doc)[L"id"]);

  // Now, resort using PhraseQuery, but with an
  // opposite-world combine:
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(5, L"field", L"wizard", L"oz");

  shared_ptr<TopDocs> hits2 =
      make_shared<QueryRescorerAnonymousInnerClass>(shared_from_this(), pq)
          .rescore(searcher, hits, 10);

  // Resorting didn't change the order:
  TestUtil::assertEquals(2, hits2->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits2->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits2->scoreDocs[1]->doc)[L"id"]);

  delete r;
  delete dir;
}

TestQueryRescorer::QueryRescorerAnonymousInnerClass::
    QueryRescorerAnonymousInnerClass(
        shared_ptr<TestQueryRescorer> outerInstance,
        shared_ptr<org::apache::lucene::search::PhraseQuery> pq)
    : QueryRescorer(pq)
{
  this->outerInstance = outerInstance;
}

float TestQueryRescorer::QueryRescorerAnonymousInnerClass::combine(
    float firstPassScore, bool secondPassMatches, float secondPassScore)
{
  float score = firstPassScore;
  if (secondPassMatches) {
    score -= 2.0 * secondPassScore;
  }
  return score;
}

void TestQueryRescorer::testExplain() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  doc->push_back(newTextField(L"field", L"wizard the the the the the oz",
                              Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  // 1 extra token, but wizard and oz are close;
  doc->push_back(newTextField(L"field", L"wizard oz the the the the the the",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Do ordinary BooleanQuery:
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wizard")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"oz")),
          Occur::SHOULD);
  shared_ptr<IndexSearcher> searcher = getSearcher(r);

  shared_ptr<TopDocs> hits = searcher->search(bq->build(), 10);
  TestUtil::assertEquals(2, hits->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits->scoreDocs[1]->doc)[L"id"]);

  // Now, resort using PhraseQuery:
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"field", L"wizard", L"oz");

  shared_ptr<Rescorer> rescorer =
      make_shared<QueryRescorerAnonymousInnerClass2>(shared_from_this(), pq);

  shared_ptr<TopDocs> hits2 = rescorer->rescore(searcher, hits, 10);

  // Resorting changed the order:
  TestUtil::assertEquals(2, hits2->totalHits);
  TestUtil::assertEquals(L"1", searcher->doc(hits2->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"0", searcher->doc(hits2->scoreDocs[1]->doc)[L"id"]);

  int docID = hits2->scoreDocs[0]->doc;
  shared_ptr<Explanation> explain =
      rescorer->explain(searcher, searcher->explain(bq->build(), docID), docID);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = explain->toString();
  assertTrue(s.find(L"TestQueryRescorer$") != wstring::npos);
  assertTrue(s.find(L"combined first and second pass score") != wstring::npos);
  assertTrue(s.find(L"first pass score") != wstring::npos);
  assertTrue(s.find(L"= second pass score") != wstring::npos);
  assertEquals(hits2->scoreDocs[0]->score, explain->getValue(), 0.0f);

  docID = hits2->scoreDocs[1]->doc;
  explain =
      rescorer->explain(searcher, searcher->explain(bq->build(), docID), docID);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  s = explain->toString();
  assertTrue(s.find(L"TestQueryRescorer$") != wstring::npos);
  assertTrue(s.find(L"combined first and second pass score") != wstring::npos);
  assertTrue(s.find(L"first pass score") != wstring::npos);
  assertTrue(s.find(L"no second pass score") != wstring::npos);
  assertFalse(s.find(L"= second pass score") != wstring::npos);
  assertEquals(hits2->scoreDocs[1]->score, explain->getValue(), 0.0f);

  delete r;
  delete dir;
}

TestQueryRescorer::QueryRescorerAnonymousInnerClass2::
    QueryRescorerAnonymousInnerClass2(
        shared_ptr<TestQueryRescorer> outerInstance,
        shared_ptr<org::apache::lucene::search::PhraseQuery> pq)
    : QueryRescorer(pq)
{
  this->outerInstance = outerInstance;
}

float TestQueryRescorer::QueryRescorerAnonymousInnerClass2::combine(
    float firstPassScore, bool secondPassMatches, float secondPassScore)
{
  float score = firstPassScore;
  if (secondPassMatches) {
    score += 2.0 * secondPassScore;
  }
  return score;
}

void TestQueryRescorer::testMissingSecondPassScore() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::YES));
  doc->push_back(newTextField(L"field", L"wizard the the the the the oz",
                              Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  // 1 extra token, but wizard and oz are close;
  doc->push_back(newTextField(L"field", L"wizard oz the the the the the the",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Do ordinary BooleanQuery:
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wizard")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"oz")),
          Occur::SHOULD);
  shared_ptr<IndexSearcher> searcher = getSearcher(r);

  shared_ptr<TopDocs> hits = searcher->search(bq->build(), 10);
  TestUtil::assertEquals(2, hits->totalHits);
  TestUtil::assertEquals(L"0", searcher->doc(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", searcher->doc(hits->scoreDocs[1]->doc)[L"id"]);

  // Now, resort using PhraseQuery, no slop:
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"field", L"wizard", L"oz");

  shared_ptr<TopDocs> hits2 =
      QueryRescorer::rescore(searcher, hits, pq, 2.0, 10);

  // Resorting changed the order:
  TestUtil::assertEquals(2, hits2->totalHits);
  TestUtil::assertEquals(L"1", searcher->doc(hits2->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"0", searcher->doc(hits2->scoreDocs[1]->doc)[L"id"]);

  // Resort using SpanNearQuery:
  shared_ptr<SpanTermQuery> t1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"wizard"));
  shared_ptr<SpanTermQuery> t2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"oz"));
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{t1, t2}, 0, true);

  shared_ptr<TopDocs> hits3 =
      QueryRescorer::rescore(searcher, hits, snq, 2.0, 10);

  // Resorting changed the order:
  TestUtil::assertEquals(2, hits3->totalHits);
  TestUtil::assertEquals(L"1", searcher->doc(hits3->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"0", searcher->doc(hits3->scoreDocs[1]->doc)[L"id"]);

  delete r;
  delete dir;
}

void TestQueryRescorer::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  int numDocs = atLeast(1000);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  const std::deque<int> idToNum = std::deque<int>(numDocs);
  int maxValue = TestUtil::nextInt(random(), 10, 1000000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    int numTokens = TestUtil::nextInt(random(), 1, 10);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (int j = 0; j < numTokens; j++) {
      b->append(L"a ");
    }
    doc->push_back(newTextField(L"field", b->toString(), Field::Store::NO));
    idToNum[i] = random()->nextInt(maxValue);
    doc->push_back(make_shared<NumericDocValuesField>(L"num", idToNum[i]));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);
  int numHits = TestUtil::nextInt(random(), 1, numDocs);
  bool reverse = random()->nextBoolean();

  // System.out.println("numHits=" + numHits + " reverse=" + reverse);
  shared_ptr<TopDocs> hits = s->search(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"a")), numHits);

  shared_ptr<TopDocs> hits2 =
      make_shared<QueryRescorerAnonymousInnerClass3>(
          shared_from_this(), make_shared<FixedScoreQuery>(idToNum, reverse))
          .rescore(s, hits, numHits);

  std::deque<optional<int>> expected(numHits);
  for (int i = 0; i < numHits; i++) {
    expected[i] = hits->scoreDocs[i]->doc;
  }

  constexpr int reverseInt = reverse ? -1 : 1;

  Arrays::sort(expected, make_shared<ComparatorAnonymousInnerClass>(
                             shared_from_this(), idToNum, r, reverseInt));

  bool fail = false;
  for (int i = 0; i < numHits; i++) {
    // System.out.println("expected=" + expected[i] + " vs " +
    // hits2.scoreDocs[i].doc + " v=" +
    // idToNum[Integer.parseInt(r.document(expected[i]).get("id"))]);
    if (expected[i] != hits2->scoreDocs[i]->doc) {
      // System.out.println("  diff!");
      fail = true;
    }
  }
  assertFalse(fail);

  delete r;
  delete dir;
}

TestQueryRescorer::QueryRescorerAnonymousInnerClass3::
    QueryRescorerAnonymousInnerClass3(
        shared_ptr<TestQueryRescorer> outerInstance,
        shared_ptr<FixedScoreQuery> new) new
    : QueryRescorer(new FixedScoreQuery)
{
  this->outerInstance = outerInstance;
}

float TestQueryRescorer::QueryRescorerAnonymousInnerClass3::combine(
    float firstPassScore, bool secondPassMatches, float secondPassScore)
{
  return secondPassScore;
}

TestQueryRescorer::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestQueryRescorer> outerInstance, deque<int> &idToNum,
    shared_ptr<IndexReader> r, int reverseInt)
{
  this->outerInstance = outerInstance;
  this->idToNum = idToNum;
  this->r = r;
  this->reverseInt = reverseInt;
}

int TestQueryRescorer::ComparatorAnonymousInnerClass::compare(optional<int> &a,
                                                              optional<int> &b)
{
  try {
    int av = idToNum[static_cast<Integer>(r->document(a)[L"id"])];
    int bv = idToNum[static_cast<Integer>(r->document(b)[L"id"])];
    if (av < bv) {
      return -reverseInt;
    } else if (bv < av) {
      return reverseInt;
    } else {
      // Tie break by docID, ascending
      return a - b;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

TestQueryRescorer::FixedScoreQuery::FixedScoreQuery(std::deque<int> &idToNum,
                                                    bool reverse)
    : idToNum(idToNum), reverse(reverse)
{
}

shared_ptr<Weight> TestQueryRescorer::FixedScoreQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{

  return make_shared<WeightAnonymousInnerClass>(shared_from_this(),
                                                FixedScoreQuery::this);
}

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    WeightAnonymousInnerClass(
        shared_ptr<FixedScoreQuery> outerInstance,
        shared_ptr<
            org::apache::lucene::search::TestQueryRescorer::FixedScoreQuery>
            shared_from_this())
    : Weight(this)
{
  this->outerInstance = outerInstance;
}

void TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    extractTerms(shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Scorer>
TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{

  return make_shared<ScorerAnonymousInnerClass>(shared_from_this(), context);
}

TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::ScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReaderContext> context)
    : Scorer(nullptr)
{
  this->outerInstance = outerInstance;
  this->context = context;
  docID = -1;
}

int TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::docID()
{
  return docID;
}

shared_ptr<DocIdSetIterator> TestQueryRescorer::FixedScoreQuery::
    WeightAnonymousInnerClass::ScorerAnonymousInnerClass::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass::
        DocIdSetIteratorAnonymousInnerClass(
            shared_ptr<ScorerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return docID;
}

int64_t TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return 1;
}

int TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass::nextDoc()
{
  docID++;
  if (docID >= outerInstance->context.reader().maxDoc()) {
    return NO_MORE_DOCS;
  }
  return docID;
}

int TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass::advance(
        int target)
{
  docID = target;
  return docID;
}

float TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::score() 
{
  int num = outerInstance->outerInstance->idToNum[static_cast<Integer>(
      context->reader()->document(docID)[L"id"])];
  if (outerInstance->outerInstance->reverse) {
    // System.out.println("score doc=" + docID + " num=" + num);
    return num;
  } else {
    // System.out.println("score doc=" + docID + " num=" + -num);
    return -num;
  }
}

bool TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation>
TestQueryRescorer::FixedScoreQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return nullptr;
}

wstring TestQueryRescorer::FixedScoreQuery::toString(const wstring &field)
{
  return L"FixedScoreQuery " + idToNum.size() + L" ids; reverse=" +
         StringHelper::toString(reverse);
}

bool TestQueryRescorer::FixedScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TestQueryRescorer::FixedScoreQuery::equalsTo(
    shared_ptr<FixedScoreQuery> other)
{
  return reverse == other->reverse && Arrays::equals(idToNum, other->idToNum);
}

int TestQueryRescorer::FixedScoreQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + (reverse ? 0 : 1);
  hash = 31 * hash + Arrays::hashCode(idToNum);
  return hash;
}

shared_ptr<Query> TestQueryRescorer::FixedScoreQuery::clone()
{
  return make_shared<FixedScoreQuery>(idToNum, reverse);
}
} // namespace org::apache::lucene::search