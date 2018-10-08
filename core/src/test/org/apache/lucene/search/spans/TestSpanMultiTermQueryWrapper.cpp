using namespace std;

#include "TestSpanMultiTermQueryWrapper.h"

namespace org::apache::lucene::search::spans
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using RegexpQuery = org::apache::lucene::search::RegexpQuery;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

void TestSpanMultiTermQueryWrapper::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), directory);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newTextField(L"field", L"", Field::Store::NO);
  doc->push_back(field);

  field->setStringValue(L"quick brown fox");
  iw->addDocument(doc);
  field->setStringValue(L"jumps over lazy broun dog");
  iw->addDocument(doc);
  field->setStringValue(L"jumps over extremely very lazy broxn dog");
  iw->addDocument(doc);
  reader = iw->getReader();
  delete iw;
  searcher = newSearcher(reader);
}

void TestSpanMultiTermQueryWrapper::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestSpanMultiTermQueryWrapper::testWildcard() 
{
  shared_ptr<WildcardQuery> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"bro?n"));
  shared_ptr<SpanQuery> swq = make_shared<SpanMultiTermQueryWrapper<>>(wq);
  // will only match quick brown fox
  shared_ptr<SpanFirstQuery> sfq = make_shared<SpanFirstQuery>(swq, 2);
  assertEquals(1, searcher->search(sfq, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testPrefix() 
{
  shared_ptr<WildcardQuery> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"extrem*"));
  shared_ptr<SpanQuery> swq = make_shared<SpanMultiTermQueryWrapper<>>(wq);
  // will only match "jumps over extremely very lazy broxn dog"
  shared_ptr<SpanFirstQuery> sfq = make_shared<SpanFirstQuery>(swq, 3);
  assertEquals(1, searcher->search(sfq, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testFuzzy() 
{
  shared_ptr<FuzzyQuery> fq =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"broan"));
  shared_ptr<SpanQuery> sfq = make_shared<SpanMultiTermQueryWrapper<>>(fq);
  // will not match quick brown fox
  shared_ptr<SpanPositionRangeQuery> sprq =
      make_shared<SpanPositionRangeQuery>(sfq, 3, 6);
  assertEquals(2, searcher->search(sprq, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testFuzzy2() 
{
  // maximum of 1 term expansion
  shared_ptr<FuzzyQuery> fq = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"broan"), 1, 0, 1, false);
  shared_ptr<SpanQuery> sfq = make_shared<SpanMultiTermQueryWrapper<>>(fq);
  // will only match jumps over lazy broun dog
  shared_ptr<SpanPositionRangeQuery> sprq =
      make_shared<SpanPositionRangeQuery>(sfq, 0, 100);
  assertEquals(1, searcher->search(sprq, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testNoSuchMultiTermsInNear() throw(
    runtime_error)
{
  // test to make sure non existent multiterms aren't throwing null pointer
  // exceptions
  shared_ptr<FuzzyQuery> fuzzyNoSuch = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"noSuch"), 1, 0, 1, false);
  shared_ptr<SpanQuery> spanNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(fuzzyNoSuch);
  shared_ptr<SpanQuery> term =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"brown"));
  shared_ptr<SpanQuery> near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanNoSuch}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);
  // flip order
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanNoSuch, term}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);

  shared_ptr<WildcardQuery> wcNoSuch =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"noSuch*"));
  shared_ptr<SpanQuery> spanWCNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(wcNoSuch);
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanWCNoSuch}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);

  shared_ptr<RegexpQuery> rgxNoSuch =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanRgxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(rgxNoSuch);
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanRgxNoSuch}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);

  shared_ptr<PrefixQuery> prfxNoSuch =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanPrfxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(prfxNoSuch);
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanPrfxNoSuch}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);

  // test single noSuch
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanPrfxNoSuch}, 1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);

  // test double noSuch
  near = make_shared<SpanNearQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanPrfxNoSuch, spanPrfxNoSuch},
      1, true);
  assertEquals(0, searcher->search(near, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testNoSuchMultiTermsInNotNear() throw(
    runtime_error)
{
  // test to make sure non existent multiterms aren't throwing non-matching
  // field exceptions
  shared_ptr<FuzzyQuery> fuzzyNoSuch = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"noSuch"), 1, 0, 1, false);
  shared_ptr<SpanQuery> spanNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(fuzzyNoSuch);
  shared_ptr<SpanQuery> term =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"brown"));
  shared_ptr<SpanNotQuery> notNear =
      make_shared<SpanNotQuery>(term, spanNoSuch, 0, 0);
  assertEquals(1, searcher->search(notNear, 10)->totalHits);

  // flip
  notNear = make_shared<SpanNotQuery>(spanNoSuch, term, 0, 0);
  assertEquals(0, searcher->search(notNear, 10)->totalHits);

  // both noSuch
  notNear = make_shared<SpanNotQuery>(spanNoSuch, spanNoSuch, 0, 0);
  assertEquals(0, searcher->search(notNear, 10)->totalHits);

  shared_ptr<WildcardQuery> wcNoSuch =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"noSuch*"));
  shared_ptr<SpanQuery> spanWCNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(wcNoSuch);
  notNear = make_shared<SpanNotQuery>(term, spanWCNoSuch, 0, 0);
  assertEquals(1, searcher->search(notNear, 10)->totalHits);

  shared_ptr<RegexpQuery> rgxNoSuch =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanRgxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(rgxNoSuch);
  notNear = make_shared<SpanNotQuery>(term, spanRgxNoSuch, 1, 1);
  assertEquals(1, searcher->search(notNear, 10)->totalHits);

  shared_ptr<PrefixQuery> prfxNoSuch =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanPrfxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(prfxNoSuch);
  notNear = make_shared<SpanNotQuery>(term, spanPrfxNoSuch, 1, 1);
  assertEquals(1, searcher->search(notNear, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testNoSuchMultiTermsInOr() throw(
    runtime_error)
{
  // test to make sure non existent multiterms aren't throwing null pointer
  // exceptions
  shared_ptr<FuzzyQuery> fuzzyNoSuch = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"noSuch"), 1, 0, 1, false);
  shared_ptr<SpanQuery> spanNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(fuzzyNoSuch);
  shared_ptr<SpanQuery> term =
      make_shared<SpanTermQuery>(make_shared<Term>(L"field", L"brown"));
  shared_ptr<SpanOrQuery> near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanNoSuch});
  assertEquals(1, searcher->search(near, 10)->totalHits);

  // flip
  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanNoSuch, term});
  assertEquals(1, searcher->search(near, 10)->totalHits);

  shared_ptr<WildcardQuery> wcNoSuch =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"noSuch*"));
  shared_ptr<SpanQuery> spanWCNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(wcNoSuch);
  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanWCNoSuch});
  assertEquals(1, searcher->search(near, 10)->totalHits);

  shared_ptr<RegexpQuery> rgxNoSuch =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanRgxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(rgxNoSuch);
  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanRgxNoSuch});
  assertEquals(1, searcher->search(near, 10)->totalHits);

  shared_ptr<PrefixQuery> prfxNoSuch =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanPrfxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(prfxNoSuch);
  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{term, spanPrfxNoSuch});
  assertEquals(1, searcher->search(near, 10)->totalHits);

  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanPrfxNoSuch});
  assertEquals(0, searcher->search(near, 10)->totalHits);

  near = make_shared<SpanOrQuery>(
      std::deque<std::shared_ptr<SpanQuery>>{spanPrfxNoSuch, spanPrfxNoSuch});
  assertEquals(0, searcher->search(near, 10)->totalHits);
}

