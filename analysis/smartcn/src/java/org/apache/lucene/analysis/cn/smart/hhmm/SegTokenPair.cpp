using namespace std;

#include "SegTokenPair.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{

SegTokenPair::SegTokenPair(std::deque<wchar_t> &idArray, int from, int to,
                           double weight)
{
  this->charArray = idArray;
  this->from = from;
  this->to = to;
  this->weight = weight;
}

int SegTokenPair::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  for (int i = 0; i < charArray.size(); i++) {
    result = prime * result + charArray[i];
  }
  result = prime * result + from;
  result = prime * result + to;
  int64_t temp;
  temp = Double::doubleToLongBits(weight);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

bool SegTokenPair::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<SegTokenPair> other = any_cast<std::shared_ptr<SegTokenPair>>(obj);
  if (!Arrays::equals(charArray, other->charArray)) {
    return false;
  }
  if (from != other->from) {
    return false;
  }
  if (to != other->to) {
    return false;
  }
  if (Double::doubleToLongBits(weight) !=
      Double::doubleToLongBits(other->weight)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm