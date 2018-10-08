using namespace std;

#include "TokenOffsetPayloadTokenFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/PayloadHelper.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/TokenOffsetPayloadTokenFilter.h"

namespace org::apache::lucene::analysis::payloads
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

void TokenOffsetPayloadTokenFilterTest::test() 
{
  wstring test = L"The quick red fox jumped over the lazy brown dogs";

  shared_ptr<TokenOffsetPayloadTokenFilter> nptf =
      make_shared<TokenOffsetPayloadTokenFilter>(whitespaceMockTokenizer(test));
  int count = 0;
  shared_ptr<PayloadAttribute> payloadAtt =
      nptf->getAttribute(PayloadAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      nptf->getAttribute(OffsetAttribute::typeid);
  nptf->reset();
  while (nptf->incrementToken()) {
    shared_ptr<BytesRef> pay = payloadAtt->getPayload();
    assertTrue(L"pay is null and it shouldn't be", pay != nullptr);
    std::deque<char> data = pay->bytes;
    int start = PayloadHelper::decodeInt(data, 0);
    assertTrue(to_wstring(start) + L" does not equal: " +
                   to_wstring(offsetAtt->startOffset()),
               start == offsetAtt->startOffset());
    int end = PayloadHelper::decodeInt(data, 4);
    assertTrue(to_wstring(end) + L" does not equal: " +
                   to_wstring(offsetAtt->endOffset()),
               end == offsetAtt->endOffset());
    count++;
  }
  assertTrue(to_wstring(count) + L" does not equal: " + to_wstring(10),
             count == 10);
}
} // namespace org::apache::lucene::analysis::payloads