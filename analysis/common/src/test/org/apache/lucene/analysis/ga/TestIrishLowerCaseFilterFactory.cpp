using namespace std;

#include "TestIrishLowerCaseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::ga
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestIrishLowerCaseFilterFactory::testCasing() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"nAthair tUISCE hARD");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IrishLowerCase").create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"n-athair", L"t-uisce", L"hard"});
}

void TestIrishLowerCaseFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"IrishLowerCase", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::ga