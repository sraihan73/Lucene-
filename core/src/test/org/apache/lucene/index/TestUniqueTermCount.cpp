using namespace std;

#include "TestUniqueTermCount.h"

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
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestUniqueTermCount::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<IndexWriterConfig> config = newIndexWriterConfig(analyzer);
  config->setMergePolicy(newLogMergePolicy());
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

void TestUniqueTermCount::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestUniqueTermCount::test() 
{
  shared_ptr<NumericDocValues> fooNorms =
      MultiDocValues::getNormValues(reader, L"foo");
  assertNotNull(fooNorms);
  for (int i = 0; i < reader->maxDoc(); i++) {
    TestUtil::assertEquals(i, fooNorms->nextDoc());
    TestUtil::assertEquals(expected[i], fooNorms->longValue());
  }
}

wstring TestUniqueTermCount::addValue()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  unordered_set<wstring> terms = unordered_set<wstring>();
  int num = TestUtil::nextInt(random(), 0, 255);
  for (int i = 0; i < num; i++) {
    sb->append(L' ');
    wchar_t term =
        static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
    sb->append(term);
    terms.insert(L"" + StringHelper::toString(term));
  }
  expected.push_back(terms.size());
  return sb->toString();
}

int64_t TestUniqueTermCount::TestSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return state->getUniqueTermCount();
}

shared_ptr<Similarity::SimWeight>
TestUniqueTermCount::TestSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestUniqueTermCount::TestSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::index