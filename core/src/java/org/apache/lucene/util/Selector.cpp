using namespace std;

#include "Selector.h"

namespace org::apache::lucene::util
{

void Selector::checkArgs(int from, int to, int k)
{
  if (k < from) {
    throw invalid_argument(L"k must be >= from");
  }
  if (k >= to) {
    throw invalid_argument(L"k must be < to");
  }
}
} // namespace org::apache::lucene::util