using namespace std;

#include "TestTrimFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestTrimFilterFactory::testTrimming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"trim me    ");
  shared_ptr<TokenStream> stream = keywordMockTokenizer(reader);
  stream = tokenFilterFactory(L"Trim").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"trim me"});
}

void TestTrimFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Trim", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous