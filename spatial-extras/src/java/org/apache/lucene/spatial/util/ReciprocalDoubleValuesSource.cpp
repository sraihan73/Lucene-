using namespace std;

#include "ReciprocalDoubleValuesSource.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

ReciprocalDoubleValuesSource::ReciprocalDoubleValuesSource(
    double distToEdge, shared_ptr<DoubleValuesSource> input)
    : distToEdge(distToEdge), input(input)
{
}

shared_ptr<DoubleValues> ReciprocalDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<DoubleValues> in_ = input->getValues(ctx, scores);
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(), in_);
}

ReciprocalDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<ReciprocalDoubleValuesSource> outerInstance,
        shared_ptr<DoubleValues> in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
}

double ReciprocalDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    doubleValue() 
{
  return outerInstance->recip(in_->doubleValue());
}

bool ReciprocalDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return in_->advanceExact(doc);
}

double ReciprocalDoubleValuesSource::recip(double in_)
{
  return distToEdge / (in_ + distToEdge);
}

bool ReciprocalDoubleValuesSource::needsScores()
{
  return input->needsScores();
}

bool ReciprocalDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return input->isCacheable(ctx);
}

shared_ptr<Explanation> ReciprocalDoubleValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  shared_ptr<Explanation> expl = input->explain(ctx, docId, scoreExplanation);
  return Explanation::match(static_cast<float>(recip(expl->getValue())),
                            to_wstring(distToEdge) + L" / (v + " +
                                to_wstring(distToEdge) + L"), computed from:",
                            expl);
}

shared_ptr<DoubleValuesSource> ReciprocalDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<ReciprocalDoubleValuesSource>(distToEdge,
                                                   input->rewrite(searcher));
}

bool ReciprocalDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<ReciprocalDoubleValuesSource> that =
      any_cast<std::shared_ptr<ReciprocalDoubleValuesSource>>(o);
  return Double::compare(that->distToEdge, distToEdge) == 0 &&
         Objects::equals(input, that->input);
}

int ReciprocalDoubleValuesSource::hashCode()
{
  return Objects::hash(distToEdge, input);
}

wstring ReciprocalDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"recip(" + to_wstring(distToEdge) + L", " + input->toString() + L")";
}
} // namespace org::apache::lucene::spatial::util