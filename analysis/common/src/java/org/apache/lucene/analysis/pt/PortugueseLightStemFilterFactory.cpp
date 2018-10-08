using namespace std;

#include "PortugueseLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PortugueseLightStemFilter.h"

namespace org::apache::lucene::analysis::pt
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PortugueseLightStemFilter =
    org::apache::lucene::analysis::pt::PortugueseLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PortugueseLightStemFilterFactory::PortugueseLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
PortugueseLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PortugueseLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::pt