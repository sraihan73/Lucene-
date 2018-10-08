using namespace std;

#include "NearSpansUnordered.h"

namespace org::apache::lucene::search::spans
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

NearSpansUnordered::NearSpansUnordered(
    int allowedSlop,
    deque<std::shared_ptr<Spans>> &subSpans) 
    : ConjunctionSpans(subSpans), allowedSlop(allowedSlop)
{

  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  this->spanWindow =
      make_shared<SpanTotalLengthEndPositionWindow>(shared_from_this());
}

NearSpansUnordered::SpanTotalLengthEndPositionWindow::
    SpanTotalLengthEndPositionWindow(
        shared_ptr<NearSpansUnordered> outerInstance)
    : org::apache::lucene::util::PriorityQueue<Spans>(subSpans.length),
      outerInstance(outerInstance)
{
}

bool NearSpansUnordered::SpanTotalLengthEndPositionWindow::lessThan(
    shared_ptr<Spans> spans1, shared_ptr<Spans> spans2)
{
  return positionsOrdered(spans1, spans2);
}

void NearSpansUnordered::SpanTotalLengthEndPositionWindow::
    startDocument() 
{
  clear();
  totalSpanLength = 0;
  maxEndPosition = -1;
  for (auto spans : outerInstance->subSpans) {
    assert(spans->startPosition() == -1);
    spans->nextStartPosition();
    assert(spans->startPosition() != NO_MORE_POSITIONS);
    add(spans);
    if (spans->endPosition() > maxEndPosition) {
      maxEndPosition = spans->endPosition();
    }
    int spanLength = spans->endPosition() - spans->startPosition();
    assert(spanLength >= 0);
    totalSpanLength += spanLength;
  }
}

bool NearSpansUnordered::SpanTotalLengthEndPositionWindow::nextPosition() throw(
    IOException)
{
  shared_ptr<Spans> topSpans = top();
  assert(topSpans->startPosition() != NO_MORE_POSITIONS);
  int spanLength = topSpans->endPosition() - topSpans->startPosition();
  int nextStartPos = topSpans->nextStartPosition();
  if (nextStartPos == NO_MORE_POSITIONS) {
    return false;
  }
  totalSpanLength -= spanLength;
  spanLength = topSpans->endPosition() - topSpans->startPosition();
  totalSpanLength += spanLength;
  if (topSpans->endPosition() > maxEndPosition) {
    maxEndPosition = topSpans->endPosition();
  }
  updateTop();
  return true;
}

bool NearSpansUnordered::SpanTotalLengthEndPositionWindow::atMatch()
{
  bool res = (maxEndPosition - top()->startPosition() - totalSpanLength) <=
             outerInstance->allowedSlop;
  return res;
}

bool NearSpansUnordered::positionsOrdered(shared_ptr<Spans> spans1,
                                          shared_ptr<Spans> spans2)
{
  assert((spans1->docID() == spans2->docID(),
          L"doc1 " + to_wstring(spans1->docID()) + L" != doc2 " +
              to_wstring(spans2->docID())));
  int start1 = spans1->startPosition();
  int start2 = spans2->startPosition();
  return (start1 == start2) ? (spans1->endPosition() < spans2->endPosition())
                            : (start1 < start2);
}

bool NearSpansUnordered::twoPhaseCurrentDocMatches() 
{
  // at doc with all subSpans
  spanWindow->startDocument();
  while (true) {
    if (spanWindow->atMatch()) {
      atFirstInCurrentDoc = true;
      oneExhaustedInCurrentDoc = false;
      return true;
    }
    if (!spanWindow->nextPosition()) {
      return false;
    }
  }
}

int NearSpansUnordered::nextStartPosition() 
{
  if (atFirstInCurrentDoc) {
    atFirstInCurrentDoc = false;
    return spanWindow->top()->startPosition();
  }
  assert(spanWindow->top()->startPosition() != -1);
  assert(spanWindow->top()->startPosition() != NO_MORE_POSITIONS);
  while (true) {
    if (!spanWindow->nextPosition()) {
      oneExhaustedInCurrentDoc = true;
      return NO_MORE_POSITIONS;
    }
    if (spanWindow->atMatch()) {
      return spanWindow->top()->startPosition();
    }
  }
}

int NearSpansUnordered::startPosition()
{
  assert(spanWindow->top() != nullptr);
  return atFirstInCurrentDoc
             ? -1
             : oneExhaustedInCurrentDoc ? NO_MORE_POSITIONS
                                        : spanWindow->top()->startPosition();
}

int NearSpansUnordered::endPosition()
{
  return atFirstInCurrentDoc
             ? -1
             : oneExhaustedInCurrentDoc ? NO_MORE_POSITIONS
                                        : spanWindow->maxEndPosition;
}

int NearSpansUnordered::width()
{
  return spanWindow->maxEndPosition - spanWindow->top()->startPosition();
}

void NearSpansUnordered::collect(shared_ptr<SpanCollector> collector) throw(
    IOException)
{
  for (auto spans : subSpans) {
    spans->collect(collector);
  }
}
} // namespace org::apache::lucene::search::spans