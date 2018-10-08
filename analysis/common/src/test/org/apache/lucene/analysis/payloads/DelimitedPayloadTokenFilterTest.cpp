using namespace std;

#include "DelimitedPayloadTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/DelimitedPayloadTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/FloatEncoder.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/IdentityEncoder.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/IntegerEncoder.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

void DelimitedPayloadTokenFilterTest::testPayloads() 
{
  wstring test =
      L"The quick|JJ red|JJ fox|NN jumped|VB over the lazy|JJ brown|JJ dogs|NN";
  shared_ptr<DelimitedPayloadTokenFilter> filter =
      make_shared<DelimitedPayloadTokenFilter>(
          whitespaceMockTokenizer(test),
          DelimitedPayloadTokenFilter::DEFAULT_DELIMITER,
          make_shared<IdentityEncoder>());
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PayloadAttribute> payAtt =
      filter->getAttribute(PayloadAttribute::typeid);
  filter->reset();
  assertTermEquals(L"The", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"quick", filter, termAtt, payAtt,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"red", filter, termAtt, payAtt,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"fox", filter, termAtt, payAtt,
                   (wstring(L"NN")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"jumped", filter, termAtt, payAtt,
                   (wstring(L"VB")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"over", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"the", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"lazy", filter, termAtt, payAtt,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"brown", filter, termAtt, payAtt,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"dogs", filter, termAtt, payAtt,
                   (wstring(L"NN")).getBytes(StandardCharsets::UTF_8));
  assertFalse(filter->incrementToken());
  filter->end();
  delete filter;
}

void DelimitedPayloadTokenFilterTest::testNext() 
{

  wstring test =
      L"The quick|JJ red|JJ fox|NN jumped|VB over the lazy|JJ brown|JJ dogs|NN";
  shared_ptr<DelimitedPayloadTokenFilter> filter =
      make_shared<DelimitedPayloadTokenFilter>(
          whitespaceMockTokenizer(test),
          DelimitedPayloadTokenFilter::DEFAULT_DELIMITER,
          make_shared<IdentityEncoder>());
  filter->reset();
  assertTermEquals(L"The", filter, nullptr);
  assertTermEquals(L"quick", filter,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"red", filter,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"fox", filter,
                   (wstring(L"NN")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"jumped", filter,
                   (wstring(L"VB")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"over", filter, nullptr);
  assertTermEquals(L"the", filter, nullptr);
  assertTermEquals(L"lazy", filter,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"brown", filter,
                   (wstring(L"JJ")).getBytes(StandardCharsets::UTF_8));
  assertTermEquals(L"dogs", filter,
                   (wstring(L"NN")).getBytes(StandardCharsets::UTF_8));
  assertFalse(filter->incrementToken());
  filter->end();
  delete filter;
}

void DelimitedPayloadTokenFilterTest::testFloatEncoding() 
{
  wstring test = L"The quick|1.0 red|2.0 fox|3.5 jumped|0.5 over the lazy|5 "
                 L"brown|99.3 dogs|83.7";
  shared_ptr<DelimitedPayloadTokenFilter> filter =
      make_shared<DelimitedPayloadTokenFilter>(
          whitespaceMockTokenizer(test), L'|', make_shared<FloatEncoder>());
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PayloadAttribute> payAtt =
      filter->getAttribute(PayloadAttribute::typeid);
  filter->reset();
  assertTermEquals(L"The", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"quick", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(1.0f));
  assertTermEquals(L"red", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(2.0f));
  assertTermEquals(L"fox", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(3.5f));
  assertTermEquals(L"jumped", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(0.5f));
  assertTermEquals(L"over", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"the", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"lazy", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(5.0f));
  assertTermEquals(L"brown", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(99.3f));
  assertTermEquals(L"dogs", filter, termAtt, payAtt,
                   PayloadHelper::encodeFloat(83.7f));
  assertFalse(filter->incrementToken());
  filter->end();
  delete filter;
}

void DelimitedPayloadTokenFilterTest::testIntEncoding() 
{
  wstring test =
      L"The quick|1 red|2 fox|3 jumped over the lazy|5 brown|99 dogs|83";
  shared_ptr<DelimitedPayloadTokenFilter> filter =
      make_shared<DelimitedPayloadTokenFilter>(
          whitespaceMockTokenizer(test), L'|', make_shared<IntegerEncoder>());
  shared_ptr<CharTermAttribute> termAtt =
      filter->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PayloadAttribute> payAtt =
      filter->getAttribute(PayloadAttribute::typeid);
  filter->reset();
  assertTermEquals(L"The", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"quick", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(1));
  assertTermEquals(L"red", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(2));
  assertTermEquals(L"fox", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(3));
  assertTermEquals(L"jumped", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"over", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"the", filter, termAtt, payAtt, nullptr);
  assertTermEquals(L"lazy", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(5));
  assertTermEquals(L"brown", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(99));
  assertTermEquals(L"dogs", filter, termAtt, payAtt,
                   PayloadHelper::encodeInt(83));
  assertFalse(filter->incrementToken());
  filter->end();
  delete filter;
}

void DelimitedPayloadTokenFilterTest::assertTermEquals(
    const wstring &expected, shared_ptr<TokenStream> stream,
    std::deque<char> &expectPay) 
{
  shared_ptr<CharTermAttribute> termAtt =
      stream->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PayloadAttribute> payloadAtt =
      stream->getAttribute(PayloadAttribute::typeid);
  assertTrue(stream->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expected, termAtt->toString());
  shared_ptr<BytesRef> payload = payloadAtt->getPayload();
  if (payload != nullptr) {
    assertTrue(to_wstring(payload->length) + L" does not equal: " +
                   expectPay.size(),
               payload->length == expectPay.size());
    for (int i = 0; i < expectPay.size(); i++) {
      assertTrue(to_wstring(expectPay[i]) + L" does not equal: " +
                     to_wstring(payload->bytes[i + payload->offset]),
                 expectPay[i] == payload->bytes[i + payload->offset]);
    }
  } else {
    assertTrue(L"expectPay is not null and it should be", expectPay.empty());
  }
}

void DelimitedPayloadTokenFilterTest::assertTermEquals(
    const wstring &expected, shared_ptr<TokenStream> stream,
    shared_ptr<CharTermAttribute> termAtt, shared_ptr<PayloadAttribute> payAtt,
    std::deque<char> &expectPay) 
{
  assertTrue(stream->incrementToken());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expected, termAtt->toString());
  shared_ptr<BytesRef> payload = payAtt->getPayload();
  if (payload != nullptr) {
    assertTrue(to_wstring(payload->length) + L" does not equal: " +
                   expectPay.size(),
               payload->length == expectPay.size());
    for (int i = 0; i < expectPay.size(); i++) {
      assertTrue(to_wstring(expectPay[i]) + L" does not equal: " +
                     to_wstring(payload->bytes[i + payload->offset]),
                 expectPay[i] == payload->bytes[i + payload->offset]);
    }
  } else {
    assertTrue(L"expectPay is not null and it should be", expectPay.empty());
  }
}
} // namespace org::apache::lucene::analysis::payloads