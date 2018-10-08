using namespace std;

#include "UpgradeIndexMergePolicy.h"

namespace org::apache::lucene::index
{
using Version = org::apache::lucene::util::Version;

UpgradeIndexMergePolicy::UpgradeIndexMergePolicy(shared_ptr<MergePolicy> in_)
    : FilterMergePolicy(in_)
{
}

bool UpgradeIndexMergePolicy::shouldUpgradeSegment(
    shared_ptr<SegmentCommitInfo> si)
{
  return !Version::LATEST->equals(si->info->getVersion());
}

shared_ptr<MergeSpecification> UpgradeIndexMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return in_->findMerges(nullptr, segmentInfos, mergeContext);
}

shared_ptr<MergeSpecification> UpgradeIndexMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  // first find all old segments
  const unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> oldSegments =
      unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>();
  for (auto si : segmentInfos) {
    const optional<bool> v = segmentsToMerge[si];
    if (v && shouldUpgradeSegment(si)) {
      oldSegments.emplace(si, v);
    }
  }

  if (verbose(mergeContext)) {
    message(L"findForcedMerges: segmentsToUpgrade=" + oldSegments,
            mergeContext);
  }

  if (oldSegments.empty()) {
    return nullptr;
  }

  shared_ptr<MergeSpecification> spec = in_->findForcedMerges(
      segmentInfos, maxSegmentCount, oldSegments, mergeContext);

  if (spec != nullptr) {
    // remove all segments that are in merge specification from oldSegments,
    // the resulting set contains all segments that are left over
    // and will be merged to one additional segment:
    for (auto om : spec->merges) {
      oldSegments.keySet().removeAll(om->segments);
    }
  }

  if (!oldSegments.empty()) {
    if (verbose(mergeContext)) {
      message(L"findForcedMerges: " + in_->getClass().getSimpleName() +
                  L" does not want to merge all old segments, merge remaining "
                  L"ones into new segment: " +
                  oldSegments,
              mergeContext);
    }
    const deque<std::shared_ptr<SegmentCommitInfo>> newInfos =
        deque<std::shared_ptr<SegmentCommitInfo>>();
    for (auto si : segmentInfos) {
      if (oldSegments.find(si) != oldSegments.end()) {
        newInfos.push_back(si);
      }
    }
    // add the final merge
    if (spec == nullptr) {
      spec = make_shared<MergeSpecification>();
    }
    spec->add(make_shared<OneMerge>(newInfos));
  }

  return spec;
}
} // namespace org::apache::lucene::index