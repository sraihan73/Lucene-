using namespace std;

#include "TestFeatureField.h"

namespace org::apache::lucene::document
{
using Store = org::apache::lucene::document::Field::Store;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using Scorer = org::apache::lucene::search::Scorer;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Weight = org::apache::lucene::search::Weight;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

float TestFeatureField::round(float f)
{
  int bits = Float::floatToIntBits(f);
  bits &= ~0 << 15; // clear last 15 bits
  return Float::intBitsToFloat(bits);
}

void TestFeatureField::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setMergePolicy(
          newLogMergePolicy(random()->nextBoolean())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FeatureField> pagerank =
      make_shared<FeatureField>(L"features", L"pagerank", 1);
  shared_ptr<FeatureField> urlLength =
      make_shared<FeatureField>(L"features", L"urlLen", 1);
  doc->push_back(pagerank);
  doc->push_back(urlLength);

  pagerank->setFeatureValue(10);
  urlLength->setFeatureValue(1.0f / 24);
  writer->addDocument(doc);

  pagerank->setFeatureValue(100);
  urlLength->setFeatureValue(1.0f / 20);
  writer->addDocument(doc);

  writer->addDocument(make_shared<Document>()); // gap

  pagerank->setFeatureValue(1);
  urlLength->setFeatureValue(1.0f / 100);
  writer->addDocument(doc);

  pagerank->setFeatureValue(42);
  urlLength->setFeatureValue(1.0f / 23);
  writer->addDocument(doc);

  writer->forceMerge(1);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  shared_ptr<LeafReaderContext> context = reader->leaves()->get(0);

  shared_ptr<Query> q =
      FeatureField::newLogQuery(L"features", L"pagerank", 3.0f, 4.5f);
  shared_ptr<Weight> w = q->createWeight(searcher, true, 2);
  shared_ptr<Scorer> s = w->scorer(context);

  assertEquals(0, s->begin().nextDoc());
  assertEquals(static_cast<float>(6.0 * log(4.5f + 10)), s->score(), 0.0f);

  assertEquals(1, s->begin().nextDoc());
  assertEquals(static_cast<float>(6.0 * log(4.5f + 100)), s->score(), 0.0f);

  assertEquals(3, s->begin().nextDoc());
  assertEquals(static_cast<float>(6.0 * log(4.5f + 1)), s->score(), 0.0f);

  assertEquals(4, s->begin().nextDoc());
  assertEquals(static_cast<float>(6.0 * log(4.5f + 42)), s->score(), 0.0f);

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, s->begin().nextDoc());

  q = FeatureField::newSaturationQuery(L"features", L"pagerank", 3.0f, 4.5f);
  w = q->createWeight(searcher, true, 2);
  s = w->scorer(context);

  assertEquals(0, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 4.5f / (4.5f + 10)), s->score(), 0.0f);

  assertEquals(1, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 4.5f / (4.5f + 100)), s->score(), 0.0f);

  assertEquals(3, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 4.5f / (4.5f + 1)), s->score(), 0.0f);

  assertEquals(4, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 4.5f / (4.5f + 42)), s->score(), 0.0f);

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, s->begin().nextDoc());

  q = FeatureField::newSigmoidQuery(L"features", L"pagerank", 3.0f, 4.5f, 0.6f);
  w = q->createWeight(searcher, true, 2);
  s = w->scorer(context);
  double kPa = pow(4.5f, 0.6f);

  assertEquals(0, s->begin().nextDoc());
  assertEquals(static_cast<float>(6 * (1 - kPa / (kPa + pow(10, 0.6f)))),
               s->score(), 0.0f);

  assertEquals(1, s->begin().nextDoc());
  assertEquals(static_cast<float>(6 * (1 - kPa / (kPa + pow(100, 0.6f)))),
               s->score(), 0.0f);

  assertEquals(3, s->begin().nextDoc());
  assertEquals(static_cast<float>(6 * (1 - kPa / (kPa + pow(1, 0.6f)))),
               s->score(), 0.0f);

  assertEquals(4, s->begin().nextDoc());
  assertEquals(static_cast<float>(6 * (1 - kPa / (kPa + pow(42, 0.6f)))),
               s->score(), 0.0f);

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, s->begin().nextDoc());

  q = FeatureField::newSaturationQuery(L"features", L"urlLen", 3.0f, 1.0f / 24);
  w = q->createWeight(searcher, true, 2);
  s = w->scorer(context);

  assertEquals(0, s->begin().nextDoc());
  assertEquals(6.0f * (1 - (1.0f / 24) / (1.0f / 24 + round(1.0f / 24))),
               s->score(), 0.0f);

  assertEquals(1, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 1.0f / 24 / (1.0f / 24 + round(1.0f / 20))),
               s->score(), 0.0f);

  assertEquals(3, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 1.0f / 24 / (1.0f / 24 + round(1.0f / 100))),
               s->score(), 0.0f);

  assertEquals(4, s->begin().nextDoc());
  assertEquals(6.0f * (1 - 1.0f / 24 / (1.0f / 24 + round(1.0f / 23))),
               s->score(), 0.0f);

  assertEquals(DocIdSetIterator::NO_MORE_DOCS, s->begin().nextDoc());

  reader->close();
  delete dir;
}

