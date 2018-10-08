using namespace std;

#include "FieldFragList.h"

namespace org::apache::lucene::search::vectorhighlight
{
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;
using Toffs = org::apache::lucene::search::vectorhighlight::FieldPhraseList::
    WeightedPhraseInfo::Toffs;

FieldFragList::FieldFragList(int fragCharSize) {}

deque<std::shared_ptr<WeightedFragInfo>> FieldFragList::getFragInfos()
{
  return fragInfos;
}

FieldFragList::WeightedFragInfo::WeightedFragInfo(
    int startOffset, int endOffset, deque<std::shared_ptr<SubInfo>> &subInfos,
    float totalBoost)
{
  this->startOffset = startOffset;
  this->endOffset = endOffset;
  this->totalBoost = totalBoost;
  this->subInfos = subInfos;
}

deque<std::shared_ptr<SubInfo>> FieldFragList::WeightedFragInfo::getSubInfos()
{
  return subInfos;
}

float FieldFragList::WeightedFragInfo::getTotalBoost() { return totalBoost; }

int FieldFragList::WeightedFragInfo::getStartOffset() { return startOffset; }

int FieldFragList::WeightedFragInfo::getEndOffset() { return endOffset; }

wstring FieldFragList::WeightedFragInfo::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"subInfos=(");
  for (auto si : subInfos) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(si->toString());
  }
  sb->append(L")/")
      ->append(totalBoost)
      ->append(L'(')
      ->append(startOffset)
      ->append(L',')
      ->append(endOffset)
      ->append(L')');
  return sb->toString();
}

FieldFragList::WeightedFragInfo::SubInfo::SubInfo(
    const wstring &text,
    deque<std::shared_ptr<WeightedPhraseInfo::Toffs>> &termsOffsets,
    int seqnum, float boost)
    : text(text), termsOffsets(termsOffsets), seqnum(seqnum), boost(boost)
{
}

deque<std::shared_ptr<WeightedPhraseInfo::Toffs>>
FieldFragList::WeightedFragInfo::SubInfo::getTermsOffsets()
{
  return termsOffsets;
}

int FieldFragList::WeightedFragInfo::SubInfo::getSeqnum() { return seqnum; }

wstring FieldFragList::WeightedFragInfo::SubInfo::getText() { return text; }

float FieldFragList::WeightedFragInfo::SubInfo::getBoost() { return boost; }

wstring FieldFragList::WeightedFragInfo::SubInfo::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(text)->append(L'(');
  for (auto to : termsOffsets) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(to->toString());
  }
  sb->append(L')');
  return sb->toString();
}
} // namespace org::apache::lucene::search::vectorhighlight