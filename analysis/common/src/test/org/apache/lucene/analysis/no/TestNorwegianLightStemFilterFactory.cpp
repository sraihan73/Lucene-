using namespace std;

#include "TestNorwegianLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::no
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestNorwegianLightStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"epler eple");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"NorwegianLightStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"epl", L"epl"});
}

void TestNorwegianLightStemFilterFactory::testBokmaalStemming() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"epler eple");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"NorwegianLightStem", {L"variant", L"nb"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"epl", L"epl"});
}

void TestNorwegianLightStemFilterFactory::testNynorskStemming() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"gutar gutane");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"NorwegianLightStem", {L"variant", L"nn"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"gut", L"gut"});
}

void TestNorwegianLightStemFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"NorwegianLightStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::no