using namespace std;

#include "LatvianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "LatvianStemFilter.h"

namespace org::apache::lucene::analysis::lv
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using LatvianStemFilter = org::apache::lucene::analysis::lv::LatvianStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

LatvianStemFilterFactory::LatvianStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
LatvianStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<LatvianStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::lv