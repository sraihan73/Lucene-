using namespace std;

#include "TestPatternReplaceFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestPatternReplaceFilterFactory::testReplaceAll() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"aabfooaabfooabfoob ab caaaaaaaaab");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"PatternReplace",
                              {L"pattern", L"a*b", L"replacement", L"-"})
               ->create(stream);

  assertTokenStreamContents(
      stream, std::deque<wstring>{L"-foo-foo-foo-", L"-", L"c-"});
}

void TestPatternReplaceFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"PatternReplace",
                       {L"pattern", L"something", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::pattern