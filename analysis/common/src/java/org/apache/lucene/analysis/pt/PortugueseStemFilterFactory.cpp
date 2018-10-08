using namespace std;

#include "PortugueseStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PortugueseStemFilter.h"

namespace org::apache::lucene::analysis::pt
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PortugueseStemFilter =
    org::apache::lucene::analysis::pt::PortugueseStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PortugueseStemFilterFactory::PortugueseStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
PortugueseStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PortugueseStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::pt