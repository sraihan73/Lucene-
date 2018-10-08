using namespace std;

#include "KoreanPartOfSpeechStopFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "KoreanPartOfSpeechStopFilter.h"

namespace org::apache::lucene::analysis::ko
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

KoreanPartOfSpeechStopFilterFactory::KoreanPartOfSpeechStopFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  shared_ptr<Set<wstring>> stopTagStr = getSet(args, L"tags");
  if (stopTagStr == nullptr) {
    stopTags = KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS;
  } else {
    stopTags =
        stopTagStr->stream().map_obj(POS::resolveTag).collect(Collectors::toSet());
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
KoreanPartOfSpeechStopFilterFactory::create(shared_ptr<TokenStream> stream)
{
  return make_shared<KoreanPartOfSpeechStopFilter>(stream, stopTags);
}
} // namespace org::apache::lucene::analysis::ko