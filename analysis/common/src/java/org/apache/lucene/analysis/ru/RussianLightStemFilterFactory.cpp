using namespace std;

#include "RussianLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "RussianLightStemFilter.h"

namespace org::apache::lucene::analysis::ru
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using RussianLightStemFilter =
    org::apache::lucene::analysis::ru::RussianLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

RussianLightStemFilterFactory::RussianLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
RussianLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<RussianLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::ru