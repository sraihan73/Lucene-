using namespace std;

#include "ReverseStringFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ReverseStringFilter.h"

namespace org::apache::lucene::analysis::reverse
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ReverseStringFilter =
    org::apache::lucene::analysis::reverse::ReverseStringFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ReverseStringFilterFactory::ReverseStringFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<ReverseStringFilter>
ReverseStringFilterFactory::create(shared_ptr<TokenStream> in_)
{
  return make_shared<ReverseStringFilter>(in_);
}
} // namespace org::apache::lucene::analysis::reverse