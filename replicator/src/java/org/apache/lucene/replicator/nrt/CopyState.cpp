using namespace std;

#include "CopyState.h"

namespace org::apache::lucene::replicator::nrt
{
using SegmentInfos = org::apache::lucene::index::SegmentInfos;

CopyState::CopyState(
    unordered_map<wstring, std::shared_ptr<FileMetaData>> &files,
    int64_t version, int64_t gen, std::deque<char> &infosBytes,
    shared_ptr<Set<wstring>> completedMergeFiles, int64_t primaryGen,
    shared_ptr<SegmentInfos> infos)
    : files(files), version(version), gen(gen),
      infosBytes(infosBytes),
      completedMergeFiles(Collections::unmodifiableSet(completedMergeFiles)),
      primaryGen(primaryGen), infos(infos)
{
  assert(completedMergeFiles != nullptr);
}

wstring CopyState::toString()
{
  return getClass().getSimpleName() + L"(version=" + to_wstring(version) + L")";
}
} // namespace org::apache::lucene::replicator::nrt