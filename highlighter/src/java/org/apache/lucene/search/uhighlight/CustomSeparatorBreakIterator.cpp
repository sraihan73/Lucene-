using namespace std;

#include "CustomSeparatorBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{

CustomSeparatorBreakIterator::CustomSeparatorBreakIterator(wchar_t separator)
    : separator(separator)
{
}

int CustomSeparatorBreakIterator::current() { return current_; }

int CustomSeparatorBreakIterator::first()
{
  text->setIndex(text->getBeginIndex());
  return current_ = text->getIndex();
}

int CustomSeparatorBreakIterator::last()
{
  text->setIndex(text->getEndIndex());
  return current_ = text->getIndex();
}

int CustomSeparatorBreakIterator::next()
{
  if (text->getIndex() == text->getEndIndex()) {
    return DONE;
  } else {
    return advanceForward();
  }
}

int CustomSeparatorBreakIterator::advanceForward()
{
  wchar_t c;
  while ((c = text->next()) != CharacterIterator::DONE) {
    if (c == separator) {
      return current_ = text->getIndex() + 1;
    }
  }
  assert(text->getIndex() == text->getEndIndex());
  return current_ = text->getIndex();
}

int CustomSeparatorBreakIterator::following(int pos)
{
  if (pos < text->getBeginIndex() || pos > text->getEndIndex()) {
    throw invalid_argument(L"offset out of bounds");
  } else if (pos == text->getEndIndex()) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    text->setIndex(text->getEndIndex());
    current_ = text->getIndex();
    return DONE;
  } else {
    text->setIndex(pos);
    current_ = text->getIndex();
    return advanceForward();
  }
}

int CustomSeparatorBreakIterator::previous()
{
  if (text->getIndex() == text->getBeginIndex()) {
    return DONE;
  } else {
    return advanceBackward();
  }
}

int CustomSeparatorBreakIterator::advanceBackward()
{
  wchar_t c;
  while ((c = text->previous()) != CharacterIterator::DONE) {
    if (c == separator) {
      return current_ = text->getIndex() + 1;
    }
  }
  assert(text->getIndex() == text->getBeginIndex());
  return current_ = text->getIndex();
}

int CustomSeparatorBreakIterator::preceding(int pos)
{
  if (pos < text->getBeginIndex() || pos > text->getEndIndex()) {
    throw invalid_argument(L"offset out of bounds");
  } else if (pos == text->getBeginIndex()) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    text->setIndex(text->getBeginIndex());
    current_ = text->getIndex();
    return DONE;
  } else {
    text->setIndex(pos);
    current_ = text->getIndex();
    return advanceBackward();
  }
}

int CustomSeparatorBreakIterator::next(int n)
{
  if (n < 0) {
    for (int i = 0; i < -n; i++) {
      previous();
    }
  } else {
    for (int i = 0; i < n; i++) {
      next();
    }
  }
  return current();
}

shared_ptr<CharacterIterator> CustomSeparatorBreakIterator::getText()
{
  return text;
}

void CustomSeparatorBreakIterator::setText(
    shared_ptr<CharacterIterator> newText)
{
  text = newText;
  current_ = text->getBeginIndex();
}
} // namespace org::apache::lucene::search::uhighlight