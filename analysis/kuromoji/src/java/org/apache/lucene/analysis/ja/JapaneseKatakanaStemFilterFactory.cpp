using namespace std;

#include "JapaneseKatakanaStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "JapaneseKatakanaStemFilter.h"

namespace org::apache::lucene::analysis::ja
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using JapaneseKatakanaStemFilter =
    org::apache::lucene::analysis::ja::JapaneseKatakanaStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring JapaneseKatakanaStemFilterFactory::MINIMUM_LENGTH_PARAM =
    L"minimumLength";

JapaneseKatakanaStemFilterFactory::JapaneseKatakanaStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      minimumLength(getInt(args, MINIMUM_LENGTH_PARAM,
                           JapaneseKatakanaStemFilter::DEFAULT_MINIMUM_LENGTH))
{
  if (minimumLength < 2) {
    throw invalid_argument(L"Illegal " + MINIMUM_LENGTH_PARAM + L" " +
                           to_wstring(minimumLength) +
                           L" (must be 2 or greater)");
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
JapaneseKatakanaStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<JapaneseKatakanaStemFilter>(input, minimumLength);
}
} // namespace org::apache::lucene::analysis::ja