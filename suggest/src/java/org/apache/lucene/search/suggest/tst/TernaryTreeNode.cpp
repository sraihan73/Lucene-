using namespace std;

#include "TernaryTreeNode.h"

namespace org::apache::lucene::search::suggest::tst
{
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

TernaryTreeNode::TernaryTreeNode() {}

int64_t TernaryTreeNode::sizeInBytes()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this());
  if (loKid != nullptr) {
    mem += loKid->sizeInBytes();
  }
  if (eqKid != nullptr) {
    mem += eqKid->sizeInBytes();
  }
  if (hiKid != nullptr) {
    mem += hiKid->sizeInBytes();
  }
  if (token != L"") {
    mem += RamUsageEstimator::shallowSizeOf(token) +
           RamUsageEstimator::NUM_BYTES_ARRAY_HEADER +
           Character::BYTES * token.length();
  }
  mem += RamUsageEstimator::shallowSizeOf(val);
  return mem;
}
} // namespace org::apache::lucene::search::suggest::tst