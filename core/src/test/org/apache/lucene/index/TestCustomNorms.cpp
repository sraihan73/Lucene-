using namespace std;

#include "TestCustomNorms.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
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
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TestCustomNorms::FLOAT_TEST_FIELD = L"normsTestFloat";
const wstring TestCustomNorms::EXCEPTION_TEST_FIELD = L"normsTestExcp";

void TestCustomNorms::testFloatNorms() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  shared_ptr<IndexWriterConfig> config = newIndexWriterConfig(analyzer);
  shared_ptr<Similarity> provider =
      make_shared<MySimProvider>(shared_from_this());
  config->setSimilarity(provider);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, config);
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  int num = atLeast(100);
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    int boost = TestUtil::nextInt(random(), 1, 10);
    wstring value = IntStream::range(0, boost)
                        .mapToObj([&](any k) { Integer::toString(boost); })
                        .collect(Collectors::joining(L" "));
    shared_ptr<Field> f =
        make_shared<TextField>(FLOAT_TEST_FIELD, value, Field::Store::YES);

    doc->push_back(f);
    writer->addDocument(doc);
    doc->removeField(FLOAT_TEST_FIELD);
    if (rarely()) {
      writer->commit();
    }
  }
  writer->commit();
  delete writer;
  shared_ptr<DirectoryReader> open = DirectoryReader::open(dir);
  shared_ptr<NumericDocValues> norms =
      MultiDocValues::getNormValues(open, FLOAT_TEST_FIELD);
  assertNotNull(norms);
  for (int i = 0; i < open->maxDoc(); i++) {
    shared_ptr<Document> document = open->document(i);
    int expected =
        static_cast<Integer>(document[FLOAT_TEST_FIELD]->split(L" ")[0]);
    TestUtil::assertEquals(i, norms->nextDoc());
    TestUtil::assertEquals(expected, norms->longValue());
  }
  open->close();
  delete dir;
  delete docs;
}

TestCustomNorms::MySimProvider::MySimProvider(
    shared_ptr<TestCustomNorms> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<Similarity> TestCustomNorms::MySimProvider::get(const wstring &field)
{
  if (FLOAT_TEST_FIELD == field) {
    return make_shared<FloatEncodingBoostSimilarity>();
  } else {
    return delegate_;
  }
}

int64_t TestCustomNorms::FloatEncodingBoostSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return state->getLength();
}

shared_ptr<Similarity::SimWeight>
TestCustomNorms::FloatEncodingBoostSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestCustomNorms::FloatEncodingBoostSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::index