using namespace std;

#include "PhraseQueue.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

PhraseQueue::PhraseQueue(int size)
    : org::apache::lucene::util::PriorityQueue<PhrasePositions>(size)
{
}

bool PhraseQueue::lessThan(shared_ptr<PhrasePositions> pp1,
                           shared_ptr<PhrasePositions> pp2)
{
  if (pp1->position == pp2->position) {
    // same doc and pp.position, so decide by actual term positions.
    // rely on: pp.position == tp.position - offset.
    if (pp1->offset == pp2->offset) {
      return pp1->ord < pp2->ord;
    } else {
      return pp1->offset < pp2->offset;
    }
  } else {
    return pp1->position < pp2->position;
  }
}
} // namespace org::apache::lucene::search