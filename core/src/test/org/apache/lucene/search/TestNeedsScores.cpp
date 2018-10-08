using namespace std;

#include "TestNeedsScores.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNeedsScores::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 5; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<TextField>(
        L"field", L"this is document " + to_wstring(i), Field::Store::NO));
    iw->addDocument(doc);
  }
  reader = iw->getReader();
  searcher = newSearcher(reader);
  delete iw;
}

void TestNeedsScores::tearDown() 
{
  IOUtils::close({reader, dir});
  LuceneTestCase::tearDown();
}

void TestNeedsScores::testProhibitedClause() 
{
  shared_ptr<Query> required =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"this"));
  shared_ptr<Query> prohibited =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"3"));
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<AssertNeedsScores>(required, true),
          BooleanClause::Occur::MUST);
  bq->add(make_shared<AssertNeedsScores>(prohibited, false),
          BooleanClause::Occur::MUST_NOT);
  assertEquals(4, searcher->search(bq->build(), 5)->totalHits); // we exclude 3
}

void TestNeedsScores::testConstantScoreQuery() 
{
  shared_ptr<Query> term =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"this"));
  shared_ptr<Query> constantScore = make_shared<ConstantScoreQuery>(
      make_shared<AssertNeedsScores>(term, false));
  assertEquals(5, searcher->search(constantScore, 5)->totalHits);
}

void TestNeedsScores::testSortByField() 
{
  shared_ptr<Query> query =
      make_shared<AssertNeedsScores>(make_shared<MatchAllDocsQuery>(), false);
  assertEquals(5, searcher->search(query, 5, Sort::INDEXORDER)->totalHits);
}

void TestNeedsScores::testSortByScore() 
{
  shared_ptr<Query> query =
      make_shared<AssertNeedsScores>(make_shared<MatchAllDocsQuery>(), true);
  assertEquals(5, searcher->search(query, 5, Sort::RELEVANCE)->totalHits);
}

TestNeedsScores::AssertNeedsScores::AssertNeedsScores(shared_ptr<Query> in_,
                                                      bool value)
    : in_(Objects::requireNonNull(in_)), value(value)
{
}

shared_ptr<Weight> TestNeedsScores::AssertNeedsScores::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  shared_ptr<Weight> *const w = in_->createWeight(searcher, needsScores, boost);
  return make_shared<FilterWeightAnonymousInnerClass>(shared_from_this(),
                                                      needsScores, w);
}

TestNeedsScores::AssertNeedsScores::FilterWeightAnonymousInnerClass::
    FilterWeightAnonymousInnerClass(
        shared_ptr<AssertNeedsScores> outerInstance, bool needsScores,
        shared_ptr<org::apache::lucene::search::Weight> w)
    : FilterWeight(w)
{
  this->outerInstance = outerInstance;
  this->needsScores = needsScores;
  this->w = w;
}

shared_ptr<Scorer>
TestNeedsScores::AssertNeedsScores::FilterWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  assertEquals(L"query=" + outerInstance->in_, outerInstance->value,
               needsScores);
  return w->scorer(context);
}

shared_ptr<Query> TestNeedsScores::AssertNeedsScores::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> in2 = in_->rewrite(reader);
  if (in2 == in_) {
    return Query::rewrite(reader);
  } else {
    return make_shared<AssertNeedsScores>(in2, value);
  }
}

int TestNeedsScores::AssertNeedsScores::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + in_->hashCode();
  result = prime * result + (value ? 1231 : 1237);
  return result;
}

bool TestNeedsScores::AssertNeedsScores::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TestNeedsScores::AssertNeedsScores::equalsTo(
    shared_ptr<AssertNeedsScores> other)
{
  return in_->equals(other->in_) && value == other->value;
}

wstring TestNeedsScores::AssertNeedsScores::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"asserting(" + in_->toString(field) + L")";
}
} // namespace org::apache::lucene::search