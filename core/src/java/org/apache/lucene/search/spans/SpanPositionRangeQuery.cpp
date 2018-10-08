using namespace std;

#include "SpanPositionRangeQuery.h"

namespace org::apache::lucene::search::spans
{
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;

SpanPositionRangeQuery::SpanPositionRangeQuery(shared_ptr<SpanQuery> match,
                                               int start, int end)
    : SpanPositionCheckQuery(match)
{
  this->start = start;
  this->end = end;
}

AcceptStatus SpanPositionRangeQuery::acceptPosition(
    shared_ptr<Spans> spans) 
{
  assert(spans->startPosition() != spans->endPosition());
  AcceptStatus res =
      (spans->startPosition() >= end)
          ? AcceptStatus::NO_MORE_IN_CURRENT_DOC
          : (spans->startPosition() >= start && spans->endPosition() <= end)
                ? AcceptStatus::YES
                : AcceptStatus::NO;
  return res;
}

int SpanPositionRangeQuery::getStart() { return start; }

int SpanPositionRangeQuery::getEnd() { return end; }

wstring SpanPositionRangeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"spanPosRange(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(match->toString(field));
  buffer->append(L", ")->append(start)->append(L", ");
  buffer->append(end);
  buffer->append(L")");
  return buffer->toString();
}

bool SpanPositionRangeQuery::equals(any o)
{
  if (!SpanPositionCheckQuery::equals(o)) {
    return false;
  }
  shared_ptr<SpanPositionRangeQuery> other =
      any_cast<std::shared_ptr<SpanPositionRangeQuery>>(o);
  return this->end == other->end && this->start == other->start;
}

int SpanPositionRangeQuery::hashCode()
{
  int h = SpanPositionCheckQuery::hashCode() ^ end;
  h = (h * 127) ^ start;
  return h;
}
} // namespace org::apache::lucene::search::spans