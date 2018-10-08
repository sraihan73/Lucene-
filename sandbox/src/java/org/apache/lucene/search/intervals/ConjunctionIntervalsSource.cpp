using namespace std;

#include "ConjunctionIntervalsSource.h"

namespace org::apache::lucene::search::intervals
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

ConjunctionIntervalsSource::ConjunctionIntervalsSource(
    deque<std::shared_ptr<IntervalsSource>> &subSources,
    shared_ptr<IntervalFunction> function)
    : subSources(subSources), function(function)
{
}

bool ConjunctionIntervalsSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<ConjunctionIntervalsSource> that =
      any_cast<std::shared_ptr<ConjunctionIntervalsSource>>(o);
  return Objects::equals(subSources, that->subSources) &&
         Objects::equals(function, that->function);
}

wstring ConjunctionIntervalsSource::toString()
{
  return function + subSources.stream()
                        .map_obj(any::toString)
                        .collect(Collectors::joining(L",", L"(", L")"));
}

void ConjunctionIntervalsSource::extractTerms(
    const wstring &field, shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto source : subSources) {
    source->extractTerms(field, terms);
  }
}

shared_ptr<IntervalIterator> ConjunctionIntervalsSource::intervals(
    const wstring &field, shared_ptr<LeafReaderContext> ctx) 
{
  deque<std::shared_ptr<IntervalIterator>> subIntervals =
      deque<std::shared_ptr<IntervalIterator>>();
  for (auto source : subSources) {
    shared_ptr<IntervalIterator> it = source->intervals(field, ctx);
    if (it == nullptr) {
      return nullptr;
    }
    subIntervals.push_back(it);
  }
  return function->apply(subIntervals);
}

int ConjunctionIntervalsSource::hashCode()
{
  return Objects::hash(subSources, function);
}
} // namespace org::apache::lucene::search::intervals