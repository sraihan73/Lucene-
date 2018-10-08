using namespace std;

#include "TestSimilarityProvider.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSimilarityProvider::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<PerFieldSimilarityWrapper> sim =
      make_shared<ExampleSimilarityProvider>();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setSimilarity(sim);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newTextField(L"foo", L"", Field::Store::NO);
  doc->push_back(field);
  shared_ptr<Field> field2 = newTextField(L"bar", L"", Field::Store::NO);
  doc->push_back(field2);

  field->setStringValue(L"quick brown fox");
  field2->setStringValue(L"quick brown fox");
  iw->addDocument(doc);
  field->setStringValue(L"jumps over lazy brown dog");
  field2->setStringValue(L"jumps over lazy brown dog");
  iw->addDocument(doc);
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
  searcher->setSimilarity(sim);
}

void TestSimilarityProvider::tearDown() 
{
  reader->close();
  delete directory;
  LuceneTestCase::tearDown();
}

void TestSimilarityProvider::testBasics() 
{
  // sanity check of norms writer
  // TODO: generalize
  shared_ptr<NumericDocValues> fooNorms =
      MultiDocValues::getNormValues(reader, L"foo");
  shared_ptr<NumericDocValues> barNorms =
      MultiDocValues::getNormValues(reader, L"bar");
  for (int i = 0; i < reader->maxDoc(); i++) {
    assertEquals(i, fooNorms->nextDoc());
    assertEquals(i, barNorms->nextDoc());
    assertFalse(fooNorms->longValue() == barNorms->longValue());
  }

  // sanity check of searching
  shared_ptr<TopDocs> foodocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"brown")), 10);
  assertTrue(foodocs->totalHits > 0);
  shared_ptr<TopDocs> bardocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"bar", L"brown")), 10);
  assertTrue(bardocs->totalHits > 0);
  assertTrue(foodocs->scoreDocs[0]->score < bardocs->scoreDocs[0]->score);
}

shared_ptr<Similarity>
TestSimilarityProvider::ExampleSimilarityProvider::get(const wstring &field)
{
  if (field == L"foo") {
    return sim1;
  } else {
    return sim2;
  }
}

int64_t
TestSimilarityProvider::Sim1::computeNorm(shared_ptr<FieldInvertState> state)
{
  return 1;
}

shared_ptr<Similarity::SimWeight> TestSimilarityProvider::Sim1::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<SimWeightAnonymousInnerClass>(shared_from_this());
}

TestSimilarityProvider::Sim1::SimWeightAnonymousInnerClass::
    SimWeightAnonymousInnerClass(shared_ptr<Sim1> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Similarity::SimScorer> TestSimilarityProvider::Sim1::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestSimilarityProvider::Sim1::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<Sim1> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestSimilarityProvider::Sim1::SimScorerAnonymousInnerClass::score(
    int doc, float freq) 
{
  return 1;
}

float TestSimilarityProvider::Sim1::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 1;
}

float TestSimilarityProvider::Sim1::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return 1;
}

int64_t
TestSimilarityProvider::Sim2::computeNorm(shared_ptr<FieldInvertState> state)
{
  return 10;
}

shared_ptr<Similarity::SimWeight> TestSimilarityProvider::Sim2::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<SimWeightAnonymousInnerClass>(shared_from_this());
}

TestSimilarityProvider::Sim2::SimWeightAnonymousInnerClass::
    SimWeightAnonymousInnerClass(shared_ptr<Sim2> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Similarity::SimScorer> TestSimilarityProvider::Sim2::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestSimilarityProvider::Sim2::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<Sim2> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestSimilarityProvider::Sim2::SimScorerAnonymousInnerClass::score(
    int doc, float freq) 
{
  return 10;
}

float TestSimilarityProvider::Sim2::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 1;
}

float TestSimilarityProvider::Sim2::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return 1;
}
} // namespace org::apache::lucene::search