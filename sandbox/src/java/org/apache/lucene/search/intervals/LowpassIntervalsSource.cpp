using namespace std;

#include "LowpassIntervalsSource.h"

namespace org::apache::lucene::search::intervals
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

LowpassIntervalsSource::LowpassIntervalsSource(shared_ptr<IntervalsSource> in_,
                                               int maxWidth)
    : in_(in_), maxWidth(maxWidth)
{
}

bool LowpassIntervalsSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<LowpassIntervalsSource> that =
      any_cast<std::shared_ptr<LowpassIntervalsSource>>(o);
  return maxWidth == that->maxWidth && Objects::equals(in_, that->in_);
}

wstring LowpassIntervalsSource::toString()
{
  return L"MAXWIDTH/" + to_wstring(maxWidth) + L"(" + in_ + L")";
}

void LowpassIntervalsSource::extractTerms(
    const wstring &field, shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  in_->extractTerms(field, terms);
}

shared_ptr<IntervalIterator> LowpassIntervalsSource::intervals(
    const wstring &field, shared_ptr<LeafReaderContext> ctx) 
{
  shared_ptr<IntervalIterator> i = in_->intervals(field, ctx);
  if (i == nullptr) {
    return nullptr;
  }
  return make_shared<IntervalFilterAnonymousInnerClass>(shared_from_this(), i);
}

LowpassIntervalsSource::IntervalFilterAnonymousInnerClass::
    IntervalFilterAnonymousInnerClass(
        shared_ptr<LowpassIntervalsSource> outerInstance,
        shared_ptr<org::apache::lucene::search::intervals::IntervalIterator> i)
    : IntervalFilter(i)
{
  this->outerInstance = outerInstance;
  this->i = i;
}

bool LowpassIntervalsSource::IntervalFilterAnonymousInnerClass::accept()
{
  return (i->end() - i->start()) + 1 <= outerInstance->maxWidth;
}

int LowpassIntervalsSource::hashCode() { return Objects::hash(in_, maxWidth); }
} // namespace org::apache::lucene::search::intervals