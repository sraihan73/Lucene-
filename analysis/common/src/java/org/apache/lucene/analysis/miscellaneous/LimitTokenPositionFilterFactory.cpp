using namespace std;

#include "LimitTokenPositionFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LimitTokenPositionFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring LimitTokenPositionFilterFactory::MAX_TOKEN_POSITION_KEY =
    L"maxTokenPosition";
const wstring LimitTokenPositionFilterFactory::CONSUME_ALL_TOKENS_KEY =
    L"consumeAllTokens";

LimitTokenPositionFilterFactory::LimitTokenPositionFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      maxTokenPosition(requireInt(args, MAX_TOKEN_POSITION_KEY)),
      consumeAllTokens(getBoolean(args, CONSUME_ALL_TOKENS_KEY, false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
LimitTokenPositionFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<LimitTokenPositionFilter>(input, maxTokenPosition,
                                               consumeAllTokens);
}
} // namespace org::apache::lucene::analysis::miscellaneous