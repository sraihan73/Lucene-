using namespace std;

#include "TestCJKWidthFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestCJKWidthFilterFactory::test() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"Ｔｅｓｔ １２３４");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"CJKWidth").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Test", L"1234"});
}

void TestCJKWidthFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"CJKWidth", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::cjk