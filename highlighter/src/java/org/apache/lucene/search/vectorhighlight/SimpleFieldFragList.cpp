using namespace std;

#include "SimpleFieldFragList.h"

namespace org::apache::lucene::search::vectorhighlight
{
using SubInfo = org::apache::lucene::search::vectorhighlight::FieldFragList::
    WeightedFragInfo::SubInfo;
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

SimpleFieldFragList::SimpleFieldFragList(int fragCharSize)
    : FieldFragList(fragCharSize)
{
}

void SimpleFieldFragList::add(
    int startOffset, int endOffset,
    deque<std::shared_ptr<WeightedPhraseInfo>> &phraseInfoList)
{
  float totalBoost = 0;
  deque<std::shared_ptr<SubInfo>> subInfos =
      deque<std::shared_ptr<SubInfo>>();
  for (auto phraseInfo : phraseInfoList) {
    subInfos.push_back(make_shared<SubInfo>(
        phraseInfo->getText(), phraseInfo->getTermsOffsets(),
        phraseInfo->getSeqnum(), phraseInfo->getBoost()));
    totalBoost += phraseInfo->getBoost();
  }
  getFragInfos().push_back(make_shared<WeightedFragInfo>(startOffset, endOffset,
                                                         subInfos, totalBoost));
}
} // namespace org::apache::lucene::search::vectorhighlight