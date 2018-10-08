using namespace std;

#include "NorwegianLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "NorwegianLightStemFilter.h"

namespace org::apache::lucene::analysis::no
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NorwegianLightStemFilter =
    org::apache::lucene::analysis::no::NorwegianLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.BOKMAAL;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.NYNORSK;

NorwegianLightStemFilterFactory::NorwegianLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  wstring variant = get(args, L"variant");
  if (variant == L"" || L"nb" == variant) {
    flags = BOKMAAL;
  } else if (L"nn" == variant) {
    flags = NYNORSK;
  } else if (L"no" == variant) {
    flags = BOKMAAL | NYNORSK;
  } else {
    throw invalid_argument(L"invalid variant: " + variant);
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
NorwegianLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<NorwegianLightStemFilter>(input, flags);
}
} // namespace org::apache::lucene::analysis::no