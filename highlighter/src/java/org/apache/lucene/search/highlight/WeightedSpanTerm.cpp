using namespace std;

#include "WeightedSpanTerm.h"

namespace org::apache::lucene::search::highlight
{

WeightedSpanTerm::WeightedSpanTerm(float weight, const wstring &term)
    : WeightedTerm(weight, term)
{
  this->positionSpans = deque<>();
}

WeightedSpanTerm::WeightedSpanTerm(float weight, const wstring &term,
                                   bool positionSensitive)
    : WeightedTerm(weight, term)
{
  this->positionSensitive = positionSensitive;
}

bool WeightedSpanTerm::checkPosition(int position)
{
  // There would probably be a slight speed improvement if PositionSpans
  // where kept in some sort of priority queue - that way this method
  // could
  // bail early without checking each PositionSpan.
  deque<std::shared_ptr<PositionSpan>>::const_iterator positionSpanIt =
      positionSpans.begin();

  while (positionSpanIt != positionSpans.end()) {
    shared_ptr<PositionSpan> posSpan = *positionSpanIt;

    if (((position >= posSpan->start) && (position <= posSpan->end))) {
      return true;
    }
    positionSpanIt++;
  }

  return false;
}

void WeightedSpanTerm::addPositionSpans(
    deque<std::shared_ptr<PositionSpan>> &positionSpans)
{
  this->positionSpans.insert(this->positionSpans.end(), positionSpans.begin(),
                             positionSpans.end());
}

bool WeightedSpanTerm::isPositionSensitive() { return positionSensitive; }

void WeightedSpanTerm::setPositionSensitive(bool positionSensitive)
{
  this->positionSensitive = positionSensitive;
}

deque<std::shared_ptr<PositionSpan>> WeightedSpanTerm::getPositionSpans()
{
  return positionSpans;
}
} // namespace org::apache::lucene::search::highlight