using namespace std;

#include "ItalianLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ItalianLightStemFilter.h"

namespace org::apache::lucene::analysis::it
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ItalianLightStemFilter =
    org::apache::lucene::analysis::it::ItalianLightStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ItalianLightStemFilterFactory::ItalianLightStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
ItalianLightStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ItalianLightStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::it