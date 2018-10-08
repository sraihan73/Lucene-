using namespace std;

#include "JapanesePartOfSpeechStopFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "JapanesePartOfSpeechStopFilter.h"

namespace org::apache::lucene::analysis::ja
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

JapanesePartOfSpeechStopFilterFactory::JapanesePartOfSpeechStopFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      stopTagFiles(get(args, L"tags"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void JapanesePartOfSpeechStopFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  stopTags.reset();
  shared_ptr<CharArraySet> cas = getWordSet(loader, stopTagFiles, false);
  if (cas != nullptr) {
    stopTags = unordered_set<>();
    for (auto element : cas) {
      std::deque<wchar_t> chars = static_cast<std::deque<wchar_t>>(element);
      stopTags->add(wstring(chars));
    }
  }
}

shared_ptr<TokenStream>
JapanesePartOfSpeechStopFilterFactory::create(shared_ptr<TokenStream> stream)
{
  // if stoptags is null, it means the file is empty
  if (stopTags != nullptr) {
    shared_ptr<TokenStream> *const filter =
        make_shared<JapanesePartOfSpeechStopFilter>(stream, stopTags);
    return filter;
  } else {
    return stream;
  }
}
} // namespace org::apache::lucene::analysis::ja