using namespace std;

#include "GermanStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GermanStemFilter.h"

namespace org::apache::lucene::analysis::de
{
using GermanStemFilter = org::apache::lucene::analysis::de::GermanStemFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GermanStemFilterFactory::GermanStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<GermanStemFilter>
GermanStemFilterFactory::create(shared_ptr<TokenStream> in_)
{
  return make_shared<GermanStemFilter>(in_);
}
} // namespace org::apache::lucene::analysis::de