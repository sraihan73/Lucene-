using namespace std;

#include "TestLimitTokenCountFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenCountFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestLimitTokenCountFilterFactory::test() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Reader> reader = make_shared<StringReader>(L"A1 B2 C3 D4 E5 F6");
    shared_ptr<MockTokenizer> tokenizer =
        make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
    tokenizer->setReader(reader);
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = tokenizer;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    stream = tokenFilterFactory(
                 L"LimitTokenCount",
                 {LimitTokenCountFilterFactory::MAX_TOKEN_COUNT_KEY, L"3",
                  LimitTokenCountFilterFactory::CONSUME_ALL_TOKENS_KEY,
                  Boolean::toString(consumeAll)})
                 ->create(stream);
    assertTokenStreamContents(stream,
                              std::deque<wstring>{L"A1", L"B2", L"C3"});
  }
}

void TestLimitTokenCountFilterFactory::testRequired() 
{
  // param is required
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"LimitTokenCount");
  });
  assertTrue(L"exception doesn't mention param: " + expected.what(),
             0 < expected.what()->find(
                     LimitTokenCountFilterFactory::MAX_TOKEN_COUNT_KEY));
}

void TestLimitTokenCountFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"LimitTokenCount",
                       {LimitTokenCountFilterFactory::MAX_TOKEN_COUNT_KEY, L"3",
                        L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous