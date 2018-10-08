using namespace std;

#include "TestSubScorerFreqs.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ChildScorer = org::apache::lucene::search::Scorer::ChildScorer;
using BasicStats = org::apache::lucene::search::similarities::BasicStats;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;
shared_ptr<org::apache::lucene::store::Directory> TestSubScorerFreqs::dir;
shared_ptr<IndexSearcher> TestSubScorerFreqs::s;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void makeIndex() throws Exception
void TestSubScorerFreqs::makeIndex() 
{
  dir = make_shared<RAMDirectory>();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  // make sure we have more than one segment occationally
  int num = atLeast(31);
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"f", L"a b c d b c d c d d", Field::Store::NO));
    w->addDocument(doc);

    doc = make_shared<Document>();
    doc->push_back(newTextField(L"f", L"a b c d", Field::Store::NO));
    w->addDocument(doc);
  }

  s = newSearcher(w->getReader());
  s->setSimilarity(make_shared<CountingSimilarity>());
  delete w;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void finish() throws Exception
void TestSubScorerFreqs::finish() 
{
  delete s->getIndexReader();
  s.reset();
  delete dir;
  dir.reset();
}

TestSubScorerFreqs::CountingCollector::CountingCollector(
    shared_ptr<Collector> other)
    : CountingCollector(
          other, new HashSet<>(Arrays::asList(L"MUST", L"SHOULD", L"MUST_NOT")))
{
}

TestSubScorerFreqs::CountingCollector::CountingCollector(
    shared_ptr<Collector> other, shared_ptr<Set<wstring>> relationships)
    : FilterCollector(other), relationships(relationships)
{
}

void TestSubScorerFreqs::CountingCollector::setSubScorers(
    shared_ptr<Scorer> scorer, const wstring &relationship) 
{
  for (auto child : scorer->getChildren()) {
    if (std::dynamic_pointer_cast<AssertingScorer>(scorer) != nullptr ||
        relationships->contains(child->relationship)) {
      setSubScorers(child->child, child->relationship);
    }
  }
  subScorers.emplace(scorer->getWeight()->getQuery(), scorer);
}

shared_ptr<LeafCollector>
TestSubScorerFreqs::CountingCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  constexpr int docBase = context->docBase;
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this(), FilterCollector::getLeafCollector(context), docBase);
}

TestSubScorerFreqs::CountingCollector::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<CountingCollector> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> getLeafCollector,
        int docBase)
    : FilterLeafCollector(getLeafCollector)
{
  this->outerInstance = outerInstance;
  this->docBase = docBase;
}

void TestSubScorerFreqs::CountingCollector::
    FilterLeafCollectorAnonymousInnerClass::collect(int doc) 
{
  const unordered_map<std::shared_ptr<Query>, float> freqs =
      unordered_map<std::shared_ptr<Query>, float>();
  for (auto ent : outerInstance->subScorers) {
    shared_ptr<Scorer> value = ent.second;
    int matchId = value->docID();
    freqs.emplace(ent.first, matchId == doc ? value->score() : 0.0f);
  }
  outerInstance->docCounts.emplace(doc + docBase, freqs);
  outerInstance->super->collect(doc);
}

void TestSubScorerFreqs::CountingCollector::
    FilterLeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  outerInstance->super->setScorer(scorer);
  outerInstance->subScorers.clear();
  outerInstance->setSubScorers(scorer, L"TOP");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTermQuery() throws Exception
void TestSubScorerFreqs::testTermQuery() 
{
  shared_ptr<TermQuery> q =
      make_shared<TermQuery>(make_shared<Term>(L"f", L"d"));
  shared_ptr<CountingCollector> c =
      make_shared<CountingCollector>(TopScoreDocCollector::create(10));
  s->search(q, c);
  constexpr int maxDocs = s->getIndexReader()->maxDoc();
  assertEquals(maxDocs, c->docCounts.size());
  for (int i = 0; i < maxDocs; i++) {
    unordered_map<std::shared_ptr<Query>, float> doc0 = c->docCounts[i];
    assertEquals(1, doc0.size());
    assertEquals(4.0F, doc0[q], FLOAT_TOLERANCE);

    unordered_map<std::shared_ptr<Query>, float> doc1 = c->docCounts[++i];
    assertEquals(1, doc1.size());
    assertEquals(1.0F, doc1[q], FLOAT_TOLERANCE);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBooleanQuery() throws Exception
void TestSubScorerFreqs::testBooleanQuery() 
{
  shared_ptr<TermQuery> aQuery =
      make_shared<TermQuery>(make_shared<Term>(L"f", L"a"));
  shared_ptr<TermQuery> dQuery =
      make_shared<TermQuery>(make_shared<Term>(L"f", L"d"));
  shared_ptr<TermQuery> cQuery =
      make_shared<TermQuery>(make_shared<Term>(L"f", L"c"));
  shared_ptr<TermQuery> yQuery =
      make_shared<TermQuery>(make_shared<Term>(L"f", L"y"));

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();

  inner->add(cQuery, Occur::SHOULD);
  inner->add(yQuery, Occur::MUST_NOT);
  query->add(inner->build(), Occur::MUST);
  query->add(aQuery, Occur::MUST);
  query->add(dQuery, Occur::MUST);

  // Only needed in Java6; Java7+ has a @SafeVarargs annotated Arrays#asList()!
  // see http://docs.oracle.com/javase/7/docs/api/java/lang/SafeVarargs.html
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") final
  // Iterable<java.util.Set<std::wstring>> occurList =
  // java.util.Arrays.asList(java.util.Collections.singleton("MUST"), new
  // java.util.HashSet<>(java.util.Arrays.asList("MUST", "SHOULD"))
  const deque<Set<wstring>> occurList = Arrays::asList(
      Collections::singleton(L"MUST"),
      unordered_set<Set<wstring>>(Arrays::asList(L"MUST", L"SHOULD")));

  for (auto occur : occurList) {
    shared_ptr<CountingCollector> c =
        make_shared<CountingCollector>(TopScoreDocCollector::create(10), occur);
    s->search(query->build(), c);
    constexpr int maxDocs = s->getIndexReader()->maxDoc();
    assertEquals(maxDocs, c->docCounts.size());
    bool includeOptional = occur->contains(L"SHOULD");
    for (int i = 0; i < maxDocs; i++) {
      unordered_map<std::shared_ptr<Query>, float> doc0 = c->docCounts[i];
      // Y doesnt exist in the index, so it's not in the scorer tree
      assertEquals(4, doc0.size());
      assertEquals(1.0F, doc0[aQuery], FLOAT_TOLERANCE);
      assertEquals(4.0F, doc0[dQuery], FLOAT_TOLERANCE);
      if (includeOptional) {
        assertEquals(3.0F, doc0[cQuery], FLOAT_TOLERANCE);
      }

      unordered_map<std::shared_ptr<Query>, float> doc1 = c->docCounts[++i];
      // Y doesnt exist in the index, so it's not in the scorer tree
      assertEquals(4, doc1.size());
      assertEquals(1.0F, doc1[aQuery], FLOAT_TOLERANCE);
      assertEquals(1.0F, doc1[dQuery], FLOAT_TOLERANCE);
      if (includeOptional) {
        assertEquals(1.0F, doc1[cQuery], FLOAT_TOLERANCE);
      }
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPhraseQuery() throws Exception
void TestSubScorerFreqs::testPhraseQuery() 
{
  shared_ptr<PhraseQuery> q = make_shared<PhraseQuery>(L"f", L"b", L"c");
  shared_ptr<CountingCollector> c =
      make_shared<CountingCollector>(TopScoreDocCollector::create(10));
  s->search(q, c);
  constexpr int maxDocs = s->getIndexReader()->maxDoc();
  assertEquals(maxDocs, c->docCounts.size());
  for (int i = 0; i < maxDocs; i++) {
    unordered_map<std::shared_ptr<Query>, float> doc0 = c->docCounts[i];
    assertEquals(1, doc0.size());
    assertEquals(2.0F, doc0[q], FLOAT_TOLERANCE);

    unordered_map<std::shared_ptr<Query>, float> doc1 = c->docCounts[++i];
    assertEquals(1, doc1.size());
    assertEquals(1.0F, doc1[q], FLOAT_TOLERANCE);
  }
}

int64_t TestSubScorerFreqs::CountingSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return 1;
}

shared_ptr<Similarity::SimWeight>
TestSubScorerFreqs::CountingSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<BasicStats>(L"", boost);
}

shared_ptr<Similarity::SimScorer>
TestSubScorerFreqs::CountingSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestSubScorerFreqs::CountingSimilarity::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<CountingSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestSubScorerFreqs::CountingSimilarity::SimScorerAnonymousInnerClass::
    score(int doc, float freq) 
{
  return freq;
}

float TestSubScorerFreqs::CountingSimilarity::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 1;
}

float TestSubScorerFreqs::CountingSimilarity::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return 1;
}
} // namespace org::apache::lucene::search