using namespace std;

#include "TestClassicSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Document = org::apache::lucene::document::Document;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using Store = org::apache::lucene::document::Field::Store;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using MultiReader = org::apache::lucene::index::MultiReader;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Term = org::apache::lucene::index::Term;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using IDFStats =
    org::apache::lucene::search::similarities::TFIDFSimilarity::IDFStats;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestClassicSimilarity::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter
  // indexWriter = new org.apache.lucene.index.IndexWriter(directory,
  // newIndexWriterConfig()))
  {
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(directory,
                                                newIndexWriterConfig());
    shared_ptr<Document> document = make_shared<Document>();
    document->push_back(make_shared<StringField>(L"test", L"hit", Store::NO));
    indexWriter->addDocument(document);
    indexWriter->commit();
  }
  indexReader = DirectoryReader::open(directory);
  indexSearcher = newSearcher(indexReader);
  indexSearcher->setSimilarity(make_shared<ClassicSimilarity>());
}

void TestClassicSimilarity::tearDown() 
{
  IOUtils::close({indexReader, directory});
  LuceneTestCase::tearDown();
}

void TestClassicSimilarity::testHit() 
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"test", L"hit"));
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testMiss() 
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"test", L"miss"));
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(0, topDocs->totalHits);
}

void TestClassicSimilarity::testEmpty() 
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"empty", L"miss"));
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(0, topDocs->totalHits);
}

void TestClassicSimilarity::testBQHit() 
{
  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"hit")),
                Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testBQHitOrMiss() 
{
  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"hit")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"miss")),
                Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testBQHitOrEmpty() 
{
  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"test", L"hit")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"empty", L"miss")),
                Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testDMQHit() 
{
  shared_ptr<Query> query =
      make_shared<DisjunctionMaxQuery>(Arrays::asList(make_shared<TermQuery>(
                                           make_shared<Term>(L"test", L"hit"))),
                                       0);
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testDMQHitOrMiss() 
{
  shared_ptr<Query> query = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(
          make_shared<TermQuery>(make_shared<Term>(L"test", L"hit")),
          make_shared<TermQuery>(make_shared<Term>(L"test", L"miss"))),
      0);
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testDMQHitOrEmpty() 
{
  shared_ptr<Query> query = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(
          make_shared<TermQuery>(make_shared<Term>(L"test", L"hit")),
          make_shared<TermQuery>(make_shared<Term>(L"empty", L"miss"))),
      0);
  shared_ptr<TopDocs> topDocs = indexSearcher->search(query, 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  TestUtil::assertEquals(1, topDocs->scoreDocs.size());
  assertTrue(topDocs->scoreDocs[0]->score != 0);
}

void TestClassicSimilarity::testSaneNormValues() 
{
  shared_ptr<ClassicSimilarity> sim = make_shared<ClassicSimilarity>();
  for (int i = 0; i < 256; i++) {
    float boost = TFIDFSimilarity::OLD_NORM_TABLE[i];
    assertFalse(L"negative boost: " + to_wstring(boost) + L", byte=" +
                    to_wstring(i),
                boost < 0.0f);
    assertFalse(L"inf bost: " + to_wstring(boost) + L", byte=" + to_wstring(i),
                isinf(boost));
    assertFalse(L"nan boost for byte=" + to_wstring(i), isnan(boost));
    if (i > 0) {
      assertTrue(L"boost is not increasing: " + to_wstring(boost) + L",byte=" +
                     to_wstring(i),
                 boost > TFIDFSimilarity::OLD_NORM_TABLE[i - 1]);
    }
  }

  shared_ptr<TFIDFSimilarity::IDFStats> stats =
      std::static_pointer_cast<IDFStats>(sim->computeWeight(
          1.0f, (make_shared<IndexSearcher>(make_shared<MultiReader>()))
                    ->collectionStatistics(L"foo")));
  for (int i = 0; i < 256; i++) {
    float boost = stats->normTable[i];
    assertFalse(L"negative boost: " + to_wstring(boost) + L", byte=" +
                    to_wstring(i),
                boost < 0.0f);
    assertFalse(L"inf bost: " + to_wstring(boost) + L", byte=" + to_wstring(i),
                isinf(boost));
    assertFalse(L"nan boost for byte=" + to_wstring(i), isnan(boost));
    if (i > 0) {
      assertTrue(L"boost is not decreasing: " + to_wstring(boost) + L",byte=" +
                     to_wstring(i),
                 boost < stats->normTable[i - 1]);
    }
  }
}

void TestClassicSimilarity::testNormEncodingBackwardCompatibility() throw(
    IOException)
{
  shared_ptr<Similarity> similarity = make_shared<ClassicSimilarity>();
  for (auto indexCreatedVersionMajor :
       std::deque<int>{Version::LUCENE_6_0_0->major, Version::LATEST->major}) {
    for (auto length : std::deque<int>{
             1, 4,
             16}) { // these length values are encoded accurately on both cases
      shared_ptr<Directory> dir = newDirectory();
      // set the version on the directory
      (make_shared<SegmentInfos>(indexCreatedVersionMajor))->commit(dir);
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, newIndexWriterConfig()->setSimilarity(similarity));
      shared_ptr<Document> doc = make_shared<Document>();
      wstring value = IntStream::range(0, length)
                          .mapToObj([&](any i) { L"b"; })
                          .collect(Collectors::joining(L" "));
      doc->push_back(make_shared<TextField>(L"foo", value, Store::NO));
      w->addDocument(doc);
      shared_ptr<IndexReader> reader = DirectoryReader::open(w);
      shared_ptr<IndexSearcher> searcher = newSearcher(reader);
      searcher->setSimilarity(similarity);
      shared_ptr<Explanation> expl = searcher->explain(
          make_shared<TermQuery>(make_shared<Term>(L"foo", L"b")), 0);
      shared_ptr<Explanation> fieldNorm = findExplanation(expl, L"fieldNorm");
      assertNotNull(fieldNorm);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(fieldNorm->toString(), 1 / sqrt(length),
                   fieldNorm->getValue(), 0.0f);
      delete w;
      delete reader;
      delete dir;
    }
  }
}

shared_ptr<Explanation>
TestClassicSimilarity::findExplanation(shared_ptr<Explanation> expl,
                                       const wstring &text)
{
  if (StringHelper::startsWith(expl->getDescription(), text)) {
    return expl;
  } else {
    for (auto sub : expl->getDetails()) {
      shared_ptr<Explanation> match = findExplanation(sub, text);
      if (match != nullptr) {
        return match;
      }
    }
  }
  return nullptr;
}

void TestClassicSimilarity::testSameNormsAsBM25()
{
  shared_ptr<ClassicSimilarity> sim1 = make_shared<ClassicSimilarity>();
  shared_ptr<BM25Similarity> sim2 = make_shared<BM25Similarity>();
  sim2->setDiscountOverlaps(true);
  for (int iter = 0; iter < 100; ++iter) {
    constexpr int length = TestUtil::nextInt(random(), 1, 1000);
    constexpr int position = random()->nextInt(length);
    constexpr int numOverlaps = random()->nextInt(length);
    shared_ptr<FieldInvertState> state = make_shared<FieldInvertState>(
        Version::LATEST->major, L"foo", position, length, numOverlaps, 100);
    assertEquals(sim2->computeNorm(state), sim1->computeNorm(state), 0.0f);
  }
}
} // namespace org::apache::lucene::search::similarities