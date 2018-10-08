using namespace std;

#include "TokenOffsetPayloadTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "TokenOffsetPayloadTokenFilter.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenOffsetPayloadTokenFilter =
    org::apache::lucene::analysis::payloads::TokenOffsetPayloadTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TokenOffsetPayloadTokenFilterFactory::TokenOffsetPayloadTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenOffsetPayloadTokenFilter>
TokenOffsetPayloadTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TokenOffsetPayloadTokenFilter>(input);
}
} // namespace org::apache::lucene::analysis::payloads