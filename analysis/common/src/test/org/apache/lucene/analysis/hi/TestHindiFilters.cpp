using namespace std;

#include "TestHindiFilters.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::hi
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestHindiFilters::testIndicNormalizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"ত্‍ अाैर");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"ৎ", L"और"});
}

void TestHindiFilters::testHindiNormalizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"क़िताब");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  stream = tokenFilterFactory(L"HindiNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"किताब"});
}

void TestHindiFilters::testStemmer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"किताबें");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  stream = tokenFilterFactory(L"HindiNormalization").create(stream);
  stream = tokenFilterFactory(L"HindiStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"किताब"});
}

void TestHindiFilters::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"IndicNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"HindiNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"HindiStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::hi