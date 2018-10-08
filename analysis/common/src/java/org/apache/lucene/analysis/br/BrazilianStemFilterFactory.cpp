using namespace std;

#include "BrazilianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "BrazilianStemFilter.h"

namespace org::apache::lucene::analysis::br
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BrazilianStemFilter =
    org::apache::lucene::analysis::br::BrazilianStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

BrazilianStemFilterFactory::BrazilianStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<BrazilianStemFilter>
BrazilianStemFilterFactory::create(shared_ptr<TokenStream> in_)
{
  return make_shared<BrazilianStemFilter>(in_);
}
} // namespace org::apache::lucene::analysis::br