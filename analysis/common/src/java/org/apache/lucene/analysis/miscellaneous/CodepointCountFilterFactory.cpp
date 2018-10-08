using namespace std;

#include "CodepointCountFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "CodepointCountFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring CodepointCountFilterFactory::MIN_KEY = L"min";
const wstring CodepointCountFilterFactory::MAX_KEY = L"max";

CodepointCountFilterFactory::CodepointCountFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      min(requireInt(args, MIN_KEY)), max(requireInt(args, MAX_KEY))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<CodepointCountFilter>
CodepointCountFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<CodepointCountFilter>(input, min, max);
}
} // namespace org::apache::lucene::analysis::miscellaneous