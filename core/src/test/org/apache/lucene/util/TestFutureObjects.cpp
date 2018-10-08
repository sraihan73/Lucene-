using namespace std;

#include "TestFutureObjects.h"

namespace org::apache::lucene::util
{

void TestFutureObjects::testCheckIndex()
{
  assertEquals(0, FutureObjects::checkIndex(0, 1));
  assertEquals(1, FutureObjects::checkIndex(1, 2));

  runtime_error e = expectThrows(out_of_range::typeid,
                                 [&]() { FutureObjects::checkIndex(-1, 0); });
  assertEquals(L"Index -1 out-of-bounds for length 0", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkIndex(0, 0); });
  assertEquals(L"Index 0 out-of-bounds for length 0", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkIndex(1, 0); });
  assertEquals(L"Index 1 out-of-bounds for length 0", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkIndex(0, -1); });
  assertEquals(L"Index 0 out-of-bounds for length -1", e.what());
}

void TestFutureObjects::testCheckFromToIndex()
{
  assertEquals(0, FutureObjects::checkFromToIndex(0, 0, 0));
  assertEquals(1, FutureObjects::checkFromToIndex(1, 2, 2));

  runtime_error e = expectThrows(out_of_range::typeid, [&]() {
    FutureObjects::checkFromToIndex(-1, 0, 0);
  });
  assertEquals(L"Range [-1, 0) out-of-bounds for length 0", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromToIndex(1, 0, 2); });
  assertEquals(L"Range [1, 0) out-of-bounds for length 2", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromToIndex(1, 3, 2); });
  assertEquals(L"Range [1, 3) out-of-bounds for length 2", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromToIndex(0, 0, -1); });
  assertEquals(L"Range [0, 0) out-of-bounds for length -1", e.what());
}

void TestFutureObjects::testCheckFromIndexSize()
{
  assertEquals(0, FutureObjects::checkFromIndexSize(0, 0, 0));
  assertEquals(1, FutureObjects::checkFromIndexSize(1, 2, 3));

  runtime_error e = expectThrows(out_of_range::typeid, [&]() {
    FutureObjects::checkFromIndexSize(-1, 0, 1);
  });
  assertEquals(L"Range [-1, -1 + 0) out-of-bounds for length 1", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromIndexSize(0, -1, 1); });
  assertEquals(L"Range [0, 0 + -1) out-of-bounds for length 1", e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromIndexSize(0, 2, 1); });
  assertEquals(L"Range [0, 0 + 2) out-of-bounds for length 1", e.what());

  e = expectThrows(out_of_range::typeid, [&]() {
    FutureObjects::checkFromIndexSize(1, numeric_limits<int>::max(),
                                      numeric_limits<int>::max());
  });
  assertEquals(L"Range [1, 1 + 2147483647) out-of-bounds for length 2147483647",
               e.what());

  e = expectThrows(out_of_range::typeid,
                   [&]() { FutureObjects::checkFromIndexSize(0, 0, -1); });
  assertEquals(L"Range [0, 0 + 0) out-of-bounds for length -1", e.what());
}
} // namespace org::apache::lucene::util