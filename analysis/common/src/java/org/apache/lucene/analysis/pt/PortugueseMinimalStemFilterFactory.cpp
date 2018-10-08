using namespace std;

#include "PortugueseMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PortugueseMinimalStemFilter.h"

namespace org::apache::lucene::analysis::pt
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PortugueseMinimalStemFilter =
    org::apache::lucene::analysis::pt::PortugueseMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PortugueseMinimalStemFilterFactory::PortugueseMinimalStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
PortugueseMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PortugueseMinimalStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::pt