using namespace std;

#include "WholeBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{

int WholeBreakIterator::current() { return current_; }

int WholeBreakIterator::first() { return (current_ = start); }

int WholeBreakIterator::following(int pos)
{
  if (pos < start || pos > end) {
    throw invalid_argument(L"offset out of bounds");
  } else if (pos == end) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    current_ = end;
    return DONE;
  } else {
    return last();
  }
}

shared_ptr<CharacterIterator> WholeBreakIterator::getText() { return text; }

int WholeBreakIterator::last() { return (current_ = end); }

int WholeBreakIterator::next()
{
  if (current_ == end) {
    return DONE;
  } else {
    return last();
  }
}

int WholeBreakIterator::next(int n)
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

int WholeBreakIterator::preceding(int pos)
{
  if (pos < start || pos > end) {
    throw invalid_argument(L"offset out of bounds");
  } else if (pos == start) {
    // this conflicts with the javadocs, but matches actual behavior (Oracle has
    // a bug in something) https://bugs.openjdk.java.net/browse/JDK-8015110
    current_ = start;
    return DONE;
  } else {
    return first();
  }
}

int WholeBreakIterator::previous()
{
  if (current_ == start) {
    return DONE;
  } else {
    return first();
  }
}

void WholeBreakIterator::setText(shared_ptr<CharacterIterator> newText)
{
  start = newText->getBeginIndex();
  end = newText->getEndIndex();
  text = newText;
  current_ = start;
}
} // namespace org::apache::lucene::search::uhighlight