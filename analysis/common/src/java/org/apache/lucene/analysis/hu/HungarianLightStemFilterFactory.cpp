using namespace std;

#include "HungarianLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "HungarianLightStemFilter.h"

namespace org::apache::lucene::analysis::hu
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HungarianLightStemFilter =
    org::apache::lucene::analysis::hu::HungarianLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

HungarianLightStemFilterFactory::HungarianLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
HungarianLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<HungarianLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::hu