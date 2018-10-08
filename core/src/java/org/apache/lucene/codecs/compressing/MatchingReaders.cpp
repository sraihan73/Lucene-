using namespace std;

#include "MatchingReaders.h"
#include "../../index/FieldInfo.h"
#include "../../index/MergeState.h"

namespace org::apache::lucene::codecs::compressing
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentReader = org::apache::lucene::index::SegmentReader;

MatchingReaders::MatchingReaders(shared_ptr<MergeState> mergeState)
    : matchingReaders(std::deque<bool>(numReaders)), count(matchedCount)
{
  // If the i'th reader is a SegmentReader and has
  // identical fieldName -> number mapping, then this
  // array will be non-null at position i:
  int numReaders = mergeState->maxDocs.size();
  int matchedCount = 0;

  // If this reader is a SegmentReader, and all of its
  // field name -> number mappings match the "merged"
  // FieldInfos, then we can do a bulk copy of the
  // stored fields:

  for (int i = 0; i < numReaders; i++) {
    for (auto fi : mergeState->fieldInfos[i]) {
      shared_ptr<FieldInfo> other =
          mergeState->mergeFieldInfos->fieldInfo(fi->number);
      if (other == nullptr || other->name != fi->name) {
        goto nextReaderContinue;
      }
    }
    matchingReaders[i] = true;
    matchedCount++;
  nextReaderContinue:;
  }
nextReaderBreak:

  if (mergeState->infoStream->isEnabled(L"SM")) {
    mergeState->infoStream->message(L"SM", L"merge store matchedCount=" +
                                               to_wstring(count) + L" vs " +
                                               to_wstring(numReaders));
    if (count != numReaders) {
      mergeState->infoStream->message(
          L"SM", L"" + to_wstring(numReaders - count) + L" non-bulk merges");
    }
  }
}
} // namespace org::apache::lucene::codecs::compressing