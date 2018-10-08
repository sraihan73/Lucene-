using namespace std;

#include "TestGreekLowerCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::el
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestGreekLowerCaseFilterFactory::testNormalization() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"Μάϊος ΜΆΪΟΣ");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"GreekLowerCase").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"μαιοσ", L"μαιοσ"});
}

void TestGreekLowerCaseFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"GreekLowerCase", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::el