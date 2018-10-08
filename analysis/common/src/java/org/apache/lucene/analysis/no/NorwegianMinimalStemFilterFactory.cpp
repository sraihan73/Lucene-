using namespace std;

#include "NorwegianMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "NorwegianMinimalStemFilter.h"

namespace org::apache::lucene::analysis::no
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using NorwegianMinimalStemFilter =
    org::apache::lucene::analysis::no::NorwegianMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.BOKMAAL;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.NYNORSK;

NorwegianMinimalStemFilterFactory::NorwegianMinimalStemFilterFactory(
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
NorwegianMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<NorwegianMinimalStemFilter>(input, flags);
}
} // namespace org::apache::lucene::analysis::no