using namespace std;

#include "FinnishLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FinnishLightStemFilter.h"

namespace org::apache::lucene::analysis::fi
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FinnishLightStemFilter =
    org::apache::lucene::analysis::fi::FinnishLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FinnishLightStemFilterFactory::FinnishLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
FinnishLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FinnishLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::fi