using namespace std;

#include "BengaliStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "BengaliStemFilter.h"

namespace org::apache::lucene::analysis::bn
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

BengaliStemFilterFactory::BengaliStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
BengaliStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<BengaliStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::bn