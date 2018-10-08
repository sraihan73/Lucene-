using namespace std;

#include "TypeAsSynonymFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "TypeAsSynonymFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TypeAsSynonymFilterFactory::TypeAsSynonymFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      prefix(get(args, L"prefix")) / *default value is null * /
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
TypeAsSynonymFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<TypeAsSynonymFilter>(input, prefix);
}
} // namespace org::apache::lucene::analysis::miscellaneous