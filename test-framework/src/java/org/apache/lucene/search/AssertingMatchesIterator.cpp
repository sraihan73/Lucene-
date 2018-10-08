using namespace std;

#include "AssertingMatchesIterator.h"

namespace org::apache::lucene::search
{

AssertingMatchesIterator::AssertingMatchesIterator(
    shared_ptr<MatchesIterator> in_)
    : in_(in_)
{
}

bool AssertingMatchesIterator::next() 
{
  assert((state != State::EXHAUSTED, state));
  bool more = in_->next();
  if (more == false) {
    state = State::EXHAUSTED;
  } else {
    state = State::ITERATING;
  }
  return more;
}

int AssertingMatchesIterator::startPosition()
{
  assert((state == State::ITERATING, state));
  return in_->startPosition();
}

int AssertingMatchesIterator::endPosition()
{
  assert((state == State::ITERATING, state));
  return in_->endPosition();
}

int AssertingMatchesIterator::startOffset() 
{
  assert((state == State::ITERATING, state));
  return in_->startOffset();
}

int AssertingMatchesIterator::endOffset() 
{
  assert((state == State::ITERATING, state));
  return in_->endOffset();
}
} // namespace org::apache::lucene::search