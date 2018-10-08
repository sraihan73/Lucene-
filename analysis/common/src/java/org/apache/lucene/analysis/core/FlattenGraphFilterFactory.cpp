using namespace std;

#include "FlattenGraphFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "FlattenGraphFilter.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

FlattenGraphFilterFactory::FlattenGraphFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
FlattenGraphFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<FlattenGraphFilter>(input);
}
} // namespace org::apache::lucene::analysis::core