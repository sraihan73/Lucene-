using namespace std;

#include "WeightedFieldFragList.h"

namespace org::apache::lucene::search::vectorhighlight
{
using SubInfo = org::apache::lucene::search::vectorhighlight::FieldFragList::
    WeightedFragInfo::SubInfo;
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;

WeightedFieldFragList::WeightedFieldFragList(int fragCharSize)
    : FieldFragList(fragCharSize)
{
}

void WeightedFieldFragList::add(
    int startOffset, int endOffset,
    deque<std::shared_ptr<WeightedPhraseInfo>> &phraseInfoList)
{
  deque<std::shared_ptr<SubInfo>> tempSubInfos =
      deque<std::shared_ptr<SubInfo>>();
  deque<std::shared_ptr<SubInfo>> realSubInfos =
      deque<std::shared_ptr<SubInfo>>();
  unordered_set<wstring> distinctTerms = unordered_set<wstring>();
  int length = 0;

  for (auto phraseInfo : phraseInfoList) {
    float phraseTotalBoost = 0;
    for (auto ti : phraseInfo->getTermsInfos()) {
      if (distinctTerms.insert(ti->getText())) {
        phraseTotalBoost += ti->getWeight() * phraseInfo->getBoost();
      }
      length++;
    }
    tempSubInfos.push_back(make_shared<SubInfo>(
        phraseInfo->getText(), phraseInfo->getTermsOffsets(),
        phraseInfo->getSeqnum(), phraseTotalBoost));
  }

  // We want that terms per fragment (length) is included into the weight.
  // Otherwise a one-word-query would cause an equal weight for all fragments
  // regardless of how much words they contain. To avoid that fragments
  // containing a high number of words possibly "outrank" more relevant
  // fragments we "bend" the length with a standard-normalization a little bit.
  float norm = length * (1 / static_cast<float>(sqrt(length)));

  float totalBoost = 0;
  for (auto tempSubInfo : tempSubInfos) {
    float subInfoBoost = tempSubInfo->getBoost() * norm;
    realSubInfos.push_back(make_shared<SubInfo>(
        tempSubInfo->getText(), tempSubInfo->getTermsOffsets(),
        tempSubInfo->getSeqnum(), subInfoBoost));
    totalBoost += subInfoBoost;
  }

  getFragInfos().push_back(make_shared<WeightedFragInfo>(
      startOffset, endOffset, realSubInfos, totalBoost));
}
} // namespace org::apache::lucene::search::vectorhighlight