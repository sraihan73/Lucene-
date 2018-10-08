using namespace std;

#include "NearSpansOrdered.h"

namespace org::apache::lucene::search::spans
{

NearSpansOrdered::NearSpansOrdered(
    int allowedSlop,
    deque<std::shared_ptr<Spans>> &subSpans) 
    : ConjunctionSpans(subSpans), allowedSlop(allowedSlop)
{
  this->atFirstInCurrentDoc =
      true; // -1 startPosition/endPosition also at doc -1
}

bool NearSpansOrdered::twoPhaseCurrentDocMatches() 
{
  assert(unpositioned());
  oneExhaustedInCurrentDoc = false;
  while (subSpans[0]->nextStartPosition() != NO_MORE_POSITIONS &&
         !oneExhaustedInCurrentDoc) {
    if (stretchToOrder() && matchWidth <= allowedSlop) {
      return atFirstInCurrentDoc = true;
    }
  }
  return false;
}

bool NearSpansOrdered::unpositioned()
{
  for (auto span : subSpans) {
    if (span->startPosition() != -1) {
      return false;
    }
  }
  return true;
}

int NearSpansOrdered::nextStartPosition() 
{
  if (atFirstInCurrentDoc) {
    atFirstInCurrentDoc = false;
    return matchStart;
  }
  oneExhaustedInCurrentDoc = false;
  while (subSpans[0]->nextStartPosition() != NO_MORE_POSITIONS &&
         !oneExhaustedInCurrentDoc) {
    if (stretchToOrder() && matchWidth <= allowedSlop) {
      return matchStart;
    }
  }
  return matchStart = matchEnd = NO_MORE_POSITIONS;
}

bool NearSpansOrdered::stretchToOrder() 
{
  shared_ptr<Spans> prevSpans = subSpans[0];
  matchStart = prevSpans->startPosition();
  assert((prevSpans->startPosition() != NO_MORE_POSITIONS,
          L"prevSpans no start position " + prevSpans));
  assert(prevSpans->endPosition() != NO_MORE_POSITIONS);
  matchWidth = 0;
  for (int i = 1; i < subSpans.size(); i++) {
    shared_ptr<Spans> spans = subSpans[i];
    assert(spans->startPosition() != NO_MORE_POSITIONS);
    assert(spans->endPosition() != NO_MORE_POSITIONS);
    if (advancePosition(spans, prevSpans->endPosition()) == NO_MORE_POSITIONS) {
      oneExhaustedInCurrentDoc = true;
      return false;
    }
    matchWidth += (spans->startPosition() - prevSpans->endPosition());
    prevSpans = spans;
  }
  matchEnd = subSpans[subSpans.size() - 1]->endPosition();
  return true; // all subSpans ordered and non overlapping
}

int NearSpansOrdered::advancePosition(shared_ptr<Spans> spans,
                                      int position) 
{
  if (std::dynamic_pointer_cast<SpanNearQuery::GapSpans>(spans) != nullptr) {
    return (std::static_pointer_cast<SpanNearQuery::GapSpans>(spans))
        ->skipToPosition(position);
  }
  while (spans->startPosition() < position) {
    spans->nextStartPosition();
  }
  return spans->startPosition();
}

int NearSpansOrdered::startPosition()
{
  return atFirstInCurrentDoc ? -1 : matchStart;
}

int NearSpansOrdered::endPosition()
{
  return atFirstInCurrentDoc ? -1 : matchEnd;
}

int NearSpansOrdered::width() { return matchWidth; }

void NearSpansOrdered::collect(shared_ptr<SpanCollector> collector) throw(
    IOException)
{
  for (auto span : subSpans) {
    span->collect(collector);
  }
}
} // namespace org::apache::lucene::search::spans