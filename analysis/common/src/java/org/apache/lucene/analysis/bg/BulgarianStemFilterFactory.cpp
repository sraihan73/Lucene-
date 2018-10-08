using namespace std;

#include "BulgarianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "BulgarianStemFilter.h"

namespace org::apache::lucene::analysis::bg
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BulgarianStemFilter =
    org::apache::lucene::analysis::bg::BulgarianStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

BulgarianStemFilterFactory::BulgarianStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
BulgarianStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<BulgarianStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::bg