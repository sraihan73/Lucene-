using namespace std;

#include "TestForceMergePolicy.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestForceMergePolicy::testForceMergePolicy() 
{
  shared_ptr<MergePolicy> mp = make_shared<ForceMergePolicy>(nullptr);
  assertNull(mp->findMerges(
      nullptr, std::static_pointer_cast<SegmentInfos>(nullptr), nullptr));
}
} // namespace org::apache::lucene::index