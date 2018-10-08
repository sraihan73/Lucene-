using namespace std;

#include "TestLimitTokenPositionFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenPositionFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestLimitTokenPositionFilterFactory::testMaxPosition1() throw(
    runtime_error)
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Reader> reader = make_shared<StringReader>(L"A1 B2 C3 D4 E5 F6");
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(reader);
    // if we are consuming all tokens, we can use the checks, otherwise we can't
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = tokenizer;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    stream = tokenFilterFactory(
                 L"LimitTokenPosition",
                 {LimitTokenPositionFilterFactory::MAX_TOKEN_POSITION_KEY, L"1",
                  LimitTokenPositionFilterFactory::CONSUME_ALL_TOKENS_KEY,
                  Boolean::toString(consumeAll)})
                 ->create(stream);
    assertTokenStreamContents(stream, std::deque<wstring>{L"A1"});
  }
}

void TestLimitTokenPositionFilterFactory::testMissingParam() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"LimitTokenPosition");
  });
  assertTrue(L"exception doesn't mention param: " + expected.what(),
             0 < expected.what()->find(
                     LimitTokenPositionFilterFactory::MAX_TOKEN_POSITION_KEY));
}

void TestLimitTokenPositionFilterFactory::testMaxPosition1WithShingles() throw(
    runtime_error)
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Reader> reader =
        make_shared<StringReader>(L"one two three four five");
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(reader);
    // if we are consuming all tokens, we can use the checks, otherwise we can't
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = tokenizer;
    stream = tokenFilterFactory(L"Shingle",
                                {L"minShingleSize", L"2", L"maxShingleSize",
                                 L"3", L"outputUnigrams", L"true"})
                 ->create(stream);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    stream = tokenFilterFactory(
                 L"LimitTokenPosition",
                 {LimitTokenPositionFilterFactory::MAX_TOKEN_POSITION_KEY, L"1",
                  LimitTokenPositionFilterFactory::CONSUME_ALL_TOKENS_KEY,
                  Boolean::toString(consumeAll)})
                 ->create(stream);
    assertTokenStreamContents(
        stream, std::deque<wstring>{L"one", L"one two", L"one two three"});
  }
}

void TestLimitTokenPositionFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"LimitTokenPosition",
                       {L"maxTokenPosition", L"3", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous