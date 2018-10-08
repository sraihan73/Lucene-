using namespace std;

#include "Intervals.h"

namespace org::apache::lucene::search::intervals
{
using BytesRef = org::apache::lucene::util::BytesRef;

Intervals::Intervals() {}

shared_ptr<IntervalsSource> Intervals::term(shared_ptr<BytesRef> term)
{
  return make_shared<TermIntervalsSource>(term);
}

shared_ptr<IntervalsSource> Intervals::term(const wstring &term)
{
  return make_shared<TermIntervalsSource>(make_shared<BytesRef>(term));
}

shared_ptr<IntervalsSource> Intervals::phrase(deque<wstring> &terms)
{
  std::deque<std::shared_ptr<IntervalsSource>> sources(terms->length);
  int i = 0;
  for (wstring term : terms) {
    sources[i] = Intervals::term(term);
    i++;
  }
  return phrase(sources);
}

shared_ptr<IntervalsSource>
Intervals::phrase(deque<IntervalsSource> &subSources)
{
  return make_shared<ConjunctionIntervalsSource>(Arrays::asList(subSources),
                                                 IntervalFunction::BLOCK);
}

shared_ptr<IntervalsSource> Intervals:: or
    (deque<IntervalsSource> & subSources)
{
  if (subSources->length == 1) {
    return subSources[0];
  }
  return make_shared<DisjunctionIntervalsSource>(Arrays::asList(subSources));
}

shared_ptr<IntervalsSource>
Intervals::maxwidth(int width, shared_ptr<IntervalsSource> subSource)
{
  return make_shared<LowpassIntervalsSource>(subSource, width);
}

shared_ptr<IntervalsSource>
Intervals::ordered(deque<IntervalsSource> &subSources)
{
  return make_shared<ConjunctionIntervalsSource>(Arrays::asList(subSources),
                                                 IntervalFunction::ORDERED);
}

shared_ptr<IntervalsSource>
Intervals::unordered(deque<IntervalsSource> &subSources)
{
  return unordered(true, subSources);
}

shared_ptr<IntervalsSource>
Intervals::unordered(bool allowOverlaps, deque<IntervalsSource> &subSources)
{
  return make_shared<ConjunctionIntervalsSource>(
      Arrays::asList(subSources), allowOverlaps
                                      ? IntervalFunction::UNORDERED
                                      : IntervalFunction::UNORDERED_NO_OVERLAP);
}

shared_ptr<IntervalsSource>
Intervals::nonOverlapping(shared_ptr<IntervalsSource> minuend,
                          shared_ptr<IntervalsSource> subtrahend)
{
  return make_shared<DifferenceIntervalsSource>(
      minuend, subtrahend, DifferenceIntervalFunction::NON_OVERLAPPING);
}

shared_ptr<IntervalsSource>
Intervals::notWithin(shared_ptr<IntervalsSource> minuend, int positions,
                     shared_ptr<IntervalsSource> subtrahend)
{
  return make_shared<DifferenceIntervalsSource>(
      minuend, subtrahend,
      make_shared<DifferenceIntervalFunction::NotWithinFunction>(positions));
}

shared_ptr<IntervalsSource>
Intervals::notContaining(shared_ptr<IntervalsSource> minuend,
                         shared_ptr<IntervalsSource> subtrahend)
{
  return make_shared<DifferenceIntervalsSource>(
      minuend, subtrahend, DifferenceIntervalFunction::NOT_CONTAINING);
}

shared_ptr<IntervalsSource>
Intervals::containing(shared_ptr<IntervalsSource> big,
                      shared_ptr<IntervalsSource> small)
{
  return make_shared<ConjunctionIntervalsSource>(Arrays::asList(big, small),
                                                 IntervalFunction::CONTAINING);
}

shared_ptr<IntervalsSource>
Intervals::notContainedBy(shared_ptr<IntervalsSource> small,
                          shared_ptr<IntervalsSource> big)
{
  return make_shared<DifferenceIntervalsSource>(
      small, big, DifferenceIntervalFunction::NOT_CONTAINED_BY);
}

shared_ptr<IntervalsSource>
Intervals::containedBy(shared_ptr<IntervalsSource> small,
                       shared_ptr<IntervalsSource> big)
{
  return make_shared<ConjunctionIntervalsSource>(
      Arrays::asList(small, big), IntervalFunction::CONTAINED_BY);
}
} // namespace org::apache::lucene::search::intervals