using namespace std;

#include "BreakIteratorWrapper.h"
#include "CharArrayIterator.h"
#include "ICUTokenizerConfig.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UProperty;
using com::ibm::icu::text::BreakIterator;
using com::ibm::icu::text::RuleBasedBreakIterator;
using com::ibm::icu::text::UnicodeSet;
using com::ibm::icu::text::UTF16;

BreakIteratorWrapper::BreakIteratorWrapper(
    shared_ptr<RuleBasedBreakIterator> rbbi)
    : rbbi(rbbi)
{
}

int BreakIteratorWrapper::current() { return rbbi->current(); }

int BreakIteratorWrapper::getRuleStatus() { return status; }

int BreakIteratorWrapper::next()
{
  int current = rbbi->current();
  int next = rbbi->next();
  status = calcStatus(current, next);
  return next;
}

int BreakIteratorWrapper::calcStatus(int current, int next)
{
  // to support presentation selectors, we need to handle alphanum, num, and
  // none at least, so currently not worth optimizing.
  // https://unicode.org/cldr/utility/deque-unicodeset.jsp?a=%5B%3AEmoji%3A%5D-%5B%3AEmoji_Presentation%3A%5D&g=Word_Break&i=
  if (next != BreakIterator::DONE && isEmoji(current, next)) {
    return ICUTokenizerConfig::EMOJI_SEQUENCE_STATUS;
  } else {
    return rbbi->getRuleStatus();
  }
}

const shared_ptr<com::ibm::icu::text::UnicodeSet>
    BreakIteratorWrapper::EMOJI_RK =
        (make_shared<com::ibm::icu::text::UnicodeSet>(
             L"[\u002a\u00230-9©®™〰〽]"))
            ->freeze();

bool BreakIteratorWrapper::isEmoji(int current, int next)
{
  int begin = start + current;
  int end = start + next;
  int codepoint = UTF16->charAt(text, 0, end, begin);
  // TODO: this can be made more aggressive and future-proof if it uses
  // [:Extended_Pictographic:]
  if (UCharacter::hasBinaryProperty(codepoint, UProperty::EMOJI)) {
    if (EMOJI_RK->contains(codepoint)) {
      // if its in EmojiRK, we don't treat it as emoji unless there is evidence
      // it forms emoji sequence, an emoji presentation selector or keycap
      // follows.
      int trailer = begin + Character::charCount(codepoint);
      return trailer < end &&
             (text[trailer] == 0xFE0F || text[trailer] == 0x20E3);
    } else {
      return true;
    }
  }
  return false;
}

void BreakIteratorWrapper::setText(std::deque<wchar_t> &text, int start,
                                   int length)
{
  this->text = text;
  this->start = start;
  textIterator->setText(text, start, length);
  rbbi->setText(textIterator);
  status = RuleBasedBreakIterator::WORD_NONE;
}
} // namespace org::apache::lucene::analysis::icu::segmentation