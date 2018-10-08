using namespace std;

#include "LogByteSizeMergePolicy.h"

namespace org::apache::lucene::index
{

const double LogByteSizeMergePolicy::DEFAULT_MAX_MERGE_MB_FOR_FORCED_MERGE =
    numeric_limits<int64_t>::max();

LogByteSizeMergePolicy::LogByteSizeMergePolicy()
{
  minMergeSize = static_cast<int64_t>(DEFAULT_MIN_MERGE_MB * 1024 * 1024);
  maxMergeSize = static_cast<int64_t>(DEFAULT_MAX_MERGE_MB * 1024 * 1024);
  // NOTE: in Java, if you cast a too-large double to long, as we are doing
  // here, then it becomes Long.MAX_VALUE
  maxMergeSizeForForcedMerge = static_cast<int64_t>(
      DEFAULT_MAX_MERGE_MB_FOR_FORCED_MERGE * 1024 * 1024);
}

int64_t LogByteSizeMergePolicy::size(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  return sizeBytes(info, mergeContext);
}

void LogByteSizeMergePolicy::setMaxMergeMB(double mb)
{
  maxMergeSize = static_cast<int64_t>(mb * 1024 * 1024);
}

double LogByteSizeMergePolicy::getMaxMergeMB()
{
  return (static_cast<double>(maxMergeSize)) / 1024 / 1024;
}

void LogByteSizeMergePolicy::setMaxMergeMBForForcedMerge(double mb)
{
  maxMergeSizeForForcedMerge = static_cast<int64_t>(mb * 1024 * 1024);
}

double LogByteSizeMergePolicy::getMaxMergeMBForForcedMerge()
{
  return (static_cast<double>(maxMergeSizeForForcedMerge)) / 1024 / 1024;
}

void LogByteSizeMergePolicy::setMinMergeMB(double mb)
{
  minMergeSize = static_cast<int64_t>(mb * 1024 * 1024);
}

double LogByteSizeMergePolicy::getMinMergeMB()
{
  return (static_cast<double>(minMergeSize)) / 1024 / 1024;
}
} // namespace org::apache::lucene::index