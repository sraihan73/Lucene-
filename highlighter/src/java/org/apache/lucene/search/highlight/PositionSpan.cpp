using namespace std;

#include "PositionSpan.h"

namespace org::apache::lucene::search::highlight
{

PositionSpan::PositionSpan(int start, int end)
{
  this->start = start;
  this->end = end;
}
} // namespace org::apache::lucene::search::highlight