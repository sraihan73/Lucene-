using namespace std;

#include "DelimitedTermFrequencyTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermFrequencyAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/DelimitedTermFrequencyTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;

void DelimitedTermFrequencyTokenFilterTest::testTermFrequency() throw(
    runtime_error)
{
  wstring test =
      L"The quick|40 red|4 fox|06 jumped|1 over the lazy|2 brown|123 dogs|1024";
  shared_ptr<DelimitedTermFrequencyTokenFilter> filter =
      make_shared<DelimitedTermFrequencyTokenFilter>(
          whitespaceMockTokenizer(test));
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<TermFrequencyAttribute> tfAtt =
      filter->getAttribute(TermFrequencyAttribute::typeid);
  filter->reset();
  assertTermEquals(L"The", filter, termAtt, tfAtt, 1);
  assertTermEquals(L"quick", filter, termAtt, tfAtt, 40);
  assertTermEquals(L"red", filter, termAtt, tfAtt, 4);
  assertTermEquals(L"fox", filter, termAtt, tfAtt, 6);
  assertTermEquals(L"jumped", filter, termAtt, tfAtt, 1);
  assertTermEquals(L"over", filter, termAtt, tfAtt, 1);
  assertTermEquals(L"the", filter, termAtt, tfAtt, 1);
  assertTermEquals(L"lazy", filter, termAtt, tfAtt, 2);
  assertTermEquals(L"brown", filter, termAtt, tfAtt, 123);
  assertTermEquals(L"dogs", filter, termAtt, tfAtt, 1024);
  assertFalse(filter->incrementToken());
  filter->end();
  delete filter;
}

void DelimitedTermFrequencyTokenFilterTest::testInvalidNegativeTf() throw(
    runtime_error)
{
  wstring test = L"foo bar|-20";
  shared_ptr<DelimitedTermFrequencyTokenFilter> filter =
      make_shared<DelimitedTermFrequencyTokenFilter>(
          whitespaceMockTokenizer(test));
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<TermFrequencyAttribute> tfAtt =
      filter->getAttribute(TermFrequencyAttribute::typeid);
  filter->reset();
  assertTermEquals(L"foo", filter, termAtt, tfAtt, 1);
  invalid_argument iae =
      expectThrows(invalid_argument::typeid, filter::incrementToken);
  assertEquals(L"Term frequency must be 1 or greater; got -20", iae.what());
}

void DelimitedTermFrequencyTokenFilterTest::testInvalidFloatTf() throw(
    runtime_error)
{
  wstring test = L"foo bar|1.2";
  shared_ptr<DelimitedTermFrequencyTokenFilter> filter =
      make_shared<DelimitedTermFrequencyTokenFilter>(
          whitespaceMockTokenizer(test));
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<TermFrequencyAttribute> tfAtt =
      filter->getAttribute(TermFrequencyAttribute::typeid);
  filter->reset();
  assertTermEquals(L"foo", filter, termAtt, tfAtt, 1);
  expectThrows(NumberFormatException::typeid, filter::incrementToken);
}

void DelimitedTermFrequencyTokenFilterTest::assertTermEquals(
    const wstring &expected, shared_ptr<TokenStream> stream,
    shared_ptr<CharTermAttribute> termAtt,
    shared_ptr<TermFrequencyAttribute> tfAtt,
    int expectedTf) 
{
  assertTrue(stream->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expected, termAtt->toString());
  assertEquals(expectedTf, tfAtt->getTermFrequency());
}
} // namespace org::apache::lucene::analysis::miscellaneous