using namespace std;

#include "JapaneseReadingFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "JapaneseReadingFormFilter.h"

namespace org::apache::lucene::analysis::ja
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using JapaneseReadingFormFilter =
    org::apache::lucene::analysis::ja::JapaneseReadingFormFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring JapaneseReadingFormFilterFactory::ROMAJI_PARAM = L"useRomaji";

JapaneseReadingFormFilterFactory::JapaneseReadingFormFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      useRomaji(getBoolean(args, ROMAJI_PARAM, false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
JapaneseReadingFormFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<JapaneseReadingFormFilter>(input, useRomaji);
}
} // namespace org::apache::lucene::analysis::ja