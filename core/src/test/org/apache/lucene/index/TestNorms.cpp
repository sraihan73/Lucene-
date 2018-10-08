using namespace std;

#include "TestNorms.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::Slow;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TestNorms::BYTE_TEST_FIELD = L"normsTestByte";

void TestNorms::testMaxByteNorms() 
{
  shared_ptr<Directory> dir =
      newFSDirectory(createTempDir(L"TestNorms.testMaxByteNorms"));
  buildIndex(dir);
  shared_ptr<DirectoryReader> open = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> normValues =
      MultiDocValues::getNormValues(open, BYTE_TEST_FIELD);
  assertNotNull(normValues);
  for (int i = 0; i < open->maxDoc(); i++) {
    shared_ptr<Document> document = open->document(i);
    int expected =
        static_cast<Integer>(document[BYTE_TEST_FIELD]->split(L" ")[0]);
    TestUtil::assertEquals(i, normValues->nextDoc());
    TestUtil::assertEquals(expected, normValues->longValue());
  }
  open->close();
  delete dir;
}

void TestNorms::buildIndex(shared_ptr<Directory> dir) 
{
  shared_ptr<Random> random = TestNorms::random();
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(TestNorms::random());
  // we need at least 3 for maxTokenLength otherwise norms are messed up
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(TestNorms::random(), 3, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> config = newIndexWriterConfig(analyzer);
  shared_ptr<Similarity> provider =
      make_shared<MySimProvider>(shared_from_this());
  config->setSimilarity(provider);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random, dir, config);
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random);
  int num = atLeast(100);
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    int boost = TestUtil::nextInt(random, 1, 255);
    wstring value = IntStream::range(0, boost)
                        .mapToObj([&](any k) { Integer::toString(boost); })
                        .collect(Collectors::joining(L" "));
    shared_ptr<Field> f =
        make_shared<TextField>(BYTE_TEST_FIELD, value, Field::Store::YES);
    doc->push_back(f);
    writer->addDocument(doc);
    doc->removeField(BYTE_TEST_FIELD);
    if (rarely()) {
      writer->commit();
    }
  }
  writer->commit();
  delete writer;
  delete docs;
}

TestNorms::MySimProvider::MySimProvider(shared_ptr<TestNorms> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Similarity> TestNorms::MySimProvider::get(const wstring &field)
{
  if (BYTE_TEST_FIELD == field) {
    return make_shared<ByteEncodingBoostSimilarity>();
  } else {
    return delegate_;
  }
}

int64_t TestNorms::ByteEncodingBoostSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return state->getLength();
}

shared_ptr<Similarity::SimWeight>
TestNorms::ByteEncodingBoostSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestNorms::ByteEncodingBoostSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

void TestNorms::testEmptyValueVsNoValue() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> cfg =
      newIndexWriterConfig()->setMergePolicy(newLogMergePolicy());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, cfg);
  shared_ptr<Document> doc = make_shared<Document>();
  w->addDocument(doc);
  doc->push_back(newTextField(L"foo", L"", Field::Store::NO));
  w->addDocument(doc);
  w->forceMerge(1);
  shared_ptr<IndexReader> reader = DirectoryReader::open(w);
  delete w;
  shared_ptr<LeafReader> leafReader = getOnlyLeafReader(reader);
  shared_ptr<NumericDocValues> normValues = leafReader->getNormValues(L"foo");
  assertNotNull(normValues);
  TestUtil::assertEquals(1, normValues->nextDoc()); // doc 0 does not have norms
  TestUtil::assertEquals(0, normValues->longValue());
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::index