using namespace std;

#include "TestIndexReaderFunctions.h"

namespace org::apache::lucene::queries::function
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using CheckHits = org::apache::lucene::search::CheckHits;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory> TestIndexReaderFunctions::dir;
shared_ptr<org::apache::lucene::analysis::Analyzer>
    TestIndexReaderFunctions::analyzer;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestIndexReaderFunctions::reader;
shared_ptr<org::apache::lucene::search::IndexSearcher>
    TestIndexReaderFunctions::searcher;
const deque<std::deque<wstring>> TestIndexReaderFunctions::documents =
    java::util::Arrays::asList(
        std::deque<wstring>{L"0", L"3.63", L"5.2", L"35", L"4343", L"test",
                             L"this is a test test test", L"2.13", L"3.69",
                             L"-0.11", L"1", L"7", L"5"},
        std::deque<wstring>{L"1", L"5.65", L"9.3", L"54", L"1954", L"bar",
                             L"second test", L"12.79", L"123.456", L"0.01",
                             L"12", L"900", L"-1"});

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestIndexReaderFunctions::beforeClass() 
{
  dir = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwConfig = newIndexWriterConfig(analyzer);
  iwConfig->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConfig);
  for (auto doc : documents) {
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(
        make_shared<StringField>(L"id", doc[0], Field::Store::NO));
    document->push_back(make_shared<SortedDocValuesField>(
        L"id", make_shared<BytesRef>(doc[0])));
    document->push_back(
        make_shared<StringField>(L"string", doc[5], Field::Store::NO));
    document->push_back(make_shared<SortedDocValuesField>(
        L"string", make_shared<BytesRef>(doc[5])));
    document->push_back(
        make_shared<TextField>(L"text", doc[6], Field::Store::NO));
    iw->addDocument(document);
  }

  reader = iw->getReader();
  searcher = newSearcher(reader);
  delete iw;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestIndexReaderFunctions::afterClass() 
{
  IOUtils::close({reader, dir, analyzer});
  searcher.reset();
  reader.reset();
  dir.reset();
  analyzer.reset();
}

