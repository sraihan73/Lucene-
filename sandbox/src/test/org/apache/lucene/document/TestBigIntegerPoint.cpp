using namespace std;

#include "TestBigIntegerPoint.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBigIntegerPoint::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a large biginteger value
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<int64_t> large =
      static_cast<int64_t>(numeric_limits<int64_t>::max())
          .multiply(static_cast<int64_t>(64));
  document->push_back(make_shared<BigIntegerPoint>(L"field", large));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(
      1, searcher->count(BigIntegerPoint::newExactQuery(L"field", large)));
  assertEquals(1, searcher->count(BigIntegerPoint::newRangeQuery(
                      L"field", large->subtract(int64_t::ONE),
                      large->add(int64_t::ONE))));
  assertEquals(
      1, searcher->count(BigIntegerPoint::newSetQuery(L"field", {large})));
  assertEquals(0, searcher->count(BigIntegerPoint::newSetQuery(
                      L"field", {large->subtract(int64_t::ONE)})));
  assertEquals(0, searcher->count(BigIntegerPoint::newSetQuery(L"field")));

  delete reader;
  delete writer;
  delete dir;
}

void TestBigIntegerPoint::testNegative() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a large biginteger value
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<int64_t> negative =
      static_cast<int64_t>(numeric_limits<int64_t>::max())
          .multiply(static_cast<int64_t>(64))
          .negate();
  document->push_back(make_shared<BigIntegerPoint>(L"field", negative));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(
      1, searcher->count(BigIntegerPoint::newExactQuery(L"field", negative)));
  assertEquals(1, searcher->count(BigIntegerPoint::newRangeQuery(
                      L"field", negative->subtract(int64_t::ONE),
                      negative->add(int64_t::ONE))));

  delete reader;
  delete writer;
  delete dir;
}

void TestBigIntegerPoint::testTooLarge() 
{
  shared_ptr<int64_t> tooLarge = int64_t::ONE::shiftLeft(128);
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<BigIntegerPoint>(L"field", tooLarge);
  });
  assertTrue(expected.what()->contains(L"requires more than 16 bytes storage"));
}

void TestBigIntegerPoint::testToString() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"BigIntegerPoint <field:1>",
      (make_shared<BigIntegerPoint>(L"field", int64_t::ONE))->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"BigIntegerPoint <field:1,-2>",
               (make_shared<BigIntegerPoint>(L"field", int64_t::ONE,
                                             static_cast<int64_t>(-2)))
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:[1 TO 1]",
      BigIntegerPoint::newExactQuery(L"field", int64_t::ONE)->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:[1 TO 17]",
               BigIntegerPoint::newRangeQuery(L"field", int64_t::ONE,
                                              static_cast<int64_t>(17))
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:[1 TO 17],[0 TO 42]",
      BigIntegerPoint::newRangeQuery(
          L"field",
          std::deque<std::shared_ptr<int64_t>>{int64_t::ONE,
                                                   int64_t::ZERO},
          std::deque<std::shared_ptr<int64_t>>{static_cast<int64_t>(17),
                                                   static_cast<int64_t>(42)})
          ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:{1}",
      BigIntegerPoint::newSetQuery(L"field", {int64_t::ONE})->toString());
}

void TestBigIntegerPoint::testQueryEquals() 
{
  shared_ptr<Query> q1, q2;
  q1 = BigIntegerPoint::newRangeQuery(L"a", static_cast<int64_t>(0),
                                      static_cast<int64_t>(1000));
  q2 = BigIntegerPoint::newRangeQuery(L"a", static_cast<int64_t>(0),
                                      static_cast<int64_t>(1000));
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(BigIntegerPoint::newRangeQuery(
      L"a", static_cast<int64_t>(1), static_cast<int64_t>(1000))));
  assertFalse(q1->equals(BigIntegerPoint::newRangeQuery(
      L"b", static_cast<int64_t>(0), static_cast<int64_t>(1000))));

  q1 = BigIntegerPoint::newExactQuery(L"a", static_cast<int64_t>(1000));
  q2 = BigIntegerPoint::newExactQuery(L"a", static_cast<int64_t>(1000));
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(
      BigIntegerPoint::newExactQuery(L"a", static_cast<int64_t>(1))));

  q1 = BigIntegerPoint::newSetQuery(L"a", {static_cast<int64_t>(0),
                                           static_cast<int64_t>(1000),
                                           static_cast<int64_t>(17)});
  q2 = BigIntegerPoint::newSetQuery(L"a", {static_cast<int64_t>(17),
                                           static_cast<int64_t>(0),
                                           static_cast<int64_t>(1000)});
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(BigIntegerPoint::newSetQuery(
      L"a", {static_cast<int64_t>(1), static_cast<int64_t>(17),
             static_cast<int64_t>(1000)})));
}
} // namespace org::apache::lucene::document