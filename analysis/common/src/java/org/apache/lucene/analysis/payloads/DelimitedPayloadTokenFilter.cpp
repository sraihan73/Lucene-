using namespace std;

#include "DelimitedPayloadTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "PayloadEncoder.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

DelimitedPayloadTokenFilter::DelimitedPayloadTokenFilter(
    shared_ptr<TokenStream> input, wchar_t delimiter,
    shared_ptr<PayloadEncoder> encoder)
    : org::apache::lucene::analysis::TokenFilter(input), delimiter(delimiter),
      encoder(encoder)
{
}

bool DelimitedPayloadTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    const std::deque<wchar_t> buffer = termAtt->buffer();
    constexpr int length = termAtt->length();
    for (int i = 0; i < length; i++) {
      if (buffer[i] == delimiter) {
        payAtt->setPayload(encoder->encode(buffer, i + 1, (length - (i + 1))));
        termAtt->setLength(i); // simply set a new length
        return true;
      }
    }
    // we have not seen the delimiter
    payAtt->setPayload(nullptr);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads