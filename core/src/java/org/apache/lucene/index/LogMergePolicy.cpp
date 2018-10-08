using namespace std;

#include "LogMergePolicy.h"

namespace org::apache::lucene::index
{

LogMergePolicy::LogMergePolicy()
    : MergePolicy(DEFAULT_NO_CFS_RATIO,
                  MergePolicy::DEFAULT_MAX_CFS_SEGMENT_SIZE)
{
}

int LogMergePolicy::getMergeFactor() { return mergeFactor; }

void LogMergePolicy::setMergeFactor(int mergeFactor)
{
  if (mergeFactor < 2) {
    throw invalid_argument(L"mergeFactor cannot be less than 2");
  }
  this->mergeFactor = mergeFactor;
}

void LogMergePolicy::setCalibrateSizeByDeletes(bool calibrateSizeByDeletes)
{
  this->calibrateSizeByDeletes = calibrateSizeByDeletes;
}

bool LogMergePolicy::getCalibrateSizeByDeletes()
{
  return calibrateSizeByDeletes;
}

int64_t LogMergePolicy::sizeDocs(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  if (calibrateSizeByDeletes) {
    int delCount = mergeContext->numDeletesToMerge(info);
    assert((assertDelCount(delCount, info)));
    return (info->info->maxDoc() - static_cast<int64_t>(delCount));
  } else {
    return info->info->maxDoc();
  }
}

int64_t LogMergePolicy::sizeBytes(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  if (calibrateSizeByDeletes) {
    return MergePolicy::size(info, mergeContext);
  }
  return info->sizeInBytes();
}

bool LogMergePolicy::isMerged(
    shared_ptr<SegmentInfos> infos, int maxNumSegments,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  constexpr int numSegments = infos->size();
  int numToMerge = 0;
  shared_ptr<SegmentCommitInfo> mergeInfo = nullptr;
  bool segmentIsOriginal = false;
  for (int i = 0; i < numSegments && numToMerge <= maxNumSegments; i++) {
    shared_ptr<SegmentCommitInfo> *const info = infos->info(i);
    const optional<bool> isOriginal = segmentsToMerge[info];
    if (isOriginal) {
      segmentIsOriginal = isOriginal;
      numToMerge++;
      mergeInfo = info;
    }
  }

  return numToMerge <= maxNumSegments &&
         (numToMerge != 1 || !segmentIsOriginal ||
          isMerged(infos, mergeInfo, mergeContext));
}

shared_ptr<MergeSpecification> LogMergePolicy::findForcedMergesSizeLimit(
    shared_ptr<SegmentInfos> infos, int last,
    shared_ptr<MergeContext> mergeContext) 
{
  shared_ptr<MergeSpecification> spec = make_shared<MergeSpecification>();
  const deque<std::shared_ptr<SegmentCommitInfo>> segments = infos->asList();

  int start = last - 1;
  while (start >= 0) {
    shared_ptr<SegmentCommitInfo> info = infos->info(start);
    if (size(info, mergeContext) > maxMergeSizeForForcedMerge ||
        sizeDocs(info, mergeContext) > maxMergeDocs) {
      if (verbose(mergeContext)) {
        message(L"findForcedMergesSizeLimit: skip segment=" + info +
                    L": size is > maxMergeSize (" +
                    to_wstring(maxMergeSizeForForcedMerge) +
                    L") or sizeDocs is > maxMergeDocs (" +
                    to_wstring(maxMergeDocs) + L")",
                mergeContext);
      }
      // need to skip that segment + add a merge for the 'right' segments,
      // unless there is only 1 which is merged.
      if (last - start - 1 > 1 ||
          (start != last - 1 &&
           !isMerged(infos, infos->info(start + 1), mergeContext))) {
        // there is more than 1 segment to the right of
        // this one, or a mergeable single segment.
        spec->add(make_shared<OneMerge>(segments.subList(start + 1, last)));
      }
      last = start;
    } else if (last - start == mergeFactor) {
      // mergeFactor eligible segments were found, add them as a merge.
      spec->add(make_shared<OneMerge>(segments.subList(start, last)));
      last = start;
    }
    --start;
  }

  // Add any left-over segments, unless there is just 1
  // already fully merged
  if (last > 0 && (++start + 1 < last ||
                   !isMerged(infos, infos->info(start), mergeContext))) {
    spec->add(make_shared<OneMerge>(segments.subList(start, last)));
  }

  return spec->merges.empty() ? nullptr : spec;
}

shared_ptr<MergeSpecification> LogMergePolicy::findForcedMergesMaxNumSegments(
    shared_ptr<SegmentInfos> infos, int maxNumSegments, int last,
    shared_ptr<MergeContext> mergeContext) 
{
  shared_ptr<MergeSpecification> spec = make_shared<MergeSpecification>();
  const deque<std::shared_ptr<SegmentCommitInfo>> segments = infos->asList();

  // First, enroll all "full" merges (size
  // mergeFactor) to potentially be run concurrently:
  while (last - maxNumSegments + 1 >= mergeFactor) {
    spec->add(
        make_shared<OneMerge>(segments.subList(last - mergeFactor, last)));
    last -= mergeFactor;
  }

  // Only if there are no full merges pending do we
  // add a final partial (< mergeFactor segments) merge:
  if (0 == spec->merges.size()) {
    if (maxNumSegments == 1) {

      // Since we must merge down to 1 segment, the
      // choice is simple:
      if (last > 1 || !isMerged(infos, infos->info(0), mergeContext)) {
        spec->add(make_shared<OneMerge>(segments.subList(0, last)));
      }
    } else if (last > maxNumSegments) {

      // Take care to pick a partial merge that is
      // least cost, but does not make the index too
      // lopsided.  If we always just picked the
      // partial tail then we could produce a highly
      // lopsided index over time:

      // We must merge this many segments to leave
      // maxNumSegments in the index (from when
      // forceMerge was first kicked off):
      constexpr int finalMergeSize = last - maxNumSegments + 1;

      // Consider all possible starting points:
      int64_t bestSize = 0;
      int bestStart = 0;

      for (int i = 0; i < last - finalMergeSize + 1; i++) {
        int64_t sumSize = 0;
        for (int j = 0; j < finalMergeSize; j++) {
          sumSize += size(infos->info(j + i), mergeContext);
        }
        if (i == 0 || (sumSize < 2 * size(infos->info(i - 1), mergeContext) &&
                       sumSize < bestSize)) {
          bestStart = i;
          bestSize = sumSize;
        }
      }

      spec->add(make_shared<OneMerge>(
          segments.subList(bestStart, bestStart + finalMergeSize)));
    }
  }
  return spec->merges.empty() ? nullptr : spec;
}

shared_ptr<MergeSpecification> LogMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> infos, int maxNumSegments,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{

  assert(maxNumSegments > 0);
  if (verbose(mergeContext)) {
    message(L"findForcedMerges: maxNumSegs=" + to_wstring(maxNumSegments) +
                L" segsToMerge=" + segmentsToMerge,
            mergeContext);
  }

  // If the segments are already merged (e.g. there's only 1 segment), or
  // there are <maxNumSegments:.
  if (isMerged(infos, maxNumSegments, segmentsToMerge, mergeContext)) {
    if (verbose(mergeContext)) {
      message(L"already merged; skip", mergeContext);
    }
    return nullptr;
  }

  // Find the newest (rightmost) segment that needs to
  // be merged (other segments may have been flushed
  // since merging started):
  int last = infos->size();
  while (last > 0) {
    shared_ptr<SegmentCommitInfo> *const info = infos->info(--last);
    if (segmentsToMerge[info] != nullptr) {
      last++;
      break;
    }
  }

  if (last == 0) {
    if (verbose(mergeContext)) {
      message(L"last == 0; skip", mergeContext);
    }
    return nullptr;
  }

  // There is only one segment already, and it is merged
  if (maxNumSegments == 1 && last == 1 &&
      isMerged(infos, infos->info(0), mergeContext)) {
    if (verbose(mergeContext)) {
      message(L"already 1 seg; skip", mergeContext);
    }
    return nullptr;
  }

  // Check if there are any segments above the threshold
  bool anyTooLarge = false;
  for (int i = 0; i < last; i++) {
    shared_ptr<SegmentCommitInfo> info = infos->info(i);
    if (size(info, mergeContext) > maxMergeSizeForForcedMerge ||
        sizeDocs(info, mergeContext) > maxMergeDocs) {
      anyTooLarge = true;
      break;
    }
  }

  if (anyTooLarge) {
    return findForcedMergesSizeLimit(infos, last, mergeContext);
  } else {
    return findForcedMergesMaxNumSegments(infos, maxNumSegments, last,
                                          mergeContext);
  }
}

shared_ptr<MergeSpecification> LogMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  const deque<std::shared_ptr<SegmentCommitInfo>> segments =
      segmentInfos->asList();
  constexpr int numSegments = segments.size();

  if (verbose(mergeContext)) {
    message(L"findForcedDeleteMerges: " + to_wstring(numSegments) +
                L" segments",
            mergeContext);
  }

  shared_ptr<MergeSpecification> spec = make_shared<MergeSpecification>();
  int firstSegmentWithDeletions = -1;
  assert(mergeContext != nullptr);
  for (int i = 0; i < numSegments; i++) {
    shared_ptr<SegmentCommitInfo> *const info = segmentInfos->info(i);
    int delCount = mergeContext->numDeletesToMerge(info);
    assert((assertDelCount(delCount, info)));
    if (delCount > 0) {
      if (verbose(mergeContext)) {
        message(L"  segment " + info->info->name + L" has deletions",
                mergeContext);
      }
      if (firstSegmentWithDeletions == -1) {
        firstSegmentWithDeletions = i;
      } else if (i - firstSegmentWithDeletions == mergeFactor) {
        // We've seen mergeFactor segments in a row with
        // deletions, so force a merge now:
        if (verbose(mergeContext)) {
          message(L"  add merge " + to_wstring(firstSegmentWithDeletions) +
                      L" to " + to_wstring(i - 1) + L" inclusive",
                  mergeContext);
        }
        spec->add(make_shared<OneMerge>(
            segments.subList(firstSegmentWithDeletions, i)));
        firstSegmentWithDeletions = i;
      }
    } else if (firstSegmentWithDeletions != -1) {
      // End of a sequence of segments with deletions, so,
      // merge those past segments even if it's fewer than
      // mergeFactor segments
      if (verbose(mergeContext)) {
        message(L"  add merge " + to_wstring(firstSegmentWithDeletions) +
                    L" to " + to_wstring(i - 1) + L" inclusive",
                mergeContext);
      }
      spec->add(make_shared<OneMerge>(
          segments.subList(firstSegmentWithDeletions, i)));
      firstSegmentWithDeletions = -1;
    }
  }

  if (firstSegmentWithDeletions != -1) {
    if (verbose(mergeContext)) {
      message(L"  add merge " + to_wstring(firstSegmentWithDeletions) +
                  L" to " + to_wstring(numSegments - 1) + L" inclusive",
              mergeContext);
    }
    spec->add(make_shared<OneMerge>(
        segments.subList(firstSegmentWithDeletions, numSegments)));
  }

  return spec;
}

LogMergePolicy::SegmentInfoAndLevel::SegmentInfoAndLevel(
    shared_ptr<SegmentCommitInfo> info, float level)
{
  this->info = info;
  this->level = level;
}

int LogMergePolicy::SegmentInfoAndLevel::compareTo(
    shared_ptr<SegmentInfoAndLevel> other)
{
  return Float::compare(other->level, level);
}

shared_ptr<MergeSpecification> LogMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> infos,
    shared_ptr<MergeContext> mergeContext) 
{

  constexpr int numSegments = infos->size();
  if (verbose(mergeContext)) {
    message(L"findMerges: " + to_wstring(numSegments) + L" segments",
            mergeContext);
  }

  // Compute levels, which is just log (base mergeFactor)
  // of the size of each segment
  const deque<std::shared_ptr<SegmentInfoAndLevel>> levels =
      deque<std::shared_ptr<SegmentInfoAndLevel>>(numSegments);
  constexpr float norm = static_cast<float>(log(mergeFactor));

  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const mergingSegments =
      mergeContext->getMergingSegments();

  for (int i = 0; i < numSegments; i++) {
    shared_ptr<SegmentCommitInfo> *const info = infos->info(i);
    int64_t size = this->size(info, mergeContext);

    // Floor tiny segments
    if (size < 1) {
      size = 1;
    }

    shared_ptr<SegmentInfoAndLevel> *const infoLevel =
        make_shared<SegmentInfoAndLevel>(info,
                                         static_cast<float>(log(size)) / norm);
    levels.push_back(infoLevel);

    if (verbose(mergeContext)) {
      constexpr int64_t segBytes = sizeBytes(info, mergeContext);
      wstring extra = mergingSegments->contains(info) ? L" [merging]" : L"";
      if (size >= maxMergeSize) {
        extra += L" [skip: too large]";
      }
      message(L"seg=" + segString(mergeContext, Collections::singleton(info)) +
                  L" level=" + to_wstring(infoLevel->level) + L" size=" +
                  wstring::format(Locale::ROOT, L"%.3f MB",
                                  segBytes / 1024 / 1024.0) +
                  extra,
              mergeContext);
    }
  }

  constexpr float levelFloor;
  if (minMergeSize <= 0) {
    levelFloor = static_cast<float>(0.0);
  } else {
    levelFloor = static_cast<float>(log(minMergeSize) / norm);
  }

  // Now, we quantize the log values into levels.  The
  // first level is any segment whose log size is within
  // LEVEL_LOG_SPAN of the max size, or, who has such as
  // segment "to the right".  Then, we find the max of all
  // other segments and use that to define the next level
  // segment, etc.

  shared_ptr<MergeSpecification> spec = nullptr;

  constexpr int numMergeableSegments = levels.size();

  int start = 0;
  while (start < numMergeableSegments) {

    // Find max level of all segments not already
    // quantized.
    float maxLevel = levels[start]->level;
    for (int i = 1 + start; i < numMergeableSegments; i++) {
      constexpr float level = levels[i]->level;
      if (level > maxLevel) {
        maxLevel = level;
      }
    }

    // Now search backwards for the rightmost segment that
    // falls into this level:
    float levelBottom;
    if (maxLevel <= levelFloor) {
      // All remaining segments fall into the min level
      levelBottom = -1.0F;
    } else {
      levelBottom = static_cast<float>(maxLevel - LEVEL_LOG_SPAN);

      // Force a boundary at the level floor
      if (levelBottom < levelFloor && maxLevel >= levelFloor) {
        levelBottom = levelFloor;
      }
    }

    int upto = numMergeableSegments - 1;
    while (upto >= start) {
      if (levels[upto]->level >= levelBottom) {
        break;
      }
      upto--;
    }
    if (verbose(mergeContext)) {
      message(L"  level " + to_wstring(levelBottom) + L" to " +
                  to_wstring(maxLevel) + L": " + to_wstring(1 + upto - start) +
                  L" segments",
              mergeContext);
    }

    // Finally, record all merges that are viable at this level:
    int end = start + mergeFactor;
    while (end <= 1 + upto) {
      bool anyTooLarge = false;
      bool anyMerging = false;
      for (int i = start; i < end; i++) {
        shared_ptr<SegmentCommitInfo> *const info = levels[i]->info;
        anyTooLarge |= (size(info, mergeContext) >= maxMergeSize ||
                        sizeDocs(info, mergeContext) >= maxMergeDocs);
        if (mergingSegments->contains(info)) {
          anyMerging = true;
          break;
        }
      }

      if (anyMerging) {
        // skip
      } else if (!anyTooLarge) {
        if (spec == nullptr) {
          spec = make_shared<MergeSpecification>();
        }
        const deque<std::shared_ptr<SegmentCommitInfo>> mergeInfos =
            deque<std::shared_ptr<SegmentCommitInfo>>(end - start);
        for (int i = start; i < end; i++) {
          mergeInfos.push_back(levels[i]->info);
              assert((find(infos->begin(), infos->end(), levels[i]->info) != infos->end().info)));
        }
        if (verbose(mergeContext)) {
          message(L"  add merge=" + segString(mergeContext, mergeInfos) +
                      L" start=" + to_wstring(start) + L" end=" +
                      to_wstring(end),
                  mergeContext);
        }
        spec->add(make_shared<OneMerge>(mergeInfos));
      } else if (verbose(mergeContext)) {
        message(L"    " + to_wstring(start) + L" to " + to_wstring(end) +
                    L": contains segment over maxMergeSize or maxMergeDocs; "
                    L"skipping",
                mergeContext);
      }

      start = end;
      end = start + mergeFactor;
    }

    start = 1 + upto;
  }

  return spec;
}

void LogMergePolicy::setMaxMergeDocs(int maxMergeDocs)
{
  this->maxMergeDocs = maxMergeDocs;
}

int LogMergePolicy::getMaxMergeDocs() { return maxMergeDocs; }

wstring LogMergePolicy::toString()
{
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(L"[" + getClass().getSimpleName() + L": ");
  sb->append(L"minMergeSize=")->append(minMergeSize)->append(L", ");
  sb->append(L"mergeFactor=")->append(mergeFactor)->append(L", ");
  sb->append(L"maxMergeSize=")->append(maxMergeSize)->append(L", ");
  sb->append(L"maxMergeSizeForForcedMerge=")
      ->append(maxMergeSizeForForcedMerge)
      ->append(L", ");
  sb->append(L"calibrateSizeByDeletes=")
      ->append(calibrateSizeByDeletes)
      ->append(L", ");
  sb->append(L"maxMergeDocs=")->append(maxMergeDocs)->append(L", ");
  sb->append(L"maxCFSSegmentSizeMB=")
      ->append(getMaxCFSSegmentSizeMB())
      ->append(L", ");
  sb->append(L"noCFSRatio=")->append(noCFSRatio);
  sb->append(L"]");
  return sb->toString();
}
} // namespace org::apache::lucene::index