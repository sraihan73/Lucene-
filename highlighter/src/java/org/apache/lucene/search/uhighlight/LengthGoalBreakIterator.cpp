using namespace std;

#include "LengthGoalBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{

shared_ptr<LengthGoalBreakIterator>
LengthGoalBreakIterator::createMinLength(shared_ptr<BreakIterator> baseIter,
                                         int minLength)
{
  return make_shared<LengthGoalBreakIterator>(baseIter, minLength, true);
}

shared_ptr<LengthGoalBreakIterator>
LengthGoalBreakIterator::createClosestToLength(
    shared_ptr<BreakIterator> baseIter, int targetLength)
{
  return make_shared<LengthGoalBreakIterator>(baseIter, targetLength, false);
}

LengthGoalBreakIterator::LengthGoalBreakIterator(
    shared_ptr<BreakIterator> baseIter, int lengthGoal, bool isMinimumLength)
    : baseIter(baseIter), lengthGoal(lengthGoal),
      isMinimumLength(isMinimumLength)
{
}

wstring LengthGoalBreakIterator::toString()
{
  wstring goalDesc = isMinimumLength ? L"minLen" : L"targetLen";
  return getClass().getSimpleName() + L"{" + goalDesc + L"=" +
         to_wstring(lengthGoal) + L", baseIter=" + baseIter + L"}";
}

any LengthGoalBreakIterator::clone()
{
  return make_shared<LengthGoalBreakIterator>(
      std::static_pointer_cast<BreakIterator>(baseIter->clone()), lengthGoal,
      isMinimumLength);
}

shared_ptr<CharacterIterator> LengthGoalBreakIterator::getText()
{
  return baseIter->getText();
}

void LengthGoalBreakIterator::setText(const wstring &newText)
{
  baseIter->setText(newText);
}

void LengthGoalBreakIterator::setText(shared_ptr<CharacterIterator> newText)
{
  baseIter->setText(newText);
}

int LengthGoalBreakIterator::current() { return baseIter->current(); }

int LengthGoalBreakIterator::first() { return baseIter->first(); }

int LengthGoalBreakIterator::last() { return baseIter->last(); }

int LengthGoalBreakIterator::next(int n)
{
  assert((false, L"Not supported"));
  return baseIter->next(n); // probably wrong
}

int LengthGoalBreakIterator::next() { return following(current()); }

int LengthGoalBreakIterator::previous()
{
  assert((false, L"Not supported"));
  return baseIter->previous();
}

int LengthGoalBreakIterator::following(int followingIdx)
{
  constexpr int startIdx = current();
  if (followingIdx < startIdx) {
    assert((false, L"Not supported"));
    return baseIter->following(followingIdx);
  }
  constexpr int targetIdx = startIdx + lengthGoal;
  // When followingIdx >= targetIdx, we can simply delegate since it will be >=
  // the target
  if (followingIdx >= targetIdx - 1) {
    return baseIter->following(followingIdx);
  }
  // If target exceeds the text length, return the last index.
  if (targetIdx >= getText()->getEndIndex()) {
    return baseIter->last();
  }

  // Find closest break >= the target
  constexpr int afterIdx = baseIter->following(targetIdx - 1);
  if (afterIdx == DONE) { // we're at the end; can this happen?
    return current();
  }
  if (afterIdx == targetIdx) { // right on the money
    return afterIdx;
  }
  if (isMinimumLength) { // thus never undershoot
    return afterIdx;
  }

  // note: it is a shame that we invoke preceding() *in addition to*
  // following(); BI's are sometimes expensive.

  // Find closest break < target
  constexpr int beforeIdx =
      baseIter->preceding(targetIdx); // or could do baseIter.previous() but we
                                      // hope the BI implements preceding()
  if (beforeIdx <= followingIdx) {    // too far back
    return moveToBreak(afterIdx);
  }

  if (targetIdx - beforeIdx <= afterIdx - targetIdx) {
    return beforeIdx;
  }
  return moveToBreak(afterIdx);
}

int LengthGoalBreakIterator::moveToBreak(int idx)
{ // precondition: idx is a known break
  // bi.isBoundary(idx) has side-effect of moving the position.  Not obvious!
  // bool moved = baseIter.isBoundary(idx); // probably not particularly
  // expensive assert moved && current() == idx;

  // TODO fix: Would prefer to do "- 1" instead of "- 2" but
  // CustomSeparatorBreakIterator has a bug.
  int current = baseIter->following(idx - 2);
  assert(
      (current == idx, L"following() didn't move us to the expected index."));
  return idx;
}

int LengthGoalBreakIterator::preceding(int offset)
{
  return baseIter->preceding(offset); // no change needed
}

bool LengthGoalBreakIterator::isBoundary(int offset)
{
  assert((false, L"Not supported"));
  return baseIter->isBoundary(offset);
}
} // namespace org::apache::lucene::search::uhighlight