using namespace std;

#include "TestScandinavianNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestScandinavianNormalizationFilterFactory::testStemming() throw(
    runtime_error)
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"räksmörgås");
  stream = tokenFilterFactory(L"ScandinavianNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"ræksmørgås"});
}

void TestScandinavianNormalizationFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ScandinavianNormalization",
                       {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous