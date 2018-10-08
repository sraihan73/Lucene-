using namespace std;

#include "LimitTokenCountFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LimitTokenCountFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring LimitTokenCountFilterFactory::MAX_TOKEN_COUNT_KEY =
    L"maxTokenCount";
const wstring LimitTokenCountFilterFactory::CONSUME_ALL_TOKENS_KEY =
    L"consumeAllTokens";

LimitTokenCountFilterFactory::LimitTokenCountFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      maxTokenCount(requireInt(args, MAX_TOKEN_COUNT_KEY)),
      consumeAllTokens(getBoolean(args, CONSUME_ALL_TOKENS_KEY, false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
LimitTokenCountFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<LimitTokenCountFilter>(input, maxTokenCount,
                                            consumeAllTokens);
}
} // namespace org::apache::lucene::analysis::miscellaneous