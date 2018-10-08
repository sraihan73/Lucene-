using namespace std;

#include "TestBengaliFilters.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::bn
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestBengaliFilters::testIndicNormalizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"ত্‍ আমি");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"ৎ", L"আমি"});
}

void TestBengaliFilters::testBengaliNormalizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"বাড়ী");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  stream = tokenFilterFactory(L"BengaliNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"বারি"});
}

void TestBengaliFilters::testStemmer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"বাড়ী");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"IndicNormalization").create(stream);
  stream = tokenFilterFactory(L"BengaliNormalization").create(stream);
  stream = tokenFilterFactory(L"BengaliStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"বার"});
}

void TestBengaliFilters::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"IndicNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"BengaliNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"BengaliStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::bn