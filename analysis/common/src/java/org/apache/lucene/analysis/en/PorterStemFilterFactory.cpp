using namespace std;

#include "PorterStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "PorterStemFilter.h"

namespace org::apache::lucene::analysis::en
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PorterStemFilter = org::apache::lucene::analysis::en::PorterStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

PorterStemFilterFactory::PorterStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<PorterStemFilter>
PorterStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PorterStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::en