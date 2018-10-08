using namespace std;

#include "MockFixedLengthPayloadFilter.h"

namespace org::apache::lucene::analysis
{
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

MockFixedLengthPayloadFilter::MockFixedLengthPayloadFilter(
    shared_ptr<Random> random, shared_ptr<TokenStream> in_, int length)
    : TokenFilter(in_), random(random), bytes(std::deque<char>(length)),
      payload(make_shared<BytesRef>(bytes))
{
  if (length < 0) {
    throw invalid_argument(L"length must be >= 0");
  }
}

bool MockFixedLengthPayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    random->nextBytes(bytes);
    payloadAtt->setPayload(payload);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis