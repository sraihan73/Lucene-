using namespace std;

#include "FrenchMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FrenchMinimalStemFilter.h"

namespace org::apache::lucene::analysis::fr
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FrenchMinimalStemFilter =
    org::apache::lucene::analysis::fr::FrenchMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FrenchMinimalStemFilterFactory::FrenchMinimalStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
FrenchMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FrenchMinimalStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::fr