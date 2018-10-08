using namespace std;

#include "SoraniStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "SoraniStemFilter.h"

namespace org::apache::lucene::analysis::ckb
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

SoraniStemFilterFactory::SoraniStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<SoraniStemFilter>
SoraniStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<SoraniStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::ckb