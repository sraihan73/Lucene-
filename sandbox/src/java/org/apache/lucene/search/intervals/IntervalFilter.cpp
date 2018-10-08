using namespace std;

#include "IntervalFilter.h"

namespace org::apache::lucene::search::intervals
{

IntervalFilter::IntervalFilter(shared_ptr<IntervalIterator> in_)
    : in_(Objects::requireNonNull(in_))
{
}

int IntervalFilter::docID() { return in_->docID(); }

int IntervalFilter::nextDoc()  { return in_->nextDoc(); }

int IntervalFilter::advance(int target) 
{
  return in_->advance(target);
}

int64_t IntervalFilter::cost() { return in_->cost(); }

int IntervalFilter::start() { return in_->start(); }

int IntervalFilter::end() { return in_->end(); }

float IntervalFilter::matchCost() { return in_->matchCost(); }

int IntervalFilter::nextInterval() 
{
  int next;
  do {
    next = in_->nextInterval();
  } while (accept() == false && next != IntervalIterator::NO_MORE_INTERVALS);
  return next;
}
} // namespace org::apache::lucene::search::intervals