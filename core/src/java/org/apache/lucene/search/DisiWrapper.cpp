using namespace std;

#include "DisiWrapper.h"

namespace org::apache::lucene::search
{
using Spans = org::apache::lucene::search::spans::Spans;

DisiWrapper::DisiWrapper(shared_ptr<Scorer> scorer)
    : iterator(scorer->begin()), scorer(scorer), cost(iterator->cost()),
      twoPhaseView(scorer->twoPhaseIterator()), spans(this->spans.reset())
{
  this->doc = -1;

  if (twoPhaseView != nullptr) {
    approximation = twoPhaseView->approximation();
    matchCost = twoPhaseView->matchCost();
  } else {
    approximation = iterator;
    matchCost = 0.0f;
  }
}

DisiWrapper::DisiWrapper(shared_ptr<Spans> spans)
    : iterator(spans), scorer(this->scorer.reset()), cost(iterator->cost()),
      twoPhaseView(spans->asTwoPhaseIterator()), spans(spans)
{
  this->doc = -1;

  if (twoPhaseView != nullptr) {
    approximation = twoPhaseView->approximation();
    matchCost = twoPhaseView->matchCost();
  } else {
    approximation = iterator;
    matchCost = 0.0f;
  }
  this->lastApproxNonMatchDoc = -2;
  this->lastApproxMatchDoc = -2;
}
} // namespace org::apache::lucene::search