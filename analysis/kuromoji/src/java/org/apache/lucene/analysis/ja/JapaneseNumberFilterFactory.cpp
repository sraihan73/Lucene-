using namespace std;

#include "JapaneseNumberFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "JapaneseNumberFilter.h"

namespace org::apache::lucene::analysis::ja
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

JapaneseNumberFilterFactory::JapaneseNumberFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
JapaneseNumberFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<JapaneseNumberFilter>(input);
}
} // namespace org::apache::lucene::analysis::ja