using namespace std;

#include "Average.h"

namespace org::apache::lucene::search::suggest
{

Average::Average(double avg, double stddev) : avg(avg), stddev(stddev) {}

wstring Average::toString()
{
  return wstring::format(Locale::ROOT, L"%.0f [+- %.2f]", avg, stddev);
}

shared_ptr<Average> Average::from(deque<double> &values)
{
  double sum = 0;
  double sumSquares = 0;

  for (auto l : values) {
    sum += l;
    sumSquares += l * l;
  }

  double avg = sum / static_cast<double>(values.size());
  return make_shared<Average>(
      (sum / static_cast<double>(values.size())),
      sqrt(sumSquares / static_cast<double>(values.size()) - avg * avg));
}
} // namespace org::apache::lucene::search::suggest