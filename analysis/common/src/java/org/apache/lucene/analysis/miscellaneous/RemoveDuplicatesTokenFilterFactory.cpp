using namespace std;

#include "RemoveDuplicatesTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "RemoveDuplicatesTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using RemoveDuplicatesTokenFilter =
    org::apache::lucene::analysis::miscellaneous::RemoveDuplicatesTokenFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

RemoveDuplicatesTokenFilterFactory::RemoveDuplicatesTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<RemoveDuplicatesTokenFilter>
RemoveDuplicatesTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<RemoveDuplicatesTokenFilter>(input);
}
} // namespace org::apache::lucene::analysis::miscellaneous