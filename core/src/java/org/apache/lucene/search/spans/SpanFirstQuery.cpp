using namespace std;

#include "SpanFirstQuery.h"

namespace org::apache::lucene::search::spans
{
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;

SpanFirstQuery::SpanFirstQuery(shared_ptr<SpanQuery> match, int end)
    : SpanPositionRangeQuery(match, 0, end)
{
}

AcceptStatus
SpanFirstQuery::acceptPosition(shared_ptr<Spans> spans) 
{
  assert((spans->startPosition() != spans->endPosition(),
          L"start equals end: " + spans->startPosition()));
  if (spans->startPosition() >= end) {
    return AcceptStatus::NO_MORE_IN_CURRENT_DOC;
  } else if (spans->endPosition() <= end) {
    return AcceptStatus::YES;
  } else {
    return AcceptStatus::NO;
  }
}

wstring SpanFirstQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"spanFirst(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(match->toString(field));
  buffer->append(L", ");
  buffer->append(end);
  buffer->append(L")");
  return buffer->toString();
}
} // namespace org::apache::lucene::search::spans