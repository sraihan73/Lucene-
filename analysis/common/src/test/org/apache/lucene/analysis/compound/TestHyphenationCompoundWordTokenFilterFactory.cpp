using namespace std;

#include "TestHyphenationCompoundWordTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::compound
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestHyphenationCompoundWordTokenFilterFactory::
    testHyphenationWithDictionary() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"min veninde som er lidt af en læsehest");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"HyphenationCompoundWord",
                              {L"hyphenator", L"da_UTF8.xml", L"dictionary",
                               L"da_compoundDictionary.txt"})
               ->create(stream);

  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"min", L"veninde", L"som",
                                                 L"er", L"lidt", L"af", L"en",
                                                 L"læsehest", L"læse", L"hest"},
                            std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 0, 0});
}

void TestHyphenationCompoundWordTokenFilterFactory::testHyphenationOnly() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"basketballkurv");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"HyphenationCompoundWord",
                              {L"hyphenator", L"da_UTF8.xml", L"minSubwordSize",
                               L"2", L"maxSubwordSize", L"4"})
               ->create(stream);

  assertTokenStreamContents(stream, std::deque<wstring>{L"basketballkurv",
                                                         L"ba", L"sket", L"bal",
                                                         L"ball", L"kurv"});
}

void TestHyphenationCompoundWordTokenFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(
        L"HyphenationCompoundWord",
        {L"hyphenator", L"da_UTF8.xml", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::compound