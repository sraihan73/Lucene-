using namespace std;

#include "TestTopFieldCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Entry = org::apache::lucene::search::FieldValueHitQueue::Entry;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestTopFieldCollector::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    iw->addDocument(doc);
  }
  ir = iw->getReader();
  delete iw;
  is = newSearcher(ir);
}

void TestTopFieldCollector::tearDown() 
{
  delete ir;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestTopFieldCollector::testSortWithoutFillFields() 
{

  // There was previously a bug in TopFieldCollector when fillFields was set
  // to false - the same doc and score was set in ScoreDoc[] array. This test
  // asserts that if fillFields is false, the documents are set properly. It
  // does not use Searcher's default search methods (with Sort) since all set
  // fillFields to true.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC), make_shared<Sort>()};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, false, false, false, true);

    is->search(q, tdc);

    std::deque<std::shared_ptr<ScoreDoc>> sd = tdc->topDocs()->scoreDocs;
    for (int j = 1; j < sd.size(); j++) {
      assertTrue(sd[j]->doc != sd[j - 1]->doc);
    }
  }
}

void TestTopFieldCollector::testSortWithoutScoreTracking() 
{

  // Two Sort criteria to instantiate the multi/single comparators.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC), make_shared<Sort>()};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, true, false, false, true);

    is->search(q, tdc);

    shared_ptr<TopDocs> td = tdc->topDocs();
    std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
    for (int j = 0; j < sd.size(); j++) {
      assertTrue(isnan(sd[j]->score));
    }
    assertTrue(isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::testSortWithoutTotalHitTracking() throw(
    runtime_error)
{
  shared_ptr<Sort> sort = make_shared<Sort>(SortField::FIELD_DOC);
  for (int i = 0; i < 2; i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    // check that setting trackTotalHits to false does not throw an NPE because
    // the index is not sorted
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc;
    if (i % 2 == 0) {
      tdc = TopFieldCollector::create(sort, 10, true, false, false, false);
    } else {
      shared_ptr<FieldDoc> fieldDoc =
          make_shared<FieldDoc>(1, NAN, std::deque<any>{1});
      tdc = TopFieldCollector::create(sort, 10, fieldDoc, true, false, false,
                                      false);
    }

    is->search(q, tdc);

    shared_ptr<TopDocs> td = tdc->topDocs();
    std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
    for (int j = 0; j < sd.size(); j++) {
      assertTrue(isnan(sd[j]->score));
    }
    assertTrue(isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::testSortWithScoreNoMaxScoreTracking() throw(
    runtime_error)
{

  // Two Sort criteria to instantiate the multi/single comparators.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC), make_shared<Sort>()};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, true, true, false, true);

    is->search(q, tdc);

    shared_ptr<TopDocs> td = tdc->topDocs();
    std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
    for (int j = 0; j < sd.size(); j++) {
      assertTrue(!isnan(sd[j]->score));
    }
    assertTrue(isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::testSortWithScoreNoMaxScoreTrackingMulti() throw(
    runtime_error)
{

  // Two Sort criteria to instantiate the multi/single comparators.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC, SortField::FIELD_SCORE)};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, true, true, false, true);

    is->search(q, tdc);

    shared_ptr<TopDocs> td = tdc->topDocs();
    std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
    for (int j = 0; j < sd.size(); j++) {
      assertTrue(!isnan(sd[j]->score));
    }
    assertTrue(isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::testSortWithScoreAndMaxScoreTracking() throw(
    runtime_error)
{

  // Two Sort criteria to instantiate the multi/single comparators.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC), make_shared<Sort>()};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<Query> q = make_shared<MatchAllDocsQuery>();
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, true, true, true, true);

    is->search(q, tdc);

    shared_ptr<TopDocs> td = tdc->topDocs();
    std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
    for (int j = 0; j < sd.size(); j++) {
      assertTrue(!isnan(sd[j]->score));
    }
    assertTrue(!isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::
    testSortWithScoreAndMaxScoreTrackingNoResults() 
{

  // Two Sort criteria to instantiate the multi/single comparators.
  std::deque<std::shared_ptr<Sort>> sort = {
      make_shared<Sort>(SortField::FIELD_DOC), make_shared<Sort>()};
  for (int i = 0; i < sort.size(); i++) {
    shared_ptr<TopDocsCollector<std::shared_ptr<Entry>>> tdc =
        TopFieldCollector::create(sort[i], 10, true, true, true, true);
    shared_ptr<TopDocs> td = tdc->topDocs();
    TestUtil::assertEquals(0, td->totalHits);
    assertTrue(isnan(td->getMaxScore()));
  }
}

void TestTopFieldCollector::testComputeScoresOnlyOnce() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<StringField> text =
      make_shared<StringField>(L"text", L"foo", Store::NO);
  doc->push_back(text);
  shared_ptr<NumericDocValuesField> relevance =
      make_shared<NumericDocValuesField>(L"relevance", 1);
  doc->push_back(relevance);
  w->addDocument(doc);
  text->setStringValue(L"bar");
  w->addDocument(doc);
  text->setStringValue(L"baz");
  w->addDocument(doc);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<Query> foo =
      make_shared<TermQuery>(make_shared<Term>(L"text", L"foo"));
  shared_ptr<Query> bar =
      make_shared<TermQuery>(make_shared<Term>(L"text", L"bar"));
  foo = make_shared<BoostQuery>(foo, 2);
  shared_ptr<Query> baz =
      make_shared<TermQuery>(make_shared<Term>(L"text", L"baz"));
  baz = make_shared<BoostQuery>(baz, 3);
  shared_ptr<Query> query = (make_shared<BooleanQuery::Builder>())
                                ->add(foo, Occur::SHOULD)
                                ->add(bar, Occur::SHOULD)
                                ->add(baz, Occur::SHOULD)
                                ->build();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  for (auto sort : std::deque<std::shared_ptr<Sort>>{
           make_shared<Sort>(SortField::FIELD_SCORE),
           make_shared<Sort>(
               make_shared<SortField>(L"f", SortField::Type::SCORE))}) {
    for (auto doDocScores : std::deque<bool>{false, true}) {
      for (auto doMaxScore : std::deque<bool>{false, true}) {
        shared_ptr<TopFieldCollector> *const topCollector =
            TopFieldCollector::create(sort, TestUtil::nextInt(random(), 1, 2),
                                      true, doDocScores, doMaxScore, true);
        shared_ptr<Collector> *const assertingCollector =
            make_shared<CollectorAnonymousInnerClass>(shared_from_this(),
                                                      topCollector);
        searcher->search(query, assertingCollector);
      }
    }
  }
  delete reader;
  delete w;
  delete dir;
}

TestTopFieldCollector::CollectorAnonymousInnerClass::
    CollectorAnonymousInnerClass(
        shared_ptr<TestTopFieldCollector> outerInstance,
        shared_ptr<org::apache::lucene::search::TopFieldCollector> topCollector)
{
  this->outerInstance = outerInstance;
  this->topCollector = topCollector;
}

shared_ptr<LeafCollector>
TestTopFieldCollector::CollectorAnonymousInnerClass::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafCollector> *const in_ =
      topCollector->getLeafCollector(context);
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(
      shared_from_this());
}

TestTopFieldCollector::CollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass::
        FilterLeafCollectorAnonymousInnerClass(
            shared_ptr<CollectorAnonymousInnerClass> outerInstance)
    : FilterLeafCollector(in_)
{
  this->outerInstance = outerInstance;
}

void TestTopFieldCollector::CollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  shared_ptr<Scorer> s =
      make_shared<ScorerAnonymousInnerClass>(shared_from_this(), scorer);
  outerInstance->outerInstance->super.setScorer(s);
}

TestTopFieldCollector::CollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass::ScorerAnonymousInnerClass::
        ScorerAnonymousInnerClass(
            shared_ptr<FilterLeafCollectorAnonymousInnerClass> outerInstance,
            shared_ptr<org::apache::lucene::search::Scorer> scorer)
    : Scorer(nullptr)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
  lastComputedDoc = -1;
}

float TestTopFieldCollector::CollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass::ScorerAnonymousInnerClass::
        score() 
{
  if (lastComputedDoc == docID()) {
    throw make_shared<AssertionError>(L"Score computed twice on " + docID());
  }
  lastComputedDoc = docID();
  return scorer->score();
}

int TestTopFieldCollector::CollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass::ScorerAnonymousInnerClass::docID()
{
  return scorer->docID();
}

shared_ptr<DocIdSetIterator> TestTopFieldCollector::
    CollectorAnonymousInnerClass::FilterLeafCollectorAnonymousInnerClass::
        ScorerAnonymousInnerClass::iterator()
{
  return scorer->begin();
}

bool TestTopFieldCollector::CollectorAnonymousInnerClass::needsScores()
{
  return topCollector->needsScores();
}
} // namespace org::apache::lucene::search