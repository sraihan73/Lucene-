using namespace std;

#include "CzechStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "CzechStemFilter.h"

namespace org::apache::lucene::analysis::cz
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CzechStemFilter = org::apache::lucene::analysis::cz::CzechStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

CzechStemFilterFactory::CzechStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
CzechStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<CzechStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::cz