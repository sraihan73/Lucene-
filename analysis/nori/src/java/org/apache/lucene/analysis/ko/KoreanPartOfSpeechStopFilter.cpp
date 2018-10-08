using namespace std;

#include "KoreanPartOfSpeechStopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "tokenattributes/PartOfSpeechAttribute.h"

namespace org::apache::lucene::analysis::ko
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::PartOfSpeechAttribute;
const shared_ptr<java::util::Set<POS::Tag>>
    KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS =
        java::util::Arrays::asList(POS::Tag::E, POS::Tag::IC, POS::Tag::J,
                                   POS::Tag::MAG, POS::Tag::MAJ, POS::Tag::MM,
                                   POS::Tag::SP, POS::Tag::SSC, POS::Tag::SSO,
                                   POS::Tag::SC, POS::Tag::SE, POS::Tag::XPN,
                                   POS::Tag::XSA, POS::Tag::XSN, POS::Tag::XSV,
                                   POS::Tag::UNA, POS::Tag::NA, POS::Tag::VSV)
            .stream()
            .collect(java::util::stream::Collectors::toSet());

KoreanPartOfSpeechStopFilter::KoreanPartOfSpeechStopFilter(
    shared_ptr<TokenStream> input)
    : KoreanPartOfSpeechStopFilter(input, DEFAULT_STOP_TAGS)
{
}

KoreanPartOfSpeechStopFilter::KoreanPartOfSpeechStopFilter(
    shared_ptr<TokenStream> input, shared_ptr<Set<POS::Tag>> stopTags)
    : org::apache::lucene::analysis::FilteringTokenFilter(input),
      stopTags(stopTags)
{
}

bool KoreanPartOfSpeechStopFilter::accept()
{
  constexpr POS::Tag leftPOS = posAtt->getLeftPOS();
  return leftPOS == nullptr || !stopTags->contains(leftPOS);
}
} // namespace org::apache::lucene::analysis::ko