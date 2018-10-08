using namespace std;

#include "TestLogMergePolicy.h"

namespace org::apache::lucene::index
{

shared_ptr<MergePolicy> TestLogMergePolicy::mergePolicy()
{
  return newLogMergePolicy(random());
}

void TestLogMergePolicy::testDefaultForcedMergeMB()
{
  shared_ptr<LogByteSizeMergePolicy> mp = make_shared<LogByteSizeMergePolicy>();
  assertTrue(mp->getMaxMergeMBForForcedMerge() > 0.0);
}
} // namespace org::apache::lucene::index