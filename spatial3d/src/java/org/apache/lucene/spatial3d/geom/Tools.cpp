using namespace std;

#include "Tools.h"

namespace org::apache::lucene::spatial3d::geom
{

Tools::Tools() {}

double Tools::safeAcos(double value)
{
  if (value > 1.0) {
    value = 1.0;
  } else if (value < -1.0) {
    value = -1.0;
  }
  return acos(value);
}
} // namespace org::apache::lucene::spatial3d::geom