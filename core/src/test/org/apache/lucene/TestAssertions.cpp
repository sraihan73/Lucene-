using namespace std;

#include "TestAssertions.h"

namespace org::apache::lucene
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

bool TestAssertions::TestTokenStream1::incrementToken() { return false; }

bool TestAssertions::TestTokenStream2::incrementToken() { return false; }

bool TestAssertions::TestTokenStream3::incrementToken() { return false; }

void TestAssertions::testTokenStreams()
{
  make_shared<TestTokenStream1>();
  make_shared<TestTokenStream2>();
  try {
    make_shared<TestTokenStream3>();
    if (assertsAreEnabled) {
      fail(L"TestTokenStream3 should fail assertion");
    }
  } catch (const AssertionError &e) {
    // expected
  }
}
} // namespace org::apache::lucene