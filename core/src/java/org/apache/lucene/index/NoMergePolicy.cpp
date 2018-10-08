using namespace std;

#include "NoMergePolicy.h"

namespace org::apache::lucene::index
{
using IOSupplier = org::apache::lucene::util::IOSupplier;
const shared_ptr<MergePolicy> NoMergePolicy::INSTANCE =
    make_shared<NoMergePolicy>();

NoMergePolicy::NoMergePolicy() : MergePolicy() {}

shared_ptr<MergeSpecification>
NoMergePolicy::findMerges(MergeTrigger mergeTrigger,
                          shared_ptr<SegmentInfos> segmentInfos,
                          shared_ptr<MergeContext> mergeContext)
{
  return nullptr;
}

shared_ptr<MergeSpecification> NoMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext)
{
  return nullptr;
}

shared_ptr<MergeSpecification>
NoMergePolicy::findForcedDeletesMerges(shared_ptr<SegmentInfos> segmentInfos,
                                       shared_ptr<MergeContext> mergeContext)
{
  return nullptr;
}

bool NoMergePolicy::useCompoundFile(shared_ptr<SegmentInfos> segments,
                                    shared_ptr<SegmentCommitInfo> newSegment,
                                    shared_ptr<MergeContext> mergeContext)
{
  return newSegment->info->getUseCompoundFile();
}

int64_t
NoMergePolicy::size(shared_ptr<SegmentCommitInfo> info,
                    shared_ptr<MergeContext> context) 
{
  return numeric_limits<int64_t>::max();
}

double NoMergePolicy::getNoCFSRatio() { return MergePolicy::getNoCFSRatio(); }

double NoMergePolicy::getMaxCFSSegmentSizeMB()
{
  return MergePolicy::getMaxCFSSegmentSizeMB();
}

void NoMergePolicy::setMaxCFSSegmentSizeMB(double v)
{
  MergePolicy::setMaxCFSSegmentSizeMB(v);
}

void NoMergePolicy::setNoCFSRatio(double noCFSRatio)
{
  MergePolicy::setNoCFSRatio(noCFSRatio);
}

bool NoMergePolicy::keepFullyDeletedSegment(
    IOSupplier<std::shared_ptr<CodecReader>>
        readerIOSupplier) 
{
  return MergePolicy::keepFullyDeletedSegment(readerIOSupplier);
}

int NoMergePolicy::numDeletesToMerge(
    shared_ptr<SegmentCommitInfo> info, int delCount,
    IOSupplier<std::shared_ptr<CodecReader>> readerSupplier) 
{
  return MergePolicy::numDeletesToMerge(info, delCount, readerSupplier);
}

wstring NoMergePolicy::toString() { return L"NoMergePolicy"; }
} // namespace org::apache::lucene::index