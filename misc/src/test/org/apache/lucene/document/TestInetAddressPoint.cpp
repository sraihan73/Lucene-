using namespace std;

#include "TestInetAddressPoint.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestInetAddressPoint::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with an address
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<InetAddress> address = InetAddress::getByName(L"1.2.3.4");
  document->push_back(make_shared<InetAddressPoint>(L"field", address));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(
      1, searcher->count(InetAddressPoint::newExactQuery(L"field", address)));
  assertEquals(1, searcher->count(
                      InetAddressPoint::newPrefixQuery(L"field", address, 24)));
  assertEquals(1, searcher->count(InetAddressPoint::newRangeQuery(
                      L"field", InetAddress::getByName(L"1.2.3.3"),
                      InetAddress::getByName(L"1.2.3.5"))));
  assertEquals(1, searcher->count(InetAddressPoint::newSetQuery(
                      L"field", {InetAddress::getByName(L"1.2.3.4")})));
  assertEquals(1, searcher->count(InetAddressPoint::newSetQuery(
                      L"field", {InetAddress::getByName(L"1.2.3.4"),
                                 InetAddress::getByName(L"1.2.3.5")})));
  assertEquals(0, searcher->count(InetAddressPoint::newSetQuery(
                      L"field", {InetAddress::getByName(L"1.2.3.3")})));
  assertEquals(0, searcher->count(InetAddressPoint::newSetQuery(L"field")));

  delete reader;
  delete writer;
  delete dir;
}

void TestInetAddressPoint::testBasicsV6() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with an address
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<InetAddress> address = InetAddress::getByName(L"fec0::f66d");
  document->push_back(make_shared<InetAddressPoint>(L"field", address));
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(
      1, searcher->count(InetAddressPoint::newExactQuery(L"field", address)));
  assertEquals(1, searcher->count(
                      InetAddressPoint::newPrefixQuery(L"field", address, 64)));
  assertEquals(1, searcher->count(InetAddressPoint::newRangeQuery(
                      L"field", InetAddress::getByName(L"fec0::f66c"),
                      InetAddress::getByName(L"fec0::f66e"))));

  delete reader;
  delete writer;
  delete dir;
}

void TestInetAddressPoint::testToString() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"InetAddressPoint <field:1.2.3.4>",
               (make_shared<InetAddressPoint>(
                    L"field", InetAddress::getByName(L"1.2.3.4")))
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"InetAddressPoint <field:1.2.3.4>",
               (make_shared<InetAddressPoint>(
                    L"field", InetAddress::getByName(L"::FFFF:1.2.3.4")))
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"InetAddressPoint <field:[fdc8:57ed:f042:ad1:f66d:4ff:fe90:ce0c]>",
      (make_shared<InetAddressPoint>(
           L"field",
           InetAddress::getByName(L"fdc8:57ed:f042:0ad1:f66d:4ff:fe90:ce0c")))
          ->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:[1.2.3.4 TO 1.2.3.4]",
               InetAddressPoint::newExactQuery(
                   L"field", InetAddress::getByName(L"1.2.3.4"))
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:[0:0:0:0:0:0:0:1 TO 0:0:0:0:0:0:0:1]",
      InetAddressPoint::newExactQuery(L"field", InetAddress::getByName(L"::1"))
          ->toString());

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:[1.2.3.0 TO 1.2.3.255]",
               InetAddressPoint::newPrefixQuery(
                   L"field", InetAddress::getByName(L"1.2.3.4"), 24)
                   ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:[fdc8:57ed:f042:ad1:0:0:0:0 TO "
      L"fdc8:57ed:f042:ad1:ffff:ffff:ffff:ffff]",
      InetAddressPoint::newPrefixQuery(
          L"field",
          InetAddress::getByName(L"fdc8:57ed:f042:0ad1:f66d:4ff:fe90:ce0c"), 64)
          ->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:{fdc8:57ed:f042:ad1:f66d:4ff:fe90:ce0c}",
               InetAddressPoint::newSetQuery(
                   L"field", {InetAddress::getByName(
                                 L"fdc8:57ed:f042:0ad1:f66d:4ff:fe90:ce0c")})
                   ->toString());
}

