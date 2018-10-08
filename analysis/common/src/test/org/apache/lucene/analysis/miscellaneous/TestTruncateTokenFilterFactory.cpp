using namespace std;

#include "TestTruncateTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/TruncateTokenFilterFactory.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestTruncateTokenFilterFactory::testTruncating() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"abcdefg 1234567 ABCDEFG abcde abc 12345 123");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream =
      tokenFilterFactory(L"Truncate",
                         {TruncateTokenFilterFactory::PREFIX_LENGTH_KEY, L"5"})
          ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"abcde", L"12345", L"ABCDE", L"abcde",
                                   L"abc", L"12345", L"123"});
}

void TestTruncateTokenFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Truncate",
                       {TruncateTokenFilterFactory::PREFIX_LENGTH_KEY, L"5",
                        L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameter(s):"));
}

void TestTruncateTokenFilterFactory::
    testNonPositivePrefixLengthArgument() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Truncate",
                       {TruncateTokenFilterFactory::PREFIX_LENGTH_KEY, L"-5"});
  });
  assertTrue(
      expected.what()->contains(TruncateTokenFilterFactory::PREFIX_LENGTH_KEY +
                                L" parameter must be a positive number: -5"));
}
} // namespace org::apache::lucene::analysis::miscellaneous