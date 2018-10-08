using namespace std;

#include "MockVariableLengthPayloadFilter.h"

namespace org::apache::lucene::analysis
{
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

MockVariableLengthPayloadFilter::MockVariableLengthPayloadFilter(
    shared_ptr<Random> random, shared_ptr<TokenStream> in_)
    : TokenFilter(in_), random(random), payload(make_shared<BytesRef>(bytes))
{
}

bool MockVariableLengthPayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    random->nextBytes(bytes);
    payload->length = random->nextInt(MAXLENGTH);
    payloadAtt->setPayload(payload);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis