using namespace std;

#include "TestLimitTokenOffsetFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenOffsetFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using org::junit::Test;

void TestLimitTokenOffsetFilter::test() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer =
        whitespaceMockTokenizer(L"A1 B2 C3 D4 E5 F6");
    tokenizer->setEnableChecks(consumeAll);
    // note with '3', this test would fail if erroneously the filter used
    // endOffset instead
    shared_ptr<TokenStream> stream =
        make_shared<LimitTokenOffsetFilter>(tokenizer, 3, consumeAll);
    assertTokenStreamContents(stream, std::deque<wstring>{L"A1", L"B2"});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments() throws Exception
void TestLimitTokenOffsetFilter::testIllegalArguments() 
{
  make_shared<LimitTokenOffsetFilter>(
      whitespaceMockTokenizer(L"A1 B2 C3 D4 E5 F6"), -1);
}
} // namespace org::apache::lucene::analysis::miscellaneous