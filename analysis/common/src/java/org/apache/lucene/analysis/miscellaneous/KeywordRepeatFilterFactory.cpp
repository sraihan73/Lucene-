using namespace std;

#include "KeywordRepeatFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "KeywordRepeatFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

KeywordRepeatFilterFactory::KeywordRepeatFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
KeywordRepeatFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<KeywordRepeatFilter>(input);
}
} // namespace org::apache::lucene::analysis::miscellaneous