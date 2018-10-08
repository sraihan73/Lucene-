using namespace std;

#include "TestPortugueseStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::pt
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestPortugueseStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"maluquice");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"PortugueseStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"maluc"});
}

void TestPortugueseStemFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"PortugueseStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::pt