using namespace std;

#include "LengthFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LengthFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring LengthFilterFactory::MIN_KEY = L"min";
const wstring LengthFilterFactory::MAX_KEY = L"max";

LengthFilterFactory::LengthFilterFactory(unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      min(requireInt(args, MIN_KEY)), max(requireInt(args, MAX_KEY))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<LengthFilter>
LengthFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<LengthFilter> *const filter =
      make_shared<LengthFilter>(input, min, max);
  return filter;
}
} // namespace org::apache::lucene::analysis::miscellaneous