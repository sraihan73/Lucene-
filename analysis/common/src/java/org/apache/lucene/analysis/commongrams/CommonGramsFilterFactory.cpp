using namespace std;

#include "CommonGramsFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../core/StopAnalyzer.h"
#include "CommonGramsFilter.h"

namespace org::apache::lucene::analysis::commongrams
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
using namespace org::apache::lucene::analysis::util;

CommonGramsFilterFactory::CommonGramsFilterFactory(
    unordered_map<wstring, wstring> &args)
    : TokenFilterFactory(args), commonWordFiles(get(args, L"words")),
      format(get(args, L"format")),
      ignoreCase(getBoolean(args, L"ignoreCase", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void CommonGramsFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (commonWordFiles != L"") {
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    if ((wstring(L"snowball")).equalsIgnoreCase(format)) {
      commonWords = getSnowballWordSet(loader, commonWordFiles, ignoreCase);
    } else {
      commonWords = getWordSet(loader, commonWordFiles, ignoreCase);
    }
  } else {
    commonWords = StopAnalyzer::ENGLISH_STOP_WORDS_SET;
  }
}

bool CommonGramsFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<CharArraySet> CommonGramsFilterFactory::getCommonWords()
{
  return commonWords;
}

shared_ptr<TokenFilter>
CommonGramsFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<CommonGramsFilter> commonGrams =
      make_shared<CommonGramsFilter>(input, commonWords);
  return commonGrams;
}
} // namespace org::apache::lucene::analysis::commongrams