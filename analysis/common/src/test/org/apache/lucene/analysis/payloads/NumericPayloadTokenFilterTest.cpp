using namespace std;

#include "NumericPayloadTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/NumericPayloadTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

void NumericPayloadTokenFilterTest::test() 
{
  wstring test = L"The quick red fox jumped over the lazy brown dogs";

  shared_ptr<MockTokenizer> *const input =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  input->setReader(make_shared<StringReader>(test));
  shared_ptr<NumericPayloadTokenFilter> nptf =
      make_shared<NumericPayloadTokenFilter>(
          make_shared<WordTokenFilter>(input), 3, L"D");
  bool seenDogs = false;
  shared_ptr<CharTermAttribute> termAtt =
      nptf->getAttribute(CharTermAttribute::typeid);
  shared_ptr<TypeAttribute> typeAtt = nptf->getAttribute(TypeAttribute::typeid);
  shared_ptr<PayloadAttribute> payloadAtt =
      nptf->getAttribute(PayloadAttribute::typeid);
  nptf->reset();
  while (nptf->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (termAtt->toString()->equals(L"dogs")) {
      seenDogs = true;
      assertTrue(typeAtt->type() + L" is not equal to " + L"D",
                 typeAtt->type() == L"D" == true);
      assertTrue(L"payloadAtt.getPayload() is null and it shouldn't be",
                 payloadAtt->getPayload() != nullptr);
      std::deque<char> bytes =
          payloadAtt->getPayload()
              ->bytes; // safe here to just use the bytes, otherwise we should
                       // use offset, length
      assertTrue(bytes.size() + L" does not equal: " +
                     to_wstring(payloadAtt->getPayload()->length),
                 bytes.size() == payloadAtt->getPayload()->length);
      assertTrue(to_wstring(payloadAtt->getPayload()->offset) +
                     L" does not equal: " + to_wstring(0),
                 payloadAtt->getPayload()->offset == 0);
      float pay = PayloadHelper::decodeFloat(bytes);
      assertTrue(to_wstring(pay) + L" does not equal: " + to_wstring(3),
                 pay == 3);
    } else {
      assertTrue(typeAtt->type() + L" is not null and it should be",
                 typeAtt->type() == L"word");
    }
  }
  assertTrue(StringHelper::toString(seenDogs) + L" does not equal: " +
                 StringHelper::toString(true),
             seenDogs == true);
}

NumericPayloadTokenFilterTest::WordTokenFilter::WordTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool NumericPayloadTokenFilterTest::WordTokenFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (termAtt->toString()->equals(L"dogs")) {
      typeAtt->setType(L"D");
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads