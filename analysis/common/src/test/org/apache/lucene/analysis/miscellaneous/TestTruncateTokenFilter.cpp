using namespace std;

#include "TestTruncateTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/TruncateTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using org::junit::Test;

void TestTruncateTokenFilter::testTruncating() 
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"abcdefg 1234567 ABCDEFG abcde abc 12345 123");
  stream = make_shared<TruncateTokenFilter>(stream, 5);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"abcde", L"12345", L"ABCDE", L"abcde",
                                   L"abc", L"12345", L"123"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testNonPositiveLength() throws Exception
void TestTruncateTokenFilter::testNonPositiveLength() 
{
  make_shared<TruncateTokenFilter>(
      whitespaceMockTokenizer(L"length must be a positive number"), -48);
}
} // namespace org::apache::lucene::analysis::miscellaneous