void TestFeatureField::testExplanations() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setMergePolicy(
          newLogMergePolicy(random()->nextBoolean())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FeatureField> pagerank =
      make_shared<FeatureField>(L"features", L"pagerank", 1);
  doc->push_back(pagerank);

  pagerank->setFeatureValue(10);
  writer->addDocument(doc);

  pagerank->setFeatureValue(100);
  writer->addDocument(doc);

  writer->addDocument(make_shared<Document>()); // gap

  pagerank->setFeatureValue(1);
  writer->addDocument(doc);

  pagerank->setFeatureValue(42);
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

  QueryUtils::check(
      random(), FeatureField::newLogQuery(L"features", L"pagerank", 1.0f, 4.5f),
      searcher);
  QueryUtils::check(
      random(),
      FeatureField::newSaturationQuery(L"features", L"pagerank", 1.0f, 12.0f),
      searcher);
  QueryUtils::check(random(),
                    FeatureField::newSigmoidQuery(L"features", L"pagerank",
                                                  1.0f, 12.0f, 0.6f),
                    searcher);

  // Test boosts that are > 1
  QueryUtils::check(
      random(), FeatureField::newLogQuery(L"features", L"pagerank", 3.0f, 4.5f),
      searcher);
  QueryUtils::check(
      random(),
      FeatureField::newSaturationQuery(L"features", L"pagerank", 3.0f, 12.0f),
      searcher);
  QueryUtils::check(random(),
                    FeatureField::newSigmoidQuery(L"features", L"pagerank",
                                                  3.0f, 12.0f, 0.6f),
                    searcher);

  // Test boosts that are < 1
  QueryUtils::check(
      random(), FeatureField::newLogQuery(L"features", L"pagerank", .2f, 4.5f),
      searcher);
  QueryUtils::check(
      random(),
      FeatureField::newSaturationQuery(L"features", L"pagerank", .2f, 12.0f),
      searcher);
  QueryUtils::check(
      random(),
      FeatureField::newSigmoidQuery(L"features", L"pagerank", .2f, 12.0f, 0.6f),
      searcher);

  reader->close();
  delete dir;
}

void TestFeatureField::testLogSimScorer() 
{
  doTestSimScorer(
      (make_shared<FeatureField::LogFunction>(4.5f))->scorer(L"foo", 3.0f));
}

void TestFeatureField::testSatuSimScorer() 
{
  doTestSimScorer(
      (make_shared<FeatureField::SaturationFunction>(L"foo", L"bar", 20.0f))
          ->scorer(L"foo", 3.0f));
}

void TestFeatureField::testSigmSimScorer() 
{
  doTestSimScorer((make_shared<FeatureField::SigmoidFunction>(20.0f, 0.6f))
                      ->scorer(L"foo", 3.0f));
}

void TestFeatureField::doTestSimScorer(shared_ptr<SimScorer> s) throw(
    IOException)
{
  float maxScore = s->score(0, numeric_limits<float>::max());
  assertTrue(Float::isFinite(maxScore)); // used to compute max scores
  // Test that the score doesn't decrease with freq
  for (int freq = 2; freq < 65536; ++freq) {
    assertTrue(s->score(freq - 1, 1LL) <= s->score(freq, 1LL));
  }
  assertTrue(s->score(65535, 1LL) <= maxScore);
}

