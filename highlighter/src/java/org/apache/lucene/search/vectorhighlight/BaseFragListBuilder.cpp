using namespace std;

#include "BaseFragListBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

BaseFragListBuilder::BaseFragListBuilder(int margin)
    : margin(margin),
      minFragCharSize(max(1, margin * MIN_FRAG_CHAR_SIZE_FACTOR))
{
  if (margin < 0) {
    throw invalid_argument(L"margin(" + to_wstring(margin) +
                           L") is too small. It must be 0 or higher.");
  }
}

BaseFragListBuilder::BaseFragListBuilder() : BaseFragListBuilder(MARGIN_DEFAULT)
{
}

shared_ptr<FieldFragList> BaseFragListBuilder::createFieldFragList(
    shared_ptr<FieldPhraseList> fieldPhraseList,
    shared_ptr<FieldFragList> fieldFragList, int fragCharSize)
{
  if (fragCharSize < minFragCharSize) {
    throw invalid_argument(L"fragCharSize(" + to_wstring(fragCharSize) +
                           L") is too small. It must be " +
                           to_wstring(minFragCharSize) + L" or higher.");
  }

  deque<std::shared_ptr<WeightedPhraseInfo>> wpil =
      deque<std::shared_ptr<WeightedPhraseInfo>>();
  shared_ptr<IteratorQueue<std::shared_ptr<WeightedPhraseInfo>>> queue =
      make_shared<IteratorQueue<std::shared_ptr<WeightedPhraseInfo>>>(
          fieldPhraseList->getPhraseList().begin());
  shared_ptr<WeightedPhraseInfo> phraseInfo = nullptr;
  int startOffset = 0;
  while ((phraseInfo = queue->top()) != nullptr) {
    // if the phrase violates the border of previous fragment, discard it and
    // try next phrase
    if (phraseInfo->getStartOffset() < startOffset) {
      queue->removeTop();
      continue;
    }

    wpil.clear();
    constexpr int currentPhraseStartOffset = phraseInfo->getStartOffset();
    int currentPhraseEndOffset = phraseInfo->getEndOffset();
    int spanStart = max(currentPhraseStartOffset - margin, startOffset);
    int spanEnd = max(currentPhraseEndOffset, spanStart + fragCharSize);
    if (acceptPhrase(queue->removeTop(),
                     currentPhraseEndOffset - currentPhraseStartOffset,
                     fragCharSize)) {
      wpil.push_back(phraseInfo);
    }
    while ((phraseInfo = queue->top()) !=
           nullptr) { // pull until we crossed the current spanEnd
      if (phraseInfo->getEndOffset() <= spanEnd) {
        currentPhraseEndOffset = phraseInfo->getEndOffset();
        if (acceptPhrase(queue->removeTop(),
                         currentPhraseEndOffset - currentPhraseStartOffset,
                         fragCharSize)) {
          wpil.push_back(phraseInfo);
        }
      } else {
        break;
      }
    }
    if (wpil.empty()) {
      continue;
    }

    constexpr int matchLen = currentPhraseEndOffset - currentPhraseStartOffset;
    // now recalculate the start and end position to "center" the result
    constexpr int newMargin =
        max(0, (fragCharSize - matchLen) /
                   2); // matchLen can be > fragCharSize prevent IAOOB here
    spanStart = currentPhraseStartOffset - newMargin;
    if (spanStart < startOffset) {
      spanStart = startOffset;
    }
    // whatever is bigger here we grow this out
    spanEnd = spanStart + max(matchLen, fragCharSize);
    startOffset = spanEnd;
    fieldFragList->add(spanStart, spanEnd, wpil);
  }
  return fieldFragList;
}

bool BaseFragListBuilder::acceptPhrase(shared_ptr<WeightedPhraseInfo> info,
                                       int matchLength, int fragCharSize)
{
  return info->getTermsOffsets().size() <= 1 || matchLength <= fragCharSize;
}
} // namespace org::apache::lucene::search::vectorhighlight