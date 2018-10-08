using namespace std;

#include "HyphenatedWordsFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "HyphenatedWordsFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HyphenatedWordsFilter =
    org::apache::lucene::analysis::miscellaneous::HyphenatedWordsFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

HyphenatedWordsFilterFactory::HyphenatedWordsFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<HyphenatedWordsFilter>
HyphenatedWordsFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<HyphenatedWordsFilter>(input);
}
} // namespace org::apache::lucene::analysis::miscellaneous