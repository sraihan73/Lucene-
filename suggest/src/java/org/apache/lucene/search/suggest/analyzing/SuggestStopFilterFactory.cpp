using namespace std;

#include "SuggestStopFilterFactory.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring SuggestStopFilterFactory::FORMAT_WORDSET = L"wordset";
const wstring SuggestStopFilterFactory::FORMAT_SNOWBALL = L"snowball";

SuggestStopFilterFactory::SuggestStopFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      stopWordFiles(get(args, L"words")),
      format(get(args, L"format",
                 (L"" == stopWordFiles ? nullptr : FORMAT_WORDSET))),
      ignoreCase(getBoolean(args, L"ignoreCase", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void SuggestStopFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (stopWordFiles != L"") {
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    if (FORMAT_WORDSET.equalsIgnoreCase(format)) {
      stopWords = getWordSet(loader, stopWordFiles, ignoreCase);
    }
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    else if (FORMAT_SNOWBALL.equalsIgnoreCase(format)) {
      stopWords = getSnowballWordSet(loader, stopWordFiles, ignoreCase);
    } else {
      throw invalid_argument(L"Unknown 'format' specified for 'words' file: " +
                             format);
    }
  } else {
    if (L"" != format) {
      throw invalid_argument(
          L"'format' can not be specified w/o an explicit 'words' file: " +
          format);
    }
    stopWords = make_shared<CharArraySet>(StopAnalyzer::ENGLISH_STOP_WORDS_SET,
                                          ignoreCase);
  }
}

bool SuggestStopFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<CharArraySet> SuggestStopFilterFactory::getStopWords()
{
  return stopWords;
}

shared_ptr<TokenStream>
SuggestStopFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<SuggestStopFilter> suggestStopFilter =
      make_shared<SuggestStopFilter>(input, stopWords);
  return suggestStopFilter;
}
} // namespace org::apache::lucene::search::suggest::analyzing