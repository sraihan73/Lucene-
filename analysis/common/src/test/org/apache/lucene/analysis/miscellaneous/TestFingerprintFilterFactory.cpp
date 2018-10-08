using namespace std;

#include "TestFingerprintFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/FingerprintFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestFingerprintFilterFactory::test() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Reader> reader = make_shared<StringReader>(L"A1 B2 A1 D4 C3");
    shared_ptr<MockTokenizer> tokenizer =
        make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
    tokenizer->setReader(reader);
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = tokenizer;
    stream = tokenFilterFactory(
                 L"Fingerprint",
                 {FingerprintFilterFactory::MAX_OUTPUT_TOKEN_SIZE_KEY, L"256",
                  FingerprintFilterFactory::SEPARATOR_KEY, L"_"})
                 ->create(stream);
    assertTokenStreamContents(stream, std::deque<wstring>{L"A1_B2_C3_D4"});
  }
}

void TestFingerprintFilterFactory::testRequired() 
{
  // no params are required
  tokenFilterFactory(L"Fingerprint");
}

void TestFingerprintFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Fingerprint",
                       {FingerprintFilterFactory::MAX_OUTPUT_TOKEN_SIZE_KEY,
                        L"3", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous