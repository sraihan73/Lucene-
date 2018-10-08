using namespace std;

#include "FilterMergePolicy.h"

namespace org::apache::lucene::index
{
using IOSupplier = org::apache::lucene::util::IOSupplier;

FilterMergePolicy::FilterMergePolicy(shared_ptr<MergePolicy> in_) : in_(in_) {}

shared_ptr<MergeSpecification> FilterMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return in_->findMerges(mergeTrigger, segmentInfos, mergeContext);
}

shared_ptr<MergeSpecification> FilterMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  return in_->findForcedMerges(segmentInfos, maxSegmentCount, segmentsToMerge,
                               mergeContext);
}

shared_ptr<MergeSpecification> FilterMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return in_->findForcedDeletesMerges(segmentInfos, mergeContext);
}

bool FilterMergePolicy::useCompoundFile(
    shared_ptr<SegmentInfos> infos, shared_ptr<SegmentCommitInfo> mergedInfo,
    shared_ptr<MergeContext> mergeContext) 
{
  return in_->useCompoundFile(infos, mergedInfo, mergeContext);
}

int64_t
FilterMergePolicy::size(shared_ptr<SegmentCommitInfo> info,
                        shared_ptr<MergeContext> context) 
{
  return in_->size(info, context);
}

double FilterMergePolicy::getNoCFSRatio() { return in_->getNoCFSRatio(); }

void FilterMergePolicy::setNoCFSRatio(double noCFSRatio)
{
  in_->setNoCFSRatio(noCFSRatio);
}

void FilterMergePolicy::setMaxCFSSegmentSizeMB(double v)
{
  in_->setMaxCFSSegmentSizeMB(v);
}

double FilterMergePolicy::getMaxCFSSegmentSizeMB()
{
  return in_->getMaxCFSSegmentSizeMB();
}

wstring FilterMergePolicy::toString()
{
  return getClass().getSimpleName() + L"(" + in_ + L")";
}

bool FilterMergePolicy::keepFullyDeletedSegment(
    IOSupplier<std::shared_ptr<CodecReader>>
        readerIOSupplier) 
{
  return in_->keepFullyDeletedSegment(readerIOSupplier);
}

int FilterMergePolicy::numDeletesToMerge(
    shared_ptr<SegmentCommitInfo> info, int delCount,
    IOSupplier<std::shared_ptr<CodecReader>> readerSupplier) 
{
  return in_->numDeletesToMerge(info, delCount, readerSupplier);
}
} // namespace org::apache::lucene::index