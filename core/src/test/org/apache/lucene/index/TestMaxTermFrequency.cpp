using namespace std;

#include "TestMaxTermFrequency.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestMaxTermFrequency::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true))
          ->setMergePolicy(newLogMergePolicy());
  config->setSimilarity(make_shared<TestSimilarity>());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, config);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> foo = newTextField(L"foo", L"", Field::Store::NO);
  doc->push_back(foo);
  for (int i = 0; i < 100; i++) {
    foo->setStringValue(addValue());
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  delete writer;
}

void TestMaxTermFrequency::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestMaxTermFrequency::test() 
{
  shared_ptr<NumericDocValues> fooNorms =
      MultiDocValues::getNormValues(reader, L"foo");
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, fooNorms->nextDoc());
    TestUtil::assertEquals(expected[i], fooNorms->longValue() & 0xff);
  }
}

wstring TestMaxTermFrequency::addValue()
{
  deque<wstring> terms = deque<wstring>();
  int maxCeiling = TestUtil::nextInt(random(), 0, 255);
  int max = 0;
  for (wchar_t ch = L'a'; ch <= L'z'; ch++) {
    int num = TestUtil::nextInt(random(), 0, maxCeiling);
    for (int i = 0; i < num; i++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      terms.push_back(Character::toString(ch));
    }
    max = max(max, num);
  }
  expected.push_back(max);
  Collections::shuffle(terms, random());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Arrays->toString(terms.toArray(std::deque<wstring>(terms.size())));
}

int64_t TestMaxTermFrequency::TestSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return state->getMaxTermFrequency();
}

shared_ptr<Similarity::SimWeight>
TestMaxTermFrequency::TestSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<SimWeightAnonymousInnerClass>(shared_from_this());
}

TestMaxTermFrequency::TestSimilarity::SimWeightAnonymousInnerClass::
    SimWeightAnonymousInnerClass(shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Similarity::SimScorer>
TestMaxTermFrequency::TestSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

TestMaxTermFrequency::TestSimilarity::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

float TestMaxTermFrequency::TestSimilarity::SimScorerAnonymousInnerClass::score(
    int doc, float freq) 
{
  return 0;
}

float TestMaxTermFrequency::TestSimilarity::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  return 0;
}

float TestMaxTermFrequency::TestSimilarity::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  return 0;
}
} // namespace org::apache::lucene::index