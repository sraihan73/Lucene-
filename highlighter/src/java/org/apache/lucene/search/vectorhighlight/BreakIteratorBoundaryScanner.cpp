using namespace std;

#include "BreakIteratorBoundaryScanner.h"

namespace org::apache::lucene::search::vectorhighlight
{

BreakIteratorBoundaryScanner::BreakIteratorBoundaryScanner(
    shared_ptr<BreakIterator> bi)
    : bi(bi)
{
}

int BreakIteratorBoundaryScanner::findStartOffset(
    shared_ptr<StringBuilder> buffer, int start)
{
  // avoid illegal start offset
  if (start > buffer->length() || start < 1) {
    return start;
  }
  bi->setText(buffer->substr(0, start));
  bi->last();
  return bi->previous();
}

int BreakIteratorBoundaryScanner::findEndOffset(
    shared_ptr<StringBuilder> buffer, int start)
{
  // avoid illegal start offset
  if (start > buffer->length() || start < 0) {
    return start;
  }
  bi->setText(buffer->substr(start));
  return bi->next() + start;
}
} // namespace org::apache::lucene::search::vectorhighlight