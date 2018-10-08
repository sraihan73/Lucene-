using namespace std;

#include "PatternReplaceFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PatternReplaceFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PatternReplaceFilterFactory::PatternReplaceFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      pattern(getPattern(args, L"pattern")),
      replacement(get(args, L"replacement")),
      replaceAll(L"all" == get(args, L"replace",
                               Arrays::asList(L"all", L"first"), L"all"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<PatternReplaceFilter>
PatternReplaceFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PatternReplaceFilter>(input, pattern, replacement,
                                           replaceAll);
}
} // namespace org::apache::lucene::analysis::pattern