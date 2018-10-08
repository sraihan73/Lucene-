using namespace std;

#include "TokenOffsetPayloadTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

TokenOffsetPayloadTokenFilter::TokenOffsetPayloadTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TokenOffsetPayloadTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<char> data(8);
    PayloadHelper::encodeInt(offsetAtt->startOffset(), data, 0);
    PayloadHelper::encodeInt(offsetAtt->endOffset(), data, 4);
    shared_ptr<BytesRef> payload = make_shared<BytesRef>(data);
    payAtt->setPayload(payload);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads