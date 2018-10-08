using namespace std;

#include "WordSegmenter.h"
#include "WordType.h"
#include "hhmm/HHMMSegmenter.h"
#include "hhmm/SegToken.h"
#include "hhmm/SegTokenFilter.h"

namespace org::apache::lucene::analysis::cn::smart
{
using HHMMSegmenter =
    org::apache::lucene::analysis::cn::smart::hhmm::HHMMSegmenter;
using SegToken = org::apache::lucene::analysis::cn::smart::hhmm::SegToken;
using SegTokenFilter =
    org::apache::lucene::analysis::cn::smart::hhmm::SegTokenFilter;

deque<std::shared_ptr<SegToken>>
WordSegmenter::segmentSentence(const wstring &sentence, int startOffset)
{

  deque<std::shared_ptr<SegToken>> segTokenList =
      hhmmSegmenter->process(sentence);
  // tokens from sentence, excluding WordType.SENTENCE_BEGIN and
  // WordType.SENTENCE_END
  deque<std::shared_ptr<SegToken>> result = Collections::emptyList();

  if (segTokenList.size() > 2) // if it's not an empty sentence
  {
    result = segTokenList.subList(1, segTokenList.size() - 1);
  }

  for (auto st : result) {
    convertSegToken(st, sentence, startOffset);
  }

  return result;
}

shared_ptr<SegToken> WordSegmenter::convertSegToken(shared_ptr<SegToken> st,
                                                    const wstring &sentence,
                                                    int sentenceStartOffset)
{

  switch (st->wordType) {
  case WordType::STRING:
  case WordType::NUMBER:
  case WordType::FULLWIDTH_NUMBER:
  case WordType::FULLWIDTH_STRING:
    st->charArray =
        sentence.substr(st->startOffset, st->endOffset - st->startOffset)
            ->toCharArray();
    break;
  default:
    break;
  }

  st = tokenFilter->filter(st);
  st->startOffset += sentenceStartOffset;
  st->endOffset += sentenceStartOffset;
  return st;
}
} // namespace org::apache::lucene::analysis::cn::smart