using namespace std;

#include "GreekStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GreekStemFilter.h"

namespace org::apache::lucene::analysis::el
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GreekStemFilter = org::apache::lucene::analysis::el::GreekStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GreekStemFilterFactory::GreekStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GreekStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GreekStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::el