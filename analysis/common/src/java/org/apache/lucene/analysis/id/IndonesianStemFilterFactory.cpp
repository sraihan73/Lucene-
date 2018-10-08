using namespace std;

#include "IndonesianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "IndonesianStemFilter.h"

namespace org::apache::lucene::analysis::id
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using IndonesianStemFilter =
    org::apache::lucene::analysis::id::IndonesianStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

IndonesianStemFilterFactory::IndonesianStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      stemDerivational(getBoolean(args, L"stemDerivational", true))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
IndonesianStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<IndonesianStemFilter>(input, stemDerivational);
}
} // namespace org::apache::lucene::analysis::id