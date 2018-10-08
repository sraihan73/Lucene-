using namespace std;

#include "TestConstantScoreQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestConstantScoreQuery::testCSQ() 
{
  shared_ptr<Query> *const q1 = make_shared<ConstantScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"a", L"b")));
  shared_ptr<Query> *const q2 = make_shared<ConstantScoreQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"a", L"c")));
  shared_ptr<Query> *const q3 = make_shared<ConstantScoreQuery>(
      TermRangeQuery::newStringRange(L"a", L"b", L"c", true, true));
  QueryUtils::check(q1);
  QueryUtils::check(q2);
  QueryUtils::checkEqual(q1, q1);
  QueryUtils::checkEqual(q2, q2);
  QueryUtils::checkEqual(q3, q3);
  QueryUtils::checkUnequal(q1, q2);
  QueryUtils::checkUnequal(q2, q3);
  QueryUtils::checkUnequal(q1, q3);
  QueryUtils::checkUnequal(
      q1, make_shared<TermQuery>(make_shared<Term>(L"a", L"b")));
}

void TestConstantScoreQuery::checkHits(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    float const expectedScore,
    type_info const innerScorerClass) 
{
  const std::deque<int> count = std::deque<int>(1);
  searcher->search(
      q, make_shared<SimpleCollectorAnonymousInnerClass>(
             shared_from_this(), expectedScore, innerScorerClass, count));
  assertEquals(L"invalid number of results", 1, count[0]);
}

TestConstantScoreQuery::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestConstantScoreQuery> outerInstance, float expectedScore,
        type_info innerScorerClass, deque<int> &count)
{
  this->outerInstance = outerInstance;
  this->expectedScore = expectedScore;
  this->innerScorerClass = innerScorerClass;
  this->count = count;
}

void TestConstantScoreQuery::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
  if (innerScorerClass != nullptr) {
    shared_ptr<FilterScorer> *const innerScorer =
        std::static_pointer_cast<FilterScorer>(scorer);
    assertEquals(L"inner Scorer is implemented by wrong class",
                 innerScorerClass, innerScorer->in_->getClass());
  }
}

void TestConstantScoreQuery::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  assertEquals(L"Score differs from expected", expectedScore,
               this->scorer.score(), 0);
  count[0]++;
}

bool TestConstantScoreQuery::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

void TestConstantScoreQuery::testWrapped2Times() 
{
  shared_ptr<Directory> directory = nullptr;
  shared_ptr<IndexReader> reader = nullptr;
  shared_ptr<IndexSearcher> searcher = nullptr;
  try {
    directory = newDirectory();
    shared_ptr<RandomIndexWriter> writer =
        make_shared<RandomIndexWriter>(random(), directory);

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"field", L"term1", Field::Store::NO));
    doc->push_back(newStringField(L"field", L"term2", Field::Store::NO));
    writer->addDocument(doc);

    reader = writer->getReader();
    delete writer;
    // we don't wrap with AssertingIndexSearcher in order to have the original
    // scorer in setScorer.
    searcher = newSearcher(reader, true, false);
    searcher->setQueryCache(nullptr); // to assert on scorer impl

    shared_ptr<BoostQuery> *const csq1 = make_shared<BoostQuery>(
        make_shared<ConstantScoreQuery>(
            make_shared<TermQuery>(make_shared<Term>(L"field", L"term1"))),
        2.0f);
    shared_ptr<BoostQuery> *const csq2 = make_shared<BoostQuery>(
        make_shared<ConstantScoreQuery>(make_shared<ConstantScoreQuery>(
            make_shared<TermQuery>(make_shared<Term>(L"field", L"term2")))),
        5.0f);

    shared_ptr<BooleanQuery::Builder> *const bq =
        make_shared<BooleanQuery::Builder>();
    bq->add(csq1, BooleanClause::Occur::SHOULD);
    bq->add(csq2, BooleanClause::Occur::SHOULD);

    shared_ptr<BoostQuery> *const csqbq = make_shared<BoostQuery>(
        make_shared<ConstantScoreQuery>(bq->build()), 17.0f);

    checkHits(searcher, csq1, csq1->getBoost(), TermScorer::typeid);
    checkHits(searcher, csq2, csq2->getBoost(), TermScorer::typeid);

    // for the combined BQ, the scorer should always be BooleanScorer's
    // BucketScorer, because our scorer supports out-of order collection!
    const type_info<std::shared_ptr<FakeScorer>> bucketScorerClass =
        FakeScorer::typeid;
    checkHits(searcher, csqbq, csqbq->getBoost(), bucketScorerClass);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader, directory});
  }
}

TestConstantScoreQuery::QueryWrapper::QueryWrapper(shared_ptr<Query> in_)
    : in_(in_)
{
}

wstring TestConstantScoreQuery::QueryWrapper::toString(const wstring &field)
{
  return L"MockQuery";
}

shared_ptr<Weight> TestConstantScoreQuery::QueryWrapper::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return in_->createWeight(searcher, needsScores, boost);
}

bool TestConstantScoreQuery::QueryWrapper::equals(any other)
{
  return sameClassAs(other) &&
         in_->equals((any_cast<std::shared_ptr<QueryWrapper>>(other)).in_);
}

int TestConstantScoreQuery::QueryWrapper::hashCode()
{
  return 31 * classHash() + in_->hashCode();
}

void TestConstantScoreQuery::testConstantScoreQueryAndFilter() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"a", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"b", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<Query> filterB = make_shared<QueryWrapper>(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"b")));
  shared_ptr<Query> query = make_shared<ConstantScoreQuery>(filterB);

  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<Query> filtered = (make_shared<BooleanQuery::Builder>())
                                   ->add(query, Occur::MUST)
                                   ->add(filterB, Occur::FILTER)
                                   ->build();
  assertEquals(
      1,
      s->search(filtered, 1)->totalHits); // Query for field:b, Filter field:b

  shared_ptr<Query> filterA = make_shared<QueryWrapper>(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"a")));
  query = make_shared<ConstantScoreQuery>(filterA);

  filtered = (make_shared<BooleanQuery::Builder>())
                 ->add(query, Occur::MUST)
                 ->add(filterB, Occur::FILTER)
                 ->build();
  assertEquals(
      0, s->search(filtered, 1)->totalHits); // Query field:b, Filter field:a

  delete r;
  delete d;
}

void TestConstantScoreQuery::testPropagatesApproximations() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<Query> q = searcher->rewrite(make_shared<ConstantScoreQuery>(pq));

  shared_ptr<Weight> *const weight = searcher->createWeight(q, true, 1);
  shared_ptr<Scorer> *const scorer =
      weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestConstantScoreQuery::testExtractTerms() 
{
  shared_ptr<IndexSearcher> *const searcher =
      newSearcher(make_shared<MultiReader>());
  shared_ptr<TermQuery> *const termQuery =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<Query> *const csq =
      searcher->rewrite(make_shared<ConstantScoreQuery>(termQuery));

  shared_ptr<Set<std::shared_ptr<Term>>> *const scoringTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(csq, true, 1)->extractTerms(scoringTerms);
  assertEquals(Collections::emptySet(), scoringTerms);

  shared_ptr<Set<std::shared_ptr<Term>>> *const matchingTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(csq, false, 1)->extractTerms(matchingTerms);
  assertEquals(Collections::singleton(make_shared<Term>(L"foo", L"bar")),
               matchingTerms);
}
} // namespace org::apache::lucene::search