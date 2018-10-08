using namespace std;

#include "TestArabicFilters.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::analysis::ar
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestArabicFilters::testNormalizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"الذين مَلكت أيمانكم");
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(reader);
  shared_ptr<TokenStream> stream =
      tokenFilterFactory(L"ArabicNormalization").create(tokenizer);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"الذين", L"ملكت", L"ايمانكم"});
}

void TestArabicFilters::testStemmer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"الذين مَلكت أيمانكم");
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(reader);
  shared_ptr<TokenStream> stream =
      tokenFilterFactory(L"ArabicNormalization").create(tokenizer);
  stream = tokenFilterFactory(L"ArabicStem").create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"ذين", L"ملكت", L"ايمانكم"});
}

void TestArabicFilters::testPersianCharFilter() 
{
  shared_ptr<Reader> reader =
      charFilterFactory(L"Persian")
          .create(make_shared<StringReader>(L"می‌خورد"));
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(reader);
  assertTokenStreamContents(tokenizer, std::deque<wstring>{L"می", L"خورد"});
}

void TestArabicFilters::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ArabicNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Arabicstem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    charFilterFactory(L"Persian", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::ar