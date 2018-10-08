using namespace std;

#include "TestUpgradeIndexMergePolicy.h"

namespace org::apache::lucene::index
{

shared_ptr<MergePolicy> TestUpgradeIndexMergePolicy::mergePolicy()
{
  return make_shared<UpgradeIndexMergePolicy>(newMergePolicy(random()));
}
} // namespace org::apache::lucene::index