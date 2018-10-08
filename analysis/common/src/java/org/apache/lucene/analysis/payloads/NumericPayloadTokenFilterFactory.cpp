using namespace std;

#include "NumericPayloadTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "NumericPayloadTokenFilter.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

NumericPayloadTokenFilterFactory::NumericPayloadTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      payload(requireFloat(args, L"payload")),
      typeMatch(require(args, L"typeMatch"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<NumericPayloadTokenFilter>
NumericPayloadTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<NumericPayloadTokenFilter>(input, payload, typeMatch);
}
} // namespace org::apache::lucene::analysis::payloads