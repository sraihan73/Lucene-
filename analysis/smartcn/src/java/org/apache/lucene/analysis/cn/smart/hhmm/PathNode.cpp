using namespace std;

#include "PathNode.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{

int PathNode::compareTo(shared_ptr<PathNode> pn)
{
  if (weight < pn->weight) {
    return -1;
  } else if (weight == pn->weight) {
    return 0;
  } else {
    return 1;
  }
}

int PathNode::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + preNode;
  int64_t temp;
  temp = Double::doubleToLongBits(weight);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

bool PathNode::equals(any obj)
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
  shared_ptr<PathNode> other = any_cast<std::shared_ptr<PathNode>>(obj);
  if (preNode != other->preNode) {
    return false;
  }
  if (Double::doubleToLongBits(weight) !=
      Double::doubleToLongBits(other->weight)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm