using namespace std;

#include "TestFrenchLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::fr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestFrenchLightStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"administrativement");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"FrenchLightStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"administratif"});
}

void TestFrenchLightStemFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"FrenchLightStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::fr