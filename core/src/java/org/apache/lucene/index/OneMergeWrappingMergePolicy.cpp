using namespace std;

#include "OneMergeWrappingMergePolicy.h"

namespace org::apache::lucene::index
{

OneMergeWrappingMergePolicy::OneMergeWrappingMergePolicy(
    shared_ptr<MergePolicy> in_, function<OneMerge *(OneMerge *)> &wrapOneMerge)
    : FilterMergePolicy(in_), wrapOneMerge(wrapOneMerge)
{
}

shared_ptr<MergeSpecification> OneMergeWrappingMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return wrapSpec(in_->findMerges(mergeTrigger, segmentInfos, mergeContext));
}

shared_ptr<MergeSpecification> OneMergeWrappingMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  return wrapSpec(in_->findForcedMerges(segmentInfos, maxSegmentCount,
                                        segmentsToMerge, mergeContext));
}

shared_ptr<MergeSpecification>
OneMergeWrappingMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return wrapSpec(in_->findForcedDeletesMerges(segmentInfos, mergeContext));
}

shared_ptr<MergeSpecification>
OneMergeWrappingMergePolicy::wrapSpec(shared_ptr<MergeSpecification> spec)
{
  shared_ptr<MergeSpecification> wrapped =
      spec == nullptr ? nullptr : make_shared<MergeSpecification>();
  if (wrapped != nullptr) {
    for (auto merge : spec->merges) {
      wrapped->add(wrapOneMerge->apply(merge));
    }
  }
  return wrapped;
}
} // namespace org::apache::lucene::index