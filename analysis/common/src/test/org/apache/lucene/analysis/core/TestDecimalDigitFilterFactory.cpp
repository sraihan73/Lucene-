using namespace std;

#include "TestDecimalDigitFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestDecimalDigitFilterFactory::testNormalization() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"١٢٣٤");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"DecimalDigit").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"1234"});
}

void TestDecimalDigitFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"DecimalDigit", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::core