void TestIndexReaderFunctions::testDocFreq() 
{
  shared_ptr<DoubleValuesSource> vs =
      IndexReaderFunctions::docFreq(make_shared<Term>(L"text", L"test"));
  assertHits(vs, std::deque<float>{2.0f, 2.0f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"docFreq(text:test)", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testMaxDoc() 
{
  shared_ptr<DoubleValuesSource> vs = IndexReaderFunctions::maxDoc();
  assertHits(vs, std::deque<float>{2.0f, 2.0f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"maxDoc()", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testNumDocs() 
{
  shared_ptr<DoubleValuesSource> vs = IndexReaderFunctions::numDocs();
  assertHits(vs, std::deque<float>{2.0f, 2.0f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"numDocs()", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testSumTotalTermFreq() 
{
  shared_ptr<LongValuesSource> vs =
      IndexReaderFunctions::sumTotalTermFreq(L"text");
  assertHits(vs->toDoubleValuesSource(), std::deque<float>{8.0f, 8.0f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"sumTotalTermFreq(text)", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testTermFreq() 
{
  assertHits(
      IndexReaderFunctions::termFreq(make_shared<Term>(L"string", L"bar")),
      std::deque<float>{0.0f, 1.0f});
  assertHits(
      IndexReaderFunctions::termFreq(make_shared<Term>(L"text", L"test")),
      std::deque<float>{3.0f, 1.0f});
  assertHits(
      IndexReaderFunctions::termFreq(make_shared<Term>(L"bogus", L"bogus")),
      std::deque<float>{0.0F, 0.0F});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"termFreq(string:bar)",
      IndexReaderFunctions::termFreq(make_shared<Term>(L"string", L"bar"))
          ->toString());
  assertCacheable(
      IndexReaderFunctions::termFreq(make_shared<Term>(L"text", L"test")),
      true);
}

void TestIndexReaderFunctions::testTotalTermFreq() 
{
  shared_ptr<DoubleValuesSource> vs =
      IndexReaderFunctions::totalTermFreq(make_shared<Term>(L"text", L"test"));
  assertHits(vs, std::deque<float>{4.0f, 4.0f});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"totalTermFreq(text:test)", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testNumDeletedDocs() 
{
  shared_ptr<DoubleValuesSource> vs = IndexReaderFunctions::numDeletedDocs();
  assertHits(vs, std::deque<float>{0, 0});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"numDeletedDocs()", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testSumDocFreq() 
{
  shared_ptr<DoubleValuesSource> vs = IndexReaderFunctions::sumDocFreq(L"text");
  assertHits(vs, std::deque<float>{6, 6});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"sumDocFreq(text)", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::testDocCount() 
{
  shared_ptr<DoubleValuesSource> vs = IndexReaderFunctions::docCount(L"text");
  assertHits(vs, std::deque<float>{2, 2});
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"docCount(text)", vs->toString());
  assertCacheable(vs, false);
}

void TestIndexReaderFunctions::assertCacheable(
    shared_ptr<DoubleValuesSource> vs, bool expected) 
{
  shared_ptr<Query> q =
      make_shared<FunctionScoreQuery>(make_shared<MatchAllDocsQuery>(), vs);
  shared_ptr<Weight> w = searcher->createWeight(q, true, 1);
  shared_ptr<LeafReaderContext> ctx = reader->leaves()[0];
  assertEquals(expected, w->isCacheable(ctx));
}

void TestIndexReaderFunctions::assertCacheable(
    shared_ptr<LongValuesSource> vs, bool expected) 
{
  shared_ptr<Query> q = make_shared<FunctionScoreQuery>(
      make_shared<MatchAllDocsQuery>(), vs->toDoubleValuesSource());
  shared_ptr<Weight> w = searcher->createWeight(q, true, 1);
  shared_ptr<LeafReaderContext> ctx = reader->leaves()[0];
  assertEquals(expected, w->isCacheable(ctx));
}

void TestIndexReaderFunctions::assertHits(
    shared_ptr<DoubleValuesSource> vs,
    std::deque<float> &scores) 
{
  shared_ptr<Query> q =
      make_shared<FunctionScoreQuery>(make_shared<MatchAllDocsQuery>(), vs);
  std::deque<std::shared_ptr<ScoreDoc>> expected(scores.size());
  std::deque<int> expectedDocs(scores.size());
  for (int i = 0; i < expected.size(); i++) {
    expectedDocs[i] = i;
    expected[i] = make_shared<ScoreDoc>(i, scores[i]);
  }
  shared_ptr<TopDocs> docs = searcher->search(
      q, documents.size(),
      make_shared<Sort>(make_shared<SortField>(L"id", SortField::Type::STRING)),
      true, false);
  CheckHits::checkHits(random(), q, L"", searcher, expectedDocs);
  CheckHits::checkHitsQuery(q, expected, docs->scoreDocs, expectedDocs);
  CheckHits::checkExplanations(q, L"", searcher);
  assertSort(vs, expected);
}

void TestIndexReaderFunctions::assertSort(
    shared_ptr<DoubleValuesSource> vs,
    std::deque<std::shared_ptr<ScoreDoc>> &expected) 
{
  bool reversed = random()->nextBoolean();
  Arrays::sort(expected, [&](a, b) {
    reversed ? static_cast<int>(b::score - a::score)
             : static_cast<int>(a::score - b::score);
  });
  std::deque<int> expectedDocs(expected.size());
  for (int i = 0; i < expected.size(); i++) {
    expectedDocs[i] = expected[i]->doc;
  }
  shared_ptr<TopDocs> docs =
      searcher->search(make_shared<MatchAllDocsQuery>(), expected.size(),
                       make_shared<Sort>(vs->getSortField(reversed)));
  CheckHits::checkHitsQuery(make_shared<MatchAllDocsQuery>(), expected,
                            docs->scoreDocs, expectedDocs);
}
} // namespace org::apache::lucene::queries::function