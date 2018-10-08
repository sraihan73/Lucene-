using namespace std;

#include "FrenchLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FrenchLightStemFilter.h"

namespace org::apache::lucene::analysis::fr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FrenchLightStemFilter =
    org::apache::lucene::analysis::fr::FrenchLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FrenchLightStemFilterFactory::FrenchLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
FrenchLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FrenchLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::fr