using namespace std;

#include "TermMatchesIterator.h"

namespace org::apache::lucene::search
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;

TermMatchesIterator::TermMatchesIterator(shared_ptr<PostingsEnum> pe) throw(
    IOException)
    : pe(pe)
{
  this->upto = pe->freq();
}

bool TermMatchesIterator::next() 
{
  if (upto-- > 0) {
    pos = pe->nextPosition();
    return true;
  }
  return false;
}

int TermMatchesIterator::startPosition() { return pos; }

int TermMatchesIterator::endPosition() { return pos; }

int TermMatchesIterator::startOffset() 
{
  return pe->startOffset();
}

int TermMatchesIterator::endOffset() 
{
  return pe->endOffset();
}
} // namespace org::apache::lucene::search