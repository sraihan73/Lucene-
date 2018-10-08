using namespace std;

#include "StandardFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"

namespace org::apache::lucene::analysis::standard
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

StandardFilterFactory::StandardFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<StandardFilter>
StandardFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<StandardFilter>(input);
}
} // namespace org::apache::lucene::analysis::standard