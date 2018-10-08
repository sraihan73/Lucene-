using namespace std;

#include "TestScandinavianFoldingFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestScandinavianFoldingFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"räksmörgås");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"ScandinavianFolding").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"raksmorgas"});
}

void TestScandinavianFoldingFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ScandinavianFolding", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous