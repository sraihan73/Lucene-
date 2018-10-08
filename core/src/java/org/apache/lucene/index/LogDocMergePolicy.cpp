using namespace std;

#include "LogDocMergePolicy.h"

namespace org::apache::lucene::index
{

LogDocMergePolicy::LogDocMergePolicy()
{
  minMergeSize = DEFAULT_MIN_MERGE_DOCS;

  // maxMergeSize(ForForcedMerge) are never used by LogDocMergePolicy; set
  // it to Long.MAX_VALUE to disable it
  maxMergeSize = numeric_limits<int64_t>::max();
  maxMergeSizeForForcedMerge = numeric_limits<int64_t>::max();
}

int64_t LogDocMergePolicy::size(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  return sizeDocs(info, mergeContext);
}

void LogDocMergePolicy::setMinMergeDocs(int minMergeDocs)
{
  minMergeSize = minMergeDocs;
}

int LogDocMergePolicy::getMinMergeDocs()
{
  return static_cast<int>(minMergeSize);
}
} // namespace org::apache::lucene::index