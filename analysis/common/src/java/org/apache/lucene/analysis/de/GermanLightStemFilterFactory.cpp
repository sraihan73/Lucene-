using namespace std;

#include "GermanLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GermanLightStemFilter.h"

namespace org::apache::lucene::analysis::de
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GermanLightStemFilter =
    org::apache::lucene::analysis::de::GermanLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GermanLightStemFilterFactory::GermanLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GermanLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GermanLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::de