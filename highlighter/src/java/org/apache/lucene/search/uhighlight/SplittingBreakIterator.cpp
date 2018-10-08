using namespace std;

#include "SplittingBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{

SplittingBreakIterator::SplittingBreakIterator(
    shared_ptr<BreakIterator> baseIter, wchar_t sliceChar)
    : baseIter(baseIter), sliceChar(sliceChar)
{
}

void SplittingBreakIterator::setText(shared_ptr<CharacterIterator> newText)
{
  throw make_shared<UnsupportedOperationException>(L"unexpected");
}

void SplittingBreakIterator::setText(const wstring &newText)
{
  this->text = newText;
  first();
}

shared_ptr<CharacterIterator> SplittingBreakIterator::getText()
{
  shared_ptr<StringCharacterIterator> charIter =
      make_shared<StringCharacterIterator>(text);
  // API doesn't say what the state should be but it should probably be at the
  // current index.
  charIter->setIndex(current());
  return charIter;
}

int SplittingBreakIterator::current()
{
  assert(current_ != DONE);
  return current_; // MUST be updated by the other methods when result isn't
                   // DONE.
}

int SplittingBreakIterator::first()
{
  sliceStartIdx = 0;
  sliceEndIdx = (int)text.find(sliceChar);
  if (sliceEndIdx == -1) {
    sliceEndIdx = text.length();
  }
  if (sliceStartIdx == sliceEndIdx) {
    return current_ = sliceStartIdx;
  }
  baseIter->setText(text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
  return current_ = sliceStartIdx +
                    baseIter->current(); // since setText() sets to first(),
                                         // just grab current()
}

int SplittingBreakIterator::last()
{
  sliceEndIdx = text.length();
  sliceStartIdx = (int)text.rfind(sliceChar);
  if (sliceStartIdx == -1) {
    sliceStartIdx = 0;
  } else {
    sliceStartIdx++; // past sliceChar
  }
  if (sliceEndIdx == sliceStartIdx) {
    return current_ = sliceEndIdx;
  }
  baseIter->setText(text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
  return current_ = sliceStartIdx + baseIter->last();
}

int SplittingBreakIterator::next()
{
  int prevCurrent = current_;
  current_ = sliceStartIdx == sliceEndIdx ? DONE : baseIter->next();
  if (current_ != DONE) {
    return current_ = current_ + sliceStartIdx;
  }
  if (sliceEndIdx >= text.length()) {
    current_ = prevCurrent; // keep current where it is
    return DONE;
  }
  sliceStartIdx = sliceEndIdx + 1;
  sliceEndIdx = (int)text.find(sliceChar, sliceStartIdx);
  if (sliceEndIdx == -1) {
    sliceEndIdx = text.length();
  }
  if (sliceStartIdx == sliceEndIdx) {
    return current_ = sliceStartIdx;
  }
  baseIter->setText(text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
  return current_ =
             sliceStartIdx +
             baseIter->current(); // use current() since at first() already
}

int SplittingBreakIterator::previous()
{ // note: closely follows next() but reversed
  int prevCurrent = current_;
  current_ = sliceStartIdx == sliceEndIdx ? DONE : baseIter->previous();
  if (current_ != DONE) {
    return current_ = current_ + sliceStartIdx;
  }
  if (sliceStartIdx == 0) {
    current_ = prevCurrent; // keep current where it is
    return DONE;
  }
  sliceEndIdx = sliceStartIdx - 1;
  sliceStartIdx = (int)text.rfind(sliceChar, sliceEndIdx - 1);
  if (sliceStartIdx == -1) {
    sliceStartIdx = 0;
  } else {
    sliceStartIdx++; // past sliceChar
  }
  if (sliceStartIdx == sliceEndIdx) {
    return current_ = sliceStartIdx;
  }
  baseIter->setText(text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
  return current_ = sliceStartIdx + baseIter->last();
}

int SplittingBreakIterator::following(int offset)
{
  // if the offset is not in this slice, update the slice
  if (offset + 1 < sliceStartIdx || offset + 1 > sliceEndIdx) {
    if (offset == text.length()) { // DONE condition
      last(); // because https://bugs.openjdk.java.net/browse/JDK-8015110
      return DONE;
    }
    sliceStartIdx = (int)text.rfind(sliceChar, offset); // no +1
    if (sliceStartIdx == -1) {
      sliceStartIdx = 0;
    } else {
      sliceStartIdx++; // move past separator
    }
    sliceEndIdx = (int)text.find(sliceChar, max(offset + 1, sliceStartIdx));
    if (sliceEndIdx == -1) {
      sliceEndIdx = text.length();
    }
    if (sliceStartIdx !=
        sliceEndIdx) { // otherwise, adjacent separator or separator at end
      baseIter->setText(
          text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
    }
  }

  // lookup following() in this slice:
  if (sliceStartIdx == sliceEndIdx) {
    return current_ = offset + 1;
  } else {
    // note: following() can never be first() if the first character is a
    // boundary (it usually is).
    //   So we have to check if we should call first() instead of following():
    if (offset == sliceStartIdx - 1) {
      // the first boundary following this offset is the very first boundary in
      // this slice
      return current_ = sliceStartIdx + baseIter->first();
    } else {
      return current_ =
                 sliceStartIdx + baseIter->following(offset - sliceStartIdx);
    }
  }
}

int SplittingBreakIterator::preceding(int offset)
{ // note: closely follows following() but reversed
  if (offset - 1 < sliceStartIdx || offset - 1 > sliceEndIdx) {
    if (offset == 0) { // DONE condition
      first(); // because https://bugs.openjdk.java.net/browse/JDK-8015110
      return DONE;
    }
    sliceEndIdx = (int)text.find(sliceChar, offset); // no -1
    if (sliceEndIdx == -1) {
      sliceEndIdx = text.length();
    }
    sliceStartIdx = (int)text.rfind(sliceChar, offset - 1);
    if (sliceStartIdx == -1) {
      sliceStartIdx = 0;
    } else {
      sliceStartIdx = min(sliceStartIdx + 1, sliceEndIdx);
    }
    if (sliceStartIdx !=
        sliceEndIdx) { // otherwise, adjacent separator or separator at end
      baseIter->setText(
          text.substr(sliceStartIdx, sliceEndIdx - sliceStartIdx));
    }
  }
  // lookup preceding() in this slice:
  if (sliceStartIdx == sliceEndIdx) {
    return current_ = offset - 1;
  } else {
    // note: preceding() can never be last() if the last character is a boundary
    // (it usually is).
    //   So we have to check if we should call last() instead of preceding():
    if (offset == sliceEndIdx + 1) {
      // the last boundary preceding this offset is the very last boundary in
      // this slice
      return current_ = sliceStartIdx + baseIter->last();
    } else {
      return current_ =
                 sliceStartIdx + baseIter->preceding(offset - sliceStartIdx);
    }
  }
}

int SplittingBreakIterator::next(int n)
{
  if (n < 0) {
    for (int i = 0; i < -n; i++) {
      if (previous() == DONE) {
        return DONE;
      }
    }
  } else {
    for (int i = 0; i < n; i++) {
      if (next() == DONE) {
        return DONE;
      }
    }
  }
  return current();
}
} // namespace org::apache::lucene::search::uhighlight