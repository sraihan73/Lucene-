using namespace std;

#include "SimpleBoundaryScanner.h"

namespace org::apache::lucene::search::vectorhighlight
{

std::deque<optional<wchar_t>> const
    SimpleBoundaryScanner::DEFAULT_BOUNDARY_CHARS = {L'.', L',',  L'!', L'?',
                                                     L' ', L'\t', L'\n'};

SimpleBoundaryScanner::SimpleBoundaryScanner()
    : SimpleBoundaryScanner(DEFAULT_MAX_SCAN, DEFAULT_BOUNDARY_CHARS)
{
}

SimpleBoundaryScanner::SimpleBoundaryScanner(int maxScan)
    : SimpleBoundaryScanner(maxScan, DEFAULT_BOUNDARY_CHARS)
{
}

SimpleBoundaryScanner::SimpleBoundaryScanner(
    std::deque<optional<wchar_t> &> &boundaryChars)
    : SimpleBoundaryScanner(DEFAULT_MAX_SCAN, boundaryChars)
{
}

SimpleBoundaryScanner::SimpleBoundaryScanner(
    int maxScan, std::deque<Character> &boundaryChars)
{
  this->maxScan = maxScan;
  this->boundaryChars = unordered_set<>();
  this->boundaryChars->addAll(Arrays::asList(boundaryChars));
}

SimpleBoundaryScanner::SimpleBoundaryScanner(
    int maxScan, shared_ptr<Set<wchar_t>> boundaryChars)
{
  this->maxScan = maxScan;
  this->boundaryChars = boundaryChars;
}

int SimpleBoundaryScanner::findStartOffset(shared_ptr<StringBuilder> buffer,
                                           int start)
{
  // avoid illegal start offset
  if (start > buffer->length() || start < 1) {
    return start;
  }
  int offset, count = maxScan;
  for (offset = start; offset > 0 && count > 0; count--) {
    // found?
    if (boundaryChars->contains(buffer->charAt(offset - 1))) {
      return offset;
    }
    offset--;
  }
  // if we scanned up to the start of the text, return it, it's a "boundary"
  if (offset == 0) {
    return 0;
  }
  // not found
  return start;
}

int SimpleBoundaryScanner::findEndOffset(shared_ptr<StringBuilder> buffer,
                                         int start)
{
  // avoid illegal start offset
  if (start > buffer->length() || start < 0) {
    return start;
  }
  int offset, count = maxScan;
  // for( offset = start; offset <= buffer.length() && count > 0; count-- ){
  for (offset = start; offset < buffer->length() && count > 0; count--) {
    // found?
    if (boundaryChars->contains(buffer->charAt(offset))) {
      return offset;
    }
    offset++;
  }
  // not found
  return start;
}
} // namespace org::apache::lucene::search::vectorhighlight