void TestFeatureField::testComputePivotFeatureValue() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, newIndexWriterConfig());

  // Make sure that we create a legal pivot on missing features
  shared_ptr<DirectoryReader> reader = writer->getReader();
  float pivot =
      FeatureField::computePivotFeatureValue(reader, L"features", L"pagerank");
  assertTrue(Float::isFinite(pivot));
  assertTrue(pivot > 0);
  reader->close();

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FeatureField> pagerank =
      make_shared<FeatureField>(L"features", L"pagerank", 1);
  doc->push_back(pagerank);

  pagerank->setFeatureValue(10);
  writer->addDocument(doc);

  pagerank->setFeatureValue(100);
  writer->addDocument(doc);

  writer->addDocument(make_shared<Document>()); // gap

  pagerank->setFeatureValue(1);
  writer->addDocument(doc);

  pagerank->setFeatureValue(42);
  writer->addDocument(doc);

  reader = writer->getReader();
  delete writer;

  pivot =
      FeatureField::computePivotFeatureValue(reader, L"features", L"pagerank");
  double expected = pow(10 * 100 * 1 * 42, 1 / 4.0); // geometric mean
  assertEquals(expected, pivot, 0.1);

  reader->close();
  delete dir;
}

void TestFeatureField::testExtractTerms() 
{
  shared_ptr<IndexReader> reader = make_shared<MultiReader>();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Query> query =
      FeatureField::newLogQuery(L"field", L"term", 2.0f, 42);

  shared_ptr<Weight> weight = searcher->createWeight(query, false, 1.0f);
  shared_ptr<Set<std::shared_ptr<Term>>> terms =
      unordered_set<std::shared_ptr<Term>>();
  weight->extractTerms(terms);
  assertEquals(Collections::emptySet(), terms);

  terms = unordered_set<>();
  weight = searcher->createWeight(query, true, 1.0f);
  weight->extractTerms(terms);
  assertEquals(Collections::singleton(make_shared<Term>(L"field", L"term")),
               terms);
}

void TestFeatureField::testDemo() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setMergePolicy(
          newLogMergePolicy(random()->nextBoolean())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FeatureField> pagerank =
      make_shared<FeatureField>(L"features", L"pagerank", 1);
  doc->push_back(pagerank);
  shared_ptr<TextField> body = make_shared<TextField>(L"body", L"", Store::NO);
  doc->push_back(body);

  pagerank->setFeatureValue(10);
  body->setStringValue(L"Apache Lucene");
  writer->addDocument(doc);

  pagerank->setFeatureValue(1000);
  body->setStringValue(L"Apache Web HTTP server");
  writer->addDocument(doc);

  pagerank->setFeatureValue(1);
  body->setStringValue(L"Lucene is a search engine");
  writer->addDocument(doc);

  pagerank->setFeatureValue(42);
  body->setStringValue(L"Lucene in the sky with diamonds");
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> reader = writer->getReader();
  delete writer;

  // NOTE: If you need to make changes below, then you likely also need to
  // update javadocs of FeatureField.

  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setSimilarity(make_shared<BM25Similarity>());
  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"apache")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"lucene")),
                Occur::SHOULD)
          ->build();
  shared_ptr<Query> boost =
      FeatureField::newSaturationQuery(L"features", L"pagerank");
  shared_ptr<Query> boostedQuery = (make_shared<BooleanQuery::Builder>())
                                       ->add(query, Occur::MUST)
                                       ->add(boost, Occur::SHOULD)
                                       ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(boostedQuery, 10);
  assertEquals(4, topDocs->scoreDocs.size());
  assertEquals(1, topDocs->scoreDocs[0]->doc);
  assertEquals(0, topDocs->scoreDocs[1]->doc);
  assertEquals(3, topDocs->scoreDocs[2]->doc);
  assertEquals(2, topDocs->scoreDocs[3]->doc);

  reader->close();
  delete dir;
}
} // namespace org::apache::lucene::document