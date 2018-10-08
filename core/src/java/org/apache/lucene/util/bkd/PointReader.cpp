using namespace std;

#include "PointReader.h"

namespace org::apache::lucene::util::bkd
{
using LongBitSet = org::apache::lucene::util::LongBitSet;

void PointReader::markOrds(int64_t count,
                           shared_ptr<LongBitSet> ordBitSet) 
{
  for (int i = 0; i < count; i++) {
    bool result = next();
    if (result == false) {
      throw make_shared<IllegalStateException>(
          L"did not see enough points from reader=" + shared_from_this());
    }
    assert((ordBitSet->get(ord()) == false, L"ord=" + to_wstring(ord()) +
                                                L" was seen twice from " +
                                                shared_from_this()));
    ordBitSet->set(ord());
  }
}

int64_t PointReader::split(int64_t count, shared_ptr<LongBitSet> rightTree,
                             shared_ptr<PointWriter> left,
                             shared_ptr<PointWriter> right,
                             bool doClearBits) 
{

  // Partition this source according to how the splitDim split the values:
  int64_t rightCount = 0;
  for (int64_t i = 0; i < count; i++) {
    bool result = next();
    assert(result);
    std::deque<char> packedValue = this->packedValue();
    int64_t ord = this->ord();
    int docID = this->docID();
    if (rightTree->get(ord)) {
      right->append(packedValue, ord, docID);
      rightCount++;
      if (doClearBits) {
        rightTree->clear(ord);
      }
    } else {
      left->append(packedValue, ord, docID);
    }
  }

  return rightCount;
}
} // namespace org::apache::lucene::util::bkd