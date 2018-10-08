using namespace std;

#include "ContainSpans.h"

namespace org::apache::lucene::search::spans
{

ContainSpans::ContainSpans(shared_ptr<Spans> bigSpans,
                           shared_ptr<Spans> littleSpans,
                           shared_ptr<Spans> sourceSpans)
    : ConjunctionSpans(Arrays::asList(bigSpans, littleSpans))
{
  this->bigSpans = Objects::requireNonNull(bigSpans);
  this->littleSpans = Objects::requireNonNull(littleSpans);
  this->sourceSpans = Objects::requireNonNull(sourceSpans);
}

int ContainSpans::startPosition()
{
  return atFirstInCurrentDoc
             ? -1
             : oneExhaustedInCurrentDoc ? NO_MORE_POSITIONS
                                        : sourceSpans->startPosition();
}

int ContainSpans::endPosition()
{
  return atFirstInCurrentDoc
             ? -1
             : oneExhaustedInCurrentDoc ? NO_MORE_POSITIONS
                                        : sourceSpans->endPosition();
}

int ContainSpans::width() { return sourceSpans->width(); }

void ContainSpans::collect(shared_ptr<SpanCollector> collector) throw(
    IOException)
{
  bigSpans->collect(collector);
  littleSpans->collect(collector);
}
} // namespace org::apache::lucene::search::spans