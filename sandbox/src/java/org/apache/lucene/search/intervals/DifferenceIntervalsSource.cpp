using namespace std;

#include "DifferenceIntervalsSource.h"

namespace org::apache::lucene::search::intervals
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

DifferenceIntervalsSource::DifferenceIntervalsSource(
    shared_ptr<IntervalsSource> minuend, shared_ptr<IntervalsSource> subtrahend,
    shared_ptr<DifferenceIntervalFunction> function)
    : minuend(minuend), subtrahend(subtrahend), function(function)
{
}

shared_ptr<IntervalIterator> DifferenceIntervalsSource::intervals(
    const wstring &field, shared_ptr<LeafReaderContext> ctx) 
{
  shared_ptr<IntervalIterator> minIt = minuend->intervals(field, ctx);
  if (minIt == nullptr) {
    return nullptr;
  }
  shared_ptr<IntervalIterator> subIt = subtrahend->intervals(field, ctx);
  if (subIt == nullptr) {
    return minIt;
  }
  return function->apply(minIt, subIt);
}

bool DifferenceIntervalsSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<DifferenceIntervalsSource> that =
      any_cast<std::shared_ptr<DifferenceIntervalsSource>>(o);
  return Objects::equals(minuend, that->minuend) &&
         Objects::equals(subtrahend, that->subtrahend) &&
         Objects::equals(function, that->function);
}

int DifferenceIntervalsSource::hashCode()
{
  return Objects::hash(minuend, subtrahend, function);
}

wstring DifferenceIntervalsSource::toString()
{
  return function + L"(" + minuend + L", " + subtrahend + L")";
}

void DifferenceIntervalsSource::extractTerms(
    const wstring &field, shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  minuend->extractTerms(field, terms);
}
} // namespace org::apache::lucene::search::intervals