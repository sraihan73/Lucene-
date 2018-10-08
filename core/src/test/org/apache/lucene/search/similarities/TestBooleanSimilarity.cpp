using namespace std;

#include "TestBooleanSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestBooleanSimilarity::testTermScoreIsEqualToBoost() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  doc->push_back(make_shared<StringField>(L"foo", L"baz", Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Store::NO));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> reader = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<BooleanSimilarity>());
  shared_ptr<TopDocs> topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2);
  TestUtil::assertEquals(2, topDocs->totalHits);
  assertEquals(1.0f, topDocs->scoreDocs[0]->score, 0.0f);
  assertEquals(1.0f, topDocs->scoreDocs[1]->score, 0.0f);

  topDocs = searcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")), 1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  assertEquals(1.0f, topDocs->scoreDocs[0]->score, 0.0f);

  topDocs = searcher->search(
      make_shared<BoostQuery>(
          make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")), 3.0f),
      1);
  TestUtil::assertEquals(1, topDocs->totalHits);
  assertEquals(3.0f, topDocs->scoreDocs[0]->score, 0.0f);

  reader->close();
  delete dir;
}

void TestBooleanSimilarity::testPhraseScoreIsEqualToBoost() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setSimilarity(make_shared<BooleanSimilarity>()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"foo", L"bar baz quux", Store::NO));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> reader = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<BooleanSimilarity>());

  shared_ptr<PhraseQuery> query =
      make_shared<PhraseQuery>(2, L"foo", L"bar", L"quux");

  shared_ptr<TopDocs> topDocs = searcher->search(query, 2);
  TestUtil::assertEquals(1, topDocs->totalHits);
  assertEquals(1.0f, topDocs->scoreDocs[0]->score, 0.0f);

  topDocs = searcher->search(make_shared<BoostQuery>(query, 7), 2);
  TestUtil::assertEquals(1, topDocs->totalHits);
  assertEquals(7.0f, topDocs->scoreDocs[0]->score, 0.0f);

  reader->close();
  delete dir;
}

void TestBooleanSimilarity::testSameNormsAsBM25()
{
  shared_ptr<BooleanSimilarity> sim1 = make_shared<BooleanSimilarity>();
  shared_ptr<BM25Similarity> sim2 = make_shared<BM25Similarity>();
  sim2->setDiscountOverlaps(true);
  for (int iter = 0; iter < 100; ++iter) {
    constexpr int length = TestUtil::nextInt(random(), 1, 100);
    constexpr int position = random()->nextInt(length);
    constexpr int numOverlaps = random()->nextInt(length);
    shared_ptr<FieldInvertState> state = make_shared<FieldInvertState>(
        Version::LATEST->major, L"foo", position, length, numOverlaps, 100);
    assertEquals(sim2->computeNorm(state), sim1->computeNorm(state), 0.0f);
  }
}
} // namespace org::apache::lucene::search::similarities