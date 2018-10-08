using namespace std;

#include "GermanMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GermanMinimalStemFilter.h"

namespace org::apache::lucene::analysis::de
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GermanMinimalStemFilter =
    org::apache::lucene::analysis::de::GermanMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GermanMinimalStemFilterFactory::GermanMinimalStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GermanMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GermanMinimalStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::de