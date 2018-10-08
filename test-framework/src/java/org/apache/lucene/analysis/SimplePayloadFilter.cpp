using namespace std;

#include "SimplePayloadFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

SimplePayloadFilter::SimplePayloadFilter(shared_ptr<TokenStream> input)
    : TokenFilter(input),
      payloadAttr(input->addAttribute(PayloadAttribute::typeid)),
      termAttr(input->addAttribute(CharTermAttribute::typeid))
{
  pos = 0;
}

bool SimplePayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    payloadAttr->setPayload(make_shared<BytesRef>(
        (L"pos: " + to_wstring(pos))->getBytes(StandardCharsets::UTF_8)));
    pos++;
    return true;
  } else {
    return false;
  }
}

void SimplePayloadFilter::reset() 
{
  TokenFilter::reset();
  pos = 0;
}
} // namespace org::apache::lucene::analysis