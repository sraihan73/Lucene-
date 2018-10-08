using namespace std;

#include "JapanesePartOfSpeechStopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "tokenattributes/PartOfSpeechAttribute.h"

namespace org::apache::lucene::analysis::ja
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ja::tokenattributes::PartOfSpeechAttribute;

JapanesePartOfSpeechStopFilter::JapanesePartOfSpeechStopFilter(
    shared_ptr<TokenStream> input, shared_ptr<Set<wstring>> stopTags)
    : org::apache::lucene::analysis::FilteringTokenFilter(input),
      stopTags(stopTags)
{
}

bool JapanesePartOfSpeechStopFilter::accept()
{
  const wstring pos = posAtt->getPartOfSpeech();
  return pos == L"" || !stopTags->contains(pos);
}
} // namespace org::apache::lucene::analysis::ja