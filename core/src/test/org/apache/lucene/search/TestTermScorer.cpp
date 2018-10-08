using namespace std;

#include "TestTermScorer.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestTermScorer::FIELD = L"field";

void TestTermScorer::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();

  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy())
          ->setSimilarity(make_shared<ClassicSimilarity>()));
  for (int i = 0; i < values.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(FIELD, values[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  indexReader = getOnlyLeafReader(writer->getReader());
  delete writer;
  indexSearcher = newSearcher(indexReader, false);
  indexSearcher->setSimilarity(make_shared<ClassicSimilarity>());
}

void TestTermScorer::tearDown() 
{
  delete indexReader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestTermScorer::test() 
{

  shared_ptr<Term> allTerm = make_shared<Term>(FIELD, L"all");
  shared_ptr<TermQuery> termQuery = make_shared<TermQuery>(allTerm);

  shared_ptr<Weight> weight = indexSearcher->createWeight(termQuery, true, 1);
  assertTrue(std::dynamic_pointer_cast<LeafReaderContext>(
                 indexSearcher->getTopReaderContext()) != nullptr);
  shared_ptr<LeafReaderContext> context =
      std::static_pointer_cast<LeafReaderContext>(
          indexSearcher->getTopReaderContext());
  shared_ptr<BulkScorer> ts = weight->bulkScorer(context);
  // we have 2 documents with the term all in them, one document for all the
  // other values
  const deque<std::shared_ptr<TestHit>> docs =
      deque<std::shared_ptr<TestHit>>();
  // must call next first

  ts->score(make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this(),
                                                            context, docs),
            nullptr);
  assertTrue(L"docs Size: " + docs.size() + L" is not: " + to_wstring(2),
             docs.size() == 2);
  shared_ptr<TestHit> doc0 = docs[0];
  shared_ptr<TestHit> doc5 = docs[1];
  // The scores should be the same
  assertTrue(to_wstring(doc0->score) + L" does not equal: " +
                 to_wstring(doc5->score),
             doc0->score == doc5->score);
}

TestTermScorer::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<TestTermScorer> outerInstance,
                                       shared_ptr<LeafReaderContext> context,
                                       deque<std::shared_ptr<TestHit>> &docs)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->docs = docs;
  base = 0;
}

void TestTermScorer::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestTermScorer::SimpleCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  float score = scorer::score();
  doc = doc + base;
  docs.push_back(make_shared<TestHit>(doc, score));
  assertTrue(L"score " + to_wstring(score) + L" is not greater than 0",
             score > 0);
  assertTrue(L"Doc: " + to_wstring(doc) +
                 L" does not equal 0 or doc does not equal 5",
             doc == 0 || doc == 5);
}

void TestTermScorer::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  base = context->docBase;
}

bool TestTermScorer::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

void TestTermScorer::testNext() 
{

  shared_ptr<Term> allTerm = make_shared<Term>(FIELD, L"all");
  shared_ptr<TermQuery> termQuery = make_shared<TermQuery>(allTerm);

  shared_ptr<Weight> weight = indexSearcher->createWeight(termQuery, true, 1);
  assertTrue(std::dynamic_pointer_cast<LeafReaderContext>(
                 indexSearcher->getTopReaderContext()) != nullptr);
  shared_ptr<LeafReaderContext> context =
      std::static_pointer_cast<LeafReaderContext>(
          indexSearcher->getTopReaderContext());
  shared_ptr<Scorer> ts = weight->scorer(context);
  assertTrue(L"next did not return a doc",
             ts->begin().nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(L"next did not return a doc",
             ts->begin().nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(L"next returned a doc and it should not have",
             ts->begin().nextDoc() == DocIdSetIterator::NO_MORE_DOCS);
}

void TestTermScorer::testAdvance() 
{

  shared_ptr<Term> allTerm = make_shared<Term>(FIELD, L"all");
  shared_ptr<TermQuery> termQuery = make_shared<TermQuery>(allTerm);

  shared_ptr<Weight> weight = indexSearcher->createWeight(termQuery, true, 1);
  assertTrue(std::dynamic_pointer_cast<LeafReaderContext>(
                 indexSearcher->getTopReaderContext()) != nullptr);
  shared_ptr<LeafReaderContext> context =
      std::static_pointer_cast<LeafReaderContext>(
          indexSearcher->getTopReaderContext());
  shared_ptr<Scorer> ts = weight->scorer(context);
  assertTrue(L"Didn't skip",
             ts->begin().advance(3) != DocIdSetIterator::NO_MORE_DOCS);
  // The next doc should be doc 5
  assertTrue(L"doc should be number 5", ts->docID() == 5);
}

TestTermScorer::TestHit::TestHit(int doc, float score)
{
  this->doc = doc;
  this->score = score;
}

wstring TestTermScorer::TestHit::toString()
{
  return wstring(L"TestHit{") + L"doc=" + to_wstring(doc) + L", score=" +
         to_wstring(score) + L"}";
}

void TestTermScorer::testDoesNotLoadNorms() 
{
  shared_ptr<Term> allTerm = make_shared<Term>(FIELD, L"all");
  shared_ptr<TermQuery> termQuery = make_shared<TermQuery>(allTerm);

  shared_ptr<LeafReader> forbiddenNorms =
      make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(),
                                                       indexReader);
  // We don't use newSearcher because it sometimes runs checkIndex which loads
  // norms
  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(forbiddenNorms);

  shared_ptr<Weight> weight = indexSearcher->createWeight(termQuery, true, 1);
  expectThrows(AssertionError::typeid, [&]() {
    weight->scorer(forbiddenNorms->getContext())->begin().nextDoc();
  });

  shared_ptr<Weight> weight2 = indexSearcher->createWeight(termQuery, false, 1);
  // should not fail this time since norms are not necessary
  weight2->scorer(forbiddenNorms->getContext())->begin().nextDoc();
}

TestTermScorer::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<TestTermScorer> outerInstance,
        shared_ptr<LeafReader> indexReader)
    : org::apache::lucene::index::FilterLeafReader(indexReader)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
TestTermScorer::FilterLeafReaderAnonymousInnerClass::getNormValues(
    const wstring &field) 
{
  fail(L"Norms should not be loaded");
  // unreachable
  return nullptr;
}

shared_ptr<CacheHelper>
TestTermScorer::FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return in_::getCoreCacheHelper();
}

shared_ptr<CacheHelper>
TestTermScorer::FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return in_::getReaderCacheHelper();
}
} // namespace org::apache::lucene::search