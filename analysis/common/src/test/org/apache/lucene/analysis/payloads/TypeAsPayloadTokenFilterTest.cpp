using namespace std;

#include "TypeAsPayloadTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/TypeAsPayloadTokenFilter.h"

namespace org::apache::lucene::analysis::payloads
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

void TypeAsPayloadTokenFilterTest::test() 
{
  wstring test = L"The quick red fox jumped over the lazy brown dogs";

  shared_ptr<TypeAsPayloadTokenFilter> nptf =
      make_shared<TypeAsPayloadTokenFilter>(
          make_shared<WordTokenFilter>(whitespaceMockTokenizer(test)));
  int count = 0;
  shared_ptr<CharTermAttribute> termAtt =
      nptf->getAttribute(CharTermAttribute::typeid);
  shared_ptr<TypeAttribute> typeAtt = nptf->getAttribute(TypeAttribute::typeid);
  shared_ptr<PayloadAttribute> payloadAtt =
      nptf->getAttribute(PayloadAttribute::typeid);
  nptf->reset();
  while (nptf->incrementToken()) {
    assertTrue(typeAtt->type() + L" is not null and it should be",
               typeAtt->type() ==
                   wstring::valueOf(towupper(termAtt->buffer()[0])));
    assertTrue(L"nextToken.getPayload() is null and it shouldn't be",
               payloadAtt->getPayload() != nullptr);
    wstring type = payloadAtt->getPayload()->utf8ToString();
    assertTrue(type + L" is not equal to " + typeAtt->type(),
               type == typeAtt->type());
    count++;
  }

  assertTrue(to_wstring(count) + L" does not equal: " + to_wstring(10),
             count == 10);
}

TypeAsPayloadTokenFilterTest::WordTokenFilter::WordTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TypeAsPayloadTokenFilterTest::WordTokenFilter::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    typeAtt->setType(wstring::valueOf(towupper(termAtt->buffer()[0])));
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads