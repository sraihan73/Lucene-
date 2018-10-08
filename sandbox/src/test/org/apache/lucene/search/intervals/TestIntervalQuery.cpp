using namespace std;

#include "TestIntervalQuery.h"

namespace org::apache::lucene::search::intervals
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using CheckHits = org::apache::lucene::search::CheckHits;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Ignore;
const wstring TestIntervalQuery::field = L"field";

void TestIntervalQuery::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  for (int i = 0; i < docFields.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(field, docFields[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
}

void TestIntervalQuery::tearDown() 
{
  delete reader;
  delete directory;
  LuceneTestCase::tearDown();
}

void TestIntervalQuery::checkHits(shared_ptr<Query> query,
                                  std::deque<int> &results) 
{
  CheckHits::checkHits(random(), query, field, searcher, results);
}

void TestIntervalQuery::testPhraseQuery() 
{
  checkHits(make_shared<IntervalQuery>(
                field, Intervals::phrase(
                           {Intervals::term(L"w1"), Intervals::term(L"w2")})),
            std::deque<int>{0});
}

void TestIntervalQuery::testOrderedNearQueryWidth3() 
{
  checkHits(make_shared<IntervalQuery>(
                field, Intervals::maxwidth(
                           3, Intervals::ordered({Intervals::term(L"w1"),
                                                  Intervals::term(L"w2")}))),
            std::deque<int>{0, 1, 2, 5});
}

void TestIntervalQuery::testOrderedNearQueryWidth4() 
{
  checkHits(make_shared<IntervalQuery>(
                field, Intervals::maxwidth(
                           4, Intervals::ordered({Intervals::term(L"w1"),
                                                  Intervals::term(L"w2")}))),
            std::deque<int>{0, 1, 2, 3, 5});
}

void TestIntervalQuery::testNestedOrderedNearQuery() 
{
  // onear/1(w1, onear/2(w2, w3))
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field, Intervals::ordered(
                 {Intervals::term(L"w1"),
                  Intervals::maxwidth(
                      3, Intervals::ordered({Intervals::term(L"w2"),
                                             Intervals::term(L"w3")}))}));

  checkHits(q, std::deque<int>{0, 1, 3});
}

void TestIntervalQuery::testUnorderedQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::unordered({Intervals::term(L"w1"), Intervals::term(L"w3")}));
  checkHits(q, std::deque<int>{0, 1, 2, 3, 5});
}

void TestIntervalQuery::testNonOverlappingQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field, Intervals::nonOverlapping(
                 Intervals::unordered(
                     {Intervals::term(L"w1"), Intervals::term(L"w3")}),
                 Intervals::unordered(
                     {Intervals::term(L"w2"), Intervals::term(L"w4")})));
  checkHits(q, std::deque<int>{1, 3, 5});
}

void TestIntervalQuery::testNotWithinQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::notWithin(Intervals::term(L"w1"), 1, Intervals::term(L"w2")));
  checkHits(q, std::deque<int>{1, 2, 3});
}

void TestIntervalQuery::testNotContainingQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::notContaining(Intervals::unordered({Intervals::term(L"w1"),
                                                     Intervals::term(L"w2")}),
                               Intervals::term(L"w3")));
  checkHits(q, std::deque<int>{0, 2, 4, 5});
}

void TestIntervalQuery::testContainingQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::containing(Intervals::unordered({Intervals::term(L"w1"),
                                                  Intervals::term(L"w2")}),
                            Intervals::term(L"w3")));
  checkHits(q, std::deque<int>{1, 3, 5});
}

void TestIntervalQuery::testContainedByQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::containedBy(Intervals::term(L"w3"),
                             Intervals::unordered({Intervals::term(L"w1"),
                                                   Intervals::term(L"w2")})));
  checkHits(q, std::deque<int>{1, 3, 5});
}

void TestIntervalQuery::testNotContainedByQuery() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field, Intervals::notContainedBy(
                 Intervals::term(L"w2"),
                 Intervals::unordered(
                     {Intervals::term(L"w1"), Intervals::term(L"w4")})));
  checkHits(q, std::deque<int>{1, 3, 4, 5});
}

void TestIntervalQuery::testNonExistentTerms() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::ordered({Intervals::term(L"w0"), Intervals::term(L"w2")}));
  checkHits(q, std::deque<int>());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testNestedOr() throws java.io.IOException
void TestIntervalQuery::testNestedOr() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field, Intervals::phrase(
                 {Intervals::term(L"coordinate"),
                  Intervals:: or ({Intervals::phrase({L"genome", L"mapping"}),
                                   Intervals::term(L"genome")}),
                  Intervals::term(L"research")}));
  checkHits(q, std::deque<int>{6, 7});
}

void TestIntervalQuery::testUnordered() 
{
  shared_ptr<Query> q = make_shared<IntervalQuery>(
      field,
      Intervals::unordered({Intervals::term(L"w1"),
                            Intervals::ordered({Intervals::term(L"w3"),
                                                Intervals::term(L"yy")})}));
  checkHits(q, std::deque<int>{3});
}
} // namespace org::apache::lucene::search::intervals