void TestSpanMultiTermQueryWrapper::testNoSuchMultiTermsInSpanFirst() throw(
    runtime_error)
{
  // this hasn't been a problem
  shared_ptr<FuzzyQuery> fuzzyNoSuch = make_shared<FuzzyQuery>(
      make_shared<Term>(L"field", L"noSuch"), 1, 0, 1, false);
  shared_ptr<SpanQuery> spanNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(fuzzyNoSuch);
  shared_ptr<SpanQuery> spanFirst = make_shared<SpanFirstQuery>(spanNoSuch, 10);

  assertEquals(0, searcher->search(spanFirst, 10)->totalHits);

  shared_ptr<WildcardQuery> wcNoSuch =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"noSuch*"));
  shared_ptr<SpanQuery> spanWCNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(wcNoSuch);
  spanFirst = make_shared<SpanFirstQuery>(spanWCNoSuch, 10);
  assertEquals(0, searcher->search(spanFirst, 10)->totalHits);

  shared_ptr<RegexpQuery> rgxNoSuch =
      make_shared<RegexpQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanRgxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(rgxNoSuch);
  spanFirst = make_shared<SpanFirstQuery>(spanRgxNoSuch, 10);
  assertEquals(0, searcher->search(spanFirst, 10)->totalHits);

  shared_ptr<PrefixQuery> prfxNoSuch =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"noSuch"));
  shared_ptr<SpanQuery> spanPrfxNoSuch =
      make_shared<SpanMultiTermQueryWrapper<>>(prfxNoSuch);
  spanFirst = make_shared<SpanFirstQuery>(spanPrfxNoSuch, 10);
  assertEquals(0, searcher->search(spanFirst, 10)->totalHits);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWrappedQueryIsNotModified()
void TestSpanMultiTermQueryWrapper::testWrappedQueryIsNotModified()
{
  shared_ptr<PrefixQuery> *const pq =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"test"));
  int pqHash = pq->hashCode();
  shared_ptr<SpanMultiTermQueryWrapper<std::shared_ptr<PrefixQuery>>> wrapper =
      make_shared<SpanMultiTermQueryWrapper<std::shared_ptr<PrefixQuery>>>(pq);
  assertEquals(pqHash, pq->hashCode());
  wrapper->setRewriteMethod(
      make_shared<SpanRewriteMethodAnonymousInnerClass>(shared_from_this()));
  assertEquals(pqHash, pq->hashCode());
}

TestSpanMultiTermQueryWrapper::SpanRewriteMethodAnonymousInnerClass::
    SpanRewriteMethodAnonymousInnerClass(
        shared_ptr<TestSpanMultiTermQueryWrapper> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<SpanQuery>
TestSpanMultiTermQueryWrapper::SpanRewriteMethodAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader,
    shared_ptr<MultiTermQuery> query) 
{
  return nullptr;
}
} // namespace org::apache::lucene::search::spans