using namespace std;

#include "ArabicStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ArabicStemFilter.h"

namespace org::apache::lucene::analysis::ar
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ArabicStemFilter = org::apache::lucene::analysis::ar::ArabicStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ArabicStemFilterFactory::ArabicStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ArabicStemFilter>
ArabicStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ArabicStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::ar