void TestInetAddressPoint::testQueryEquals() 
{
  shared_ptr<Query> q1, q2;
  q1 = InetAddressPoint::newRangeQuery(L"a", InetAddress::getByName(L"1.2.3.3"),
                                       InetAddress::getByName(L"1.2.3.5"));
  q2 = InetAddressPoint::newRangeQuery(L"a", InetAddress::getByName(L"1.2.3.3"),
                                       InetAddress::getByName(L"1.2.3.5"));
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(
      InetAddressPoint::newRangeQuery(L"a", InetAddress::getByName(L"1.2.3.3"),
                                      InetAddress::getByName(L"1.2.3.7"))));
  assertFalse(q1->equals(
      InetAddressPoint::newRangeQuery(L"b", InetAddress::getByName(L"1.2.3.3"),
                                      InetAddress::getByName(L"1.2.3.5"))));

  q1 = InetAddressPoint::newPrefixQuery(L"a",
                                        InetAddress::getByName(L"1.2.3.3"), 16);
  q2 = InetAddressPoint::newPrefixQuery(L"a",
                                        InetAddress::getByName(L"1.2.3.3"), 16);
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(InetAddressPoint::newPrefixQuery(
      L"a", InetAddress::getByName(L"1.1.3.5"), 16)));
  assertFalse(q1->equals(InetAddressPoint::newPrefixQuery(
      L"a", InetAddress::getByName(L"1.2.3.5"), 24)));

  q1 =
      InetAddressPoint::newExactQuery(L"a", InetAddress::getByName(L"1.2.3.3"));
  q2 =
      InetAddressPoint::newExactQuery(L"a", InetAddress::getByName(L"1.2.3.3"));
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(InetAddressPoint::newExactQuery(
      L"a", InetAddress::getByName(L"1.2.3.5"))));

  q1 =
      InetAddressPoint::newSetQuery(L"a", {InetAddress::getByName(L"1.2.3.3"),
                                           InetAddress::getByName(L"1.2.3.5")});
  q2 =
      InetAddressPoint::newSetQuery(L"a", {InetAddress::getByName(L"1.2.3.3"),
                                           InetAddress::getByName(L"1.2.3.5")});
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
  assertFalse(q1->equals(InetAddressPoint::newSetQuery(
      L"a", {InetAddress::getByName(L"1.2.3.3"),
             InetAddress::getByName(L"1.2.3.7")})));
}

void TestInetAddressPoint::testPrefixQuery() 
{
  assertEquals(
      InetAddressPoint::newRangeQuery(L"a", InetAddress::getByName(L"1.2.3.0"),
                                      InetAddress::getByName(L"1.2.3.255")),
      InetAddressPoint::newPrefixQuery(
          L"a", InetAddress::getByName(L"1.2.3.127"), 24));
  assertEquals(InetAddressPoint::newRangeQuery(
                   L"a", InetAddress::getByName(L"1.2.3.128"),
                   InetAddress::getByName(L"1.2.3.255")),
               InetAddressPoint::newPrefixQuery(
                   L"a", InetAddress::getByName(L"1.2.3.213"), 25));
  assertEquals(InetAddressPoint::newRangeQuery(
                   L"a", InetAddress::getByName(L"2001::a000:0"),
                   InetAddress::getByName(L"2001::afff:ffff")),
               InetAddressPoint::newPrefixQuery(
                   L"a", InetAddress::getByName(L"2001::a6bd:fc80"), 100));
}

void TestInetAddressPoint::testNextUp() 
{
  assertEquals(InetAddress::getByName(L"::1"),
               InetAddressPoint::nextUp(InetAddress::getByName(L"::")));

  assertEquals(InetAddress::getByName(L"::1:0"),
               InetAddressPoint::nextUp(InetAddress::getByName(L"::ffff")));

  assertEquals(InetAddress::getByName(L"1.2.4.0"),
               InetAddressPoint::nextUp(InetAddress::getByName(L"1.2.3.255")));

  assertEquals(
      InetAddress::getByName(L"0.0.0.0"),
      InetAddressPoint::nextUp(InetAddress::getByName(L"::fffe:ffff:ffff")));

  assertEquals(
      InetAddress::getByName(L"::1:0:0:0"),
      InetAddressPoint::nextUp(InetAddress::getByName(L"255.255.255.255")));

  shared_ptr<ArithmeticException> e =
      expectThrows(ArithmeticException::typeid, [&]() {
        InetAddressPoint::nextUp(java::net::InetAddress::getByName(
            L"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
      });
  assertEquals(L"Overflow: there is no greater InetAddress than "
               L"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
               e->getMessage());
}

void TestInetAddressPoint::testNextDown() 
{
  assertEquals(
      InetAddress::getByName(L"ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"),
      InetAddressPoint::nextDown(
          InetAddress::getByName(L"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")));

  assertEquals(InetAddress::getByName(L"::ffff"),
               InetAddressPoint::nextDown(InetAddress::getByName(L"::1:0")));

  assertEquals(InetAddress::getByName(L"1.2.3.255"),
               InetAddressPoint::nextDown(InetAddress::getByName(L"1.2.4.0")));

  assertEquals(InetAddress::getByName(L"::fffe:ffff:ffff"),
               InetAddressPoint::nextDown(InetAddress::getByName(L"0.0.0.0")));

  assertEquals(
      InetAddress::getByName(L"255.255.255.255"),
      InetAddressPoint::nextDown(InetAddress::getByName(L"::1:0:0:0")));

  shared_ptr<ArithmeticException> e =
      expectThrows(ArithmeticException::typeid, [&]() {
        InetAddressPoint::nextDown(java::net::InetAddress::getByName(L"::"));
      });
  assertEquals(
      L"Underflow: there is no smaller InetAddress than 0:0:0:0:0:0:0:0",
      e->getMessage());
}
} // namespace org::apache::lucene::document