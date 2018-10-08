using namespace std;

#include "TestSimilarity.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

float TestSimilarity::SimpleSimilarity::lengthNorm(int length) { return 1; }

float TestSimilarity::SimpleSimilarity::tf(float freq) { return freq; }

float TestSimilarity::SimpleSimilarity::sloppyFreq(int distance)
{
  return 2.0f;
}

float TestSimilarity::SimpleSimilarity::idf(int64_t docFreq,
                                            int64_t docCount)
{
  return 1.0f;
}

shared_ptr<Explanation> TestSimilarity::SimpleSimilarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    std::deque<std::shared_ptr<TermStatistics>> &stats)
{
  return Explanation::match(1.0f, L"Inexplicable");
}

void TestSimilarity::testSimilarity() 
{
  shared_ptr<Directory> store = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), store,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setSimilarity(make_shared<SimpleSimilarity>()));

  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"field", L"a c", Field::Store::YES));

  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"field", L"a b c", Field::Store::YES));

  writer->addDocument(d1);
  writer->addDocument(d2);
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<SimpleSimilarity>());

  shared_ptr<Term> a = make_shared<Term>(L"field", L"a");
  shared_ptr<Term> b = make_shared<Term>(L"field", L"b");
  shared_ptr<Term> c = make_shared<Term>(L"field", L"c");

  searcher->search(
      make_shared<TermQuery>(b),
      make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this()));

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(a), BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(b), BooleanClause::Occur::SHOULD);
  // System.out.println(bq.toString("field"));
  searcher->search(
      bq->build(),
      make_shared<SimpleCollectorAnonymousInnerClass2>(shared_from_this()));

  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(a->field(), a->bytes(), c->bytes());
  // System.out.println(pq.toString("field"));
  searcher->search(
      pq, make_shared<SimpleCollectorAnonymousInnerClass3>(shared_from_this()));

  pq = make_shared<PhraseQuery>(2, a->field(), a->bytes(), c->bytes());
  // System.out.println(pq.toString("field"));
  searcher->search(
      pq, make_shared<SimpleCollectorAnonymousInnerClass4>(shared_from_this()));

  delete reader;
  delete store;
}

TestSimilarity::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  assertEquals(1.0f, scorer::score(), 0);
}

bool TestSimilarity::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

TestSimilarity::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
  base = 0;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  // System.out.println("Doc=" + doc + " score=" + score);
  assertEquals(static_cast<float>(doc) + base + 1, scorer::score(), 0);
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  base = context->docBase;
}

bool TestSimilarity::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}

TestSimilarity::SimpleCollectorAnonymousInnerClass3::
    SimpleCollectorAnonymousInnerClass3(
        shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass3::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass3::collect(
    int doc) 
{
  // System.out.println("Doc=" + doc + " score=" + score);
  assertEquals(1.0f, scorer::score(), 0);
}

bool TestSimilarity::SimpleCollectorAnonymousInnerClass3::needsScores()
{
  return true;
}

TestSimilarity::SimpleCollectorAnonymousInnerClass4::
    SimpleCollectorAnonymousInnerClass4(
        shared_ptr<TestSimilarity> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass4::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestSimilarity::SimpleCollectorAnonymousInnerClass4::collect(
    int doc) 
{
  // System.out.println("Doc=" + doc + " score=" + score);
  assertEquals(2.0f, scorer::score(), 0);
}

bool TestSimilarity::SimpleCollectorAnonymousInnerClass4::needsScores()
{
  return true;
}
} // namespace org::apache::lucene::search