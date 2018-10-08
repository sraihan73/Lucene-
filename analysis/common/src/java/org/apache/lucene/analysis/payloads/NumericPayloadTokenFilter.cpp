using namespace std;

#include "NumericPayloadTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

NumericPayloadTokenFilter::NumericPayloadTokenFilter(
    shared_ptr<TokenStream> input, float payload, const wstring &typeMatch)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  if (typeMatch == L"") {
    throw invalid_argument(L"typeMatch must not be null");
  }
  // Need to encode the payload
  thePayload = make_shared<BytesRef>(PayloadHelper::encodeFloat(payload));
  this->typeMatch = typeMatch;
}

bool NumericPayloadTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (typeAtt->type() == typeMatch) {
      payloadAtt->setPayload(thePayload);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads