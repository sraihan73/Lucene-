using namespace std;

#include "KStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "KStemFilter.h"

namespace org::apache::lucene::analysis::en
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KStemFilter = org::apache::lucene::analysis::en::KStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

KStemFilterFactory::KStemFilterFactory(unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenFilter>
KStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<KStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::en