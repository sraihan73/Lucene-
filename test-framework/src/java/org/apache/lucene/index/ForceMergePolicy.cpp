using namespace std;

#include "ForceMergePolicy.h"

namespace org::apache::lucene::index
{

ForceMergePolicy::ForceMergePolicy(shared_ptr<MergePolicy> in_)
    : FilterMergePolicy(in_)
{
}

shared_ptr<MergeSpecification> ForceMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return nullptr;
}
} // namespace org::apache::lucene::index