using namespace std;

#include "SwedishLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "SwedishLightStemFilter.h"

namespace org::apache::lucene::analysis::sv
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SwedishLightStemFilter =
    org::apache::lucene::analysis::sv::SwedishLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

SwedishLightStemFilterFactory::SwedishLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
SwedishLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<SwedishLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::sv