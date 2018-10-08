using namespace std;

#include "TruncateTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "TruncateTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring TruncateTokenFilterFactory::PREFIX_LENGTH_KEY = L"prefixLength";

TruncateTokenFilterFactory::TruncateTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      prefixLength(
          StringHelper::fromString<char>(get(args, PREFIX_LENGTH_KEY, L"5")))
{
  if (prefixLength < 1) {
    throw invalid_argument(PREFIX_LENGTH_KEY +
                           L" parameter must be a positive number: " +
                           to_wstring(prefixLength));
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameter(s): " + args);
  }
}

shared_ptr<TokenStream>
TruncateTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TruncateTokenFilter>(input, prefixLength);
}
} // namespace org::apache::lucene::analysis::miscellaneous