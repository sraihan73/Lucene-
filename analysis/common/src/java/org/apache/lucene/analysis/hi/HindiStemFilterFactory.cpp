using namespace std;

#include "HindiStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "HindiStemFilter.h"

namespace org::apache::lucene::analysis::hi
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HindiStemFilter = org::apache::lucene::analysis::hi::HindiStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

HindiStemFilterFactory::HindiStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
HindiStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<HindiStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::hi