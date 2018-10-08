using namespace std;

#include "TestFingerprintFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/FingerprintFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestFingerprintFilter::testDupsAndSorting() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(L"B A B E");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = make_shared<FingerprintFilter>(tokenizer);
    assertTokenStreamContents(stream, std::deque<wstring>{L"A B E"});
  }
}

void TestFingerprintFilter::testAllDupValues() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(L"B2 B2");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = make_shared<FingerprintFilter>(tokenizer);
    assertTokenStreamContents(stream, std::deque<wstring>{L"B2"});
  }
}

void TestFingerprintFilter::testMaxFingerprintSize() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer =
        whitespaceMockTokenizer(L"B2 A1 C3 D4 E5 F6 G7 H1");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream =
        make_shared<FingerprintFilter>(tokenizer, 4, L' ');
    assertTokenStreamContents(stream, std::deque<wstring>());
  }
}

void TestFingerprintFilter::testCustomSeparator() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer =
        whitespaceMockTokenizer(L"B2 A1 C3 B2");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = make_shared<FingerprintFilter>(
        tokenizer, FingerprintFilter::DEFAULT_MAX_OUTPUT_TOKEN_SIZE, L'_');
    assertTokenStreamContents(stream, std::deque<wstring>{L"A1_B2_C3"});
  }
}

void TestFingerprintFilter::testSingleToken() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(L"A1");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = make_shared<FingerprintFilter>(tokenizer);
    assertTokenStreamContents(stream, std::deque<wstring>{L"A1"});
  }
}

void TestFingerprintFilter::testEmpty() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer = whitespaceMockTokenizer(L"");
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = make_shared<FingerprintFilter>(tokenizer);
    assertTokenStreamContents(stream, std::deque<wstring>(0));
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous