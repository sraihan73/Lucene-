using namespace std;

#include "SpanishLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "SpanishLightStemFilter.h"

namespace org::apache::lucene::analysis::es
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SpanishLightStemFilter =
    org::apache::lucene::analysis::es::SpanishLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

SpanishLightStemFilterFactory::SpanishLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
SpanishLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<SpanishLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::es