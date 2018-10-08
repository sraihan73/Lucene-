using namespace std;

#include "TypeAsPayloadTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "TypeAsPayloadTokenFilter.h"

namespace org::apache::lucene::analysis::payloads
{
using TypeAsPayloadTokenFilter =
    org::apache::lucene::analysis::payloads::TypeAsPayloadTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TypeAsPayloadTokenFilterFactory::TypeAsPayloadTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TypeAsPayloadTokenFilter>
TypeAsPayloadTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TypeAsPayloadTokenFilter>(input);
}
} // namespace org::apache::lucene::analysis::payloads