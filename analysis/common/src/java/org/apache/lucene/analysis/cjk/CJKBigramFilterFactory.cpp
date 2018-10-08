using namespace std;

#include "CJKBigramFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "CJKBigramFilter.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CJKBigramFilter = org::apache::lucene::analysis::cjk::CJKBigramFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

CJKBigramFilterFactory::CJKBigramFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      flags(flags), outputUnigrams(getBoolean(args, L"outputUnigrams", false))
{
  int flags = 0;
  if (getBoolean(args, L"han", true)) {
    flags |= CJKBigramFilter::HAN;
  }
  if (getBoolean(args, L"hiragana", true)) {
    flags |= CJKBigramFilter::HIRAGANA;
  }
  if (getBoolean(args, L"katakana", true)) {
    flags |= CJKBigramFilter::KATAKANA;
  }
  if (getBoolean(args, L"hangul", true)) {
    flags |= CJKBigramFilter::HANGUL;
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
CJKBigramFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<CJKBigramFilter>(input, flags, outputUnigrams);
}
} // namespace org::apache::lucene::analysis::cjk