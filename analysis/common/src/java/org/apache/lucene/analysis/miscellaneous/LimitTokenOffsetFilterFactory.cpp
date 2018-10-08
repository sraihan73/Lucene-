using namespace std;

#include "LimitTokenOffsetFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LimitTokenOffsetFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring LimitTokenOffsetFilterFactory::MAX_START_OFFSET =
    L"maxStartOffset";
const wstring LimitTokenOffsetFilterFactory::CONSUME_ALL_TOKENS_KEY =
    L"consumeAllTokens";

LimitTokenOffsetFilterFactory::LimitTokenOffsetFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  maxStartOffset = requireInt(args, MAX_START_OFFSET);
  consumeAllTokens = getBoolean(args, CONSUME_ALL_TOKENS_KEY, false);
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
LimitTokenOffsetFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<LimitTokenOffsetFilter>(input, maxStartOffset,
                                             consumeAllTokens);
}
} // namespace org::apache::lucene::analysis::miscellaneous