using namespace std;

#include "TypeAsPayloadTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

TypeAsPayloadTokenFilter::TypeAsPayloadTokenFilter(
    shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool TypeAsPayloadTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    wstring type = typeAtt->type();
    if (type != L"" && !type.isEmpty()) {
      payloadAtt->setPayload(make_shared<BytesRef>(type));
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::payloads