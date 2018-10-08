using namespace std;

#include "TieredMergePolicy.h"

namespace org::apache::lucene::index
{

TieredMergePolicy::TieredMergePolicy()
    : MergePolicy(DEFAULT_NO_CFS_RATIO,
                  MergePolicy::DEFAULT_MAX_CFS_SEGMENT_SIZE)
{
}

shared_ptr<TieredMergePolicy> TieredMergePolicy::setMaxMergeAtOnce(int v)
{
  if (v < 2) {
    throw invalid_argument(L"maxMergeAtOnce must be > 1 (got " + to_wstring(v) +
                           L")");
  }
  maxMergeAtOnce = v;
  return shared_from_this();
}

int TieredMergePolicy::getMaxMergeAtOnce() { return maxMergeAtOnce; }

shared_ptr<TieredMergePolicy>
TieredMergePolicy::setMaxMergeAtOnceExplicit(int v)
{
  if (v < 2) {
    throw invalid_argument(L"maxMergeAtOnceExplicit must be > 1 (got " +
                           to_wstring(v) + L")");
  }
  maxMergeAtOnceExplicit = v;
  return shared_from_this();
}

int TieredMergePolicy::getMaxMergeAtOnceExplicit()
{
  return maxMergeAtOnceExplicit;
}

shared_ptr<TieredMergePolicy> TieredMergePolicy::setMaxMergedSegmentMB(double v)
{
  if (v < 0.0) {
    throw invalid_argument(L"maxMergedSegmentMB must be >=0 (got " +
                           to_wstring(v) + L")");
  }
  v *= 1024 * 1024;
  maxMergedSegmentBytes = v > numeric_limits<int64_t>::max()
                              ? numeric_limits<int64_t>::max()
                              : static_cast<int64_t>(v);
  return shared_from_this();
}

double TieredMergePolicy::getMaxMergedSegmentMB()
{
  return maxMergedSegmentBytes / 1024 / 1024.0;
}

shared_ptr<TieredMergePolicy>
TieredMergePolicy::setReclaimDeletesWeight(double v)
{
  if (v < 0.0) {
    throw invalid_argument(L"reclaimDeletesWeight must be >= 0.0 (got " +
                           to_wstring(v) + L")");
  }
  reclaimDeletesWeight = v;
  return shared_from_this();
}

double TieredMergePolicy::getReclaimDeletesWeight()
{
  return reclaimDeletesWeight;
}

shared_ptr<TieredMergePolicy> TieredMergePolicy::setFloorSegmentMB(double v)
{
  if (v <= 0.0) {
    throw invalid_argument(L"floorSegmentMB must be > 0.0 (got " +
                           to_wstring(v) + L")");
  }
  v *= 1024 * 1024;
  floorSegmentBytes = v > numeric_limits<int64_t>::max()
                          ? numeric_limits<int64_t>::max()
                          : static_cast<int64_t>(v);
  return shared_from_this();
}

double TieredMergePolicy::getFloorSegmentMB()
{
  return floorSegmentBytes / (1024 * 1024.0);
}

shared_ptr<TieredMergePolicy>
TieredMergePolicy::setForceMergeDeletesPctAllowed(double v)
{
  if (v < 0.0 || v > 100.0) {
    throw invalid_argument(L"forceMergeDeletesPctAllowed must be between 0.0 "
                           L"and 100.0 inclusive (got " +
                           to_wstring(v) + L")");
  }
  forceMergeDeletesPctAllowed = v;
  return shared_from_this();
}

double TieredMergePolicy::getForceMergeDeletesPctAllowed()
{
  return forceMergeDeletesPctAllowed;
}

shared_ptr<TieredMergePolicy> TieredMergePolicy::setSegmentsPerTier(double v)
{
  if (v < 2.0) {
    throw invalid_argument(L"segmentsPerTier must be >= 2.0 (got " +
                           to_wstring(v) + L")");
  }
  segsPerTier = v;
  return shared_from_this();
}

double TieredMergePolicy::getSegmentsPerTier() { return segsPerTier; }

TieredMergePolicy::SegmentByteSizeDescending::SegmentByteSizeDescending(
    shared_ptr<TieredMergePolicy> outerInstance,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t> &sizeInBytes)
    : sizeInBytes(sizeInBytes), outerInstance(outerInstance)
{
}

int TieredMergePolicy::SegmentByteSizeDescending::compare(
    shared_ptr<SegmentCommitInfo> o1, shared_ptr<SegmentCommitInfo> o2)
{
  // Sort by largest size:
  int cmp = Long::compare(sizeInBytes[o2], sizeInBytes[o1]);
  if (cmp == 0) {
    cmp = o1->info->name.compare(o2->info->name);
  }
  return cmp;
}

TieredMergePolicy::MergeScore::MergeScore() {}

unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
TieredMergePolicy::getSegmentSizes(
    shared_ptr<MergeContext> mergeContext,
    shared_ptr<deque<std::shared_ptr<SegmentCommitInfo>>>
        infos) 
{
  unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t> sizeInBytes =
      unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>();
  for (auto info : infos) {
    sizeInBytes.emplace(info, size(info, mergeContext));
  }
  return sizeInBytes;
}

shared_ptr<MergeSpecification> TieredMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> infos,
    shared_ptr<MergeContext> mergeContext) 
{
  if (verbose(mergeContext)) {
    message(L"findMerges: " + to_wstring(infos->size()) + L" segments",
            mergeContext);
  }
  if (infos->empty()) {
    return nullptr;
  }
  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const merging =
      mergeContext->getMergingSegments();
  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const toBeMerged =
      unordered_set<std::shared_ptr<SegmentCommitInfo>>();

  const deque<std::shared_ptr<SegmentCommitInfo>> infosSorted =
      deque<std::shared_ptr<SegmentCommitInfo>>(infos->asList());

  // The size can change concurrently while we are running here, because deletes
  // are now applied concurrently, and this can piss off TimSort!  So we
  // call size() once per segment and sort by that:
  unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t> sizeInBytes =
      getSegmentSizes(mergeContext, infos->asList());

  infosSorted.sort(
      make_shared<SegmentByteSizeDescending>(shared_from_this(), sizeInBytes));

  // Compute total index bytes & print details about the index
  int64_t totIndexBytes = 0;
  int64_t minSegmentBytes = numeric_limits<int64_t>::max();
  for (auto info : infosSorted) {
    constexpr int64_t segBytes = sizeInBytes[info];
    if (verbose(mergeContext)) {
      wstring extra = merging->contains(info) ? L" [merging]" : L"";
      if (segBytes >= maxMergedSegmentBytes / 2.0) {
        extra += L" [skip: too large]";
      } else if (segBytes < floorSegmentBytes) {
        extra += L" [floored]";
      }
      message(
          L"  seg=" + segString(mergeContext, Collections::singleton(info)) +
              L" size=" +
              wstring::format(Locale::ROOT, L"%.3f", segBytes / 1024 / 1024.0) +
              L" MB" + extra,
          mergeContext);
    }

    minSegmentBytes = min(segBytes, minSegmentBytes);
    // Accum total byte size
    totIndexBytes += segBytes;
  }

  // If we have too-large segments, grace them out
  // of the maxSegmentCount:
  int tooBigCount = 0;
  while (tooBigCount < infosSorted.size()) {
    int64_t segBytes = sizeInBytes[infosSorted[tooBigCount]];
    if (segBytes < maxMergedSegmentBytes / 2.0) {
      break;
    }
    totIndexBytes -= segBytes;
    tooBigCount++;
  }

  minSegmentBytes = floorSize(minSegmentBytes);

  // Compute max allowed segs in the index
  int64_t levelSize = minSegmentBytes;
  int64_t bytesLeft = totIndexBytes;
  double allowedSegCount = 0;
  while (true) {
    constexpr double segCountLevel = bytesLeft / static_cast<double>(levelSize);
    if (segCountLevel < segsPerTier) {
      allowedSegCount += ceil(segCountLevel);
      break;
    }
    allowedSegCount += segsPerTier;
    bytesLeft -= segsPerTier * levelSize;
    levelSize *= maxMergeAtOnce;
  }
  int allowedSegCountInt = static_cast<int>(allowedSegCount);

  shared_ptr<MergeSpecification> spec = nullptr;

  // Cycle to possibly select more than one merge:
  while (true) {

    int64_t mergingBytes = 0;

    // Gather eligible segments for merging, ie segments
    // not already being merged and not already picked (by
    // prior iteration of this loop) for merging:
    const deque<std::shared_ptr<SegmentCommitInfo>> eligible =
        deque<std::shared_ptr<SegmentCommitInfo>>();
    for (int idx = tooBigCount; idx < infosSorted.size(); idx++) {
      shared_ptr<SegmentCommitInfo> *const info = infosSorted[idx];
      if (merging->contains(info)) {
        mergingBytes += sizeInBytes[info];
      } else if (!toBeMerged->contains(info)) {
        eligible.push_back(info);
      }
    }

    constexpr bool maxMergeIsRunning = mergingBytes >= maxMergedSegmentBytes;

    if (verbose(mergeContext)) {
      message(L"  allowedSegmentCount=" + to_wstring(allowedSegCountInt) +
                  L" vs count=" + infosSorted.size() + L" (eligible count=" +
                  eligible.size() + L") tooBigCount=" + to_wstring(tooBigCount),
              mergeContext);
    }

    if (eligible.empty()) {
      return spec;
    }

    if (eligible.size() > allowedSegCountInt) {

      // OK we are over budget -- find best merge!
      shared_ptr<MergeScore> bestScore = nullptr;
      deque<std::shared_ptr<SegmentCommitInfo>> best;
      bool bestTooLarge = false;
      int64_t bestMergeBytes = 0;

      // Consider all merge starts:
      for (int startIdx = 0; startIdx <= eligible.size() - maxMergeAtOnce;
           startIdx++) {

        int64_t totAfterMergeBytes = 0;

        const deque<std::shared_ptr<SegmentCommitInfo>> candidate =
            deque<std::shared_ptr<SegmentCommitInfo>>();
        bool hitTooLarge = false;
        for (int idx = startIdx;
             idx < eligible.size() && candidate.size() < maxMergeAtOnce;
             idx++) {
          shared_ptr<SegmentCommitInfo> *const info = eligible[idx];
          constexpr int64_t segBytes = sizeInBytes[info];

          if (totAfterMergeBytes + segBytes > maxMergedSegmentBytes) {
            hitTooLarge = true;
            // NOTE: we continue, so that we can try
            // "packing" smaller segments into this merge
            // to see if we can get closer to the max
            // size; this in general is not perfect since
            // this is really "bin packing" and we'd have
            // to try different permutations.
            continue;
          }
          candidate.push_back(info);
          totAfterMergeBytes += segBytes;
        }

        // We should never see an empty candidate: we iterated over
        // maxMergeAtOnce segments, and already pre-excluded the too-large
        // segments:
        assert(candidate.size() > 0);

        shared_ptr<MergeScore> *const score =
            this->score(candidate, hitTooLarge, sizeInBytes);
        if (verbose(mergeContext)) {
          message(L"  maybe=" + segString(mergeContext, candidate) +
                      L" score=" + to_wstring(score->getScore()) + L" " +
                      score->getExplanation() + L" tooLarge=" +
                      StringHelper::toString(hitTooLarge) + L" size=" +
                      wstring::format(Locale::ROOT, L"%.3f MB",
                                      totAfterMergeBytes / 1024.0 / 1024.0),
                  mergeContext);
        }

        // If we are already running a max sized merge
        // (maxMergeIsRunning), don't allow another max
        // sized merge to kick off:
        if ((bestScore == nullptr ||
             score->getScore() < bestScore->getScore()) &&
            (!hitTooLarge || !maxMergeIsRunning)) {
          best = candidate;
          bestScore = score;
          bestTooLarge = hitTooLarge;
          bestMergeBytes = totAfterMergeBytes;
        }
      }

      if (best.size() > 0) {
        if (spec == nullptr) {
          spec = make_shared<MergeSpecification>();
        }
        shared_ptr<OneMerge> *const merge = make_shared<OneMerge>(best);
        spec->add(merge);
        toBeMerged->addAll(merge->segments);

        if (verbose(mergeContext)) {
          message(L"  add merge=" + segString(mergeContext, merge->segments) +
                      L" size=" +
                      wstring::format(Locale::ROOT, L"%.3f MB",
                                      bestMergeBytes / 1024.0 / 1024.0) +
                      L" score=" +
                      wstring::format(Locale::ROOT, L"%.3f",
                                      bestScore->getScore()) +
                      L" " + bestScore->getExplanation() +
                      (bestTooLarge ? L" [max merge]" : L""),
                  mergeContext);
        }
      } else {
        return spec;
      }
    } else {
      return spec;
    }
  }
}

shared_ptr<MergeScore> TieredMergePolicy::score(
    deque<std::shared_ptr<SegmentCommitInfo>> &candidate, bool hitTooLarge,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
        &sizeInBytes) 
{
  int64_t totBeforeMergeBytes = 0;
  int64_t totAfterMergeBytes = 0;
  int64_t totAfterMergeBytesFloored = 0;
  for (auto info : candidate) {
    constexpr int64_t segBytes = sizeInBytes[info];
    totAfterMergeBytes += segBytes;
    totAfterMergeBytesFloored += floorSize(segBytes);
    totBeforeMergeBytes += info->sizeInBytes();
  }

  // Roughly measure "skew" of the merge, i.e. how
  // "balanced" the merge is (whether the segments are
  // about the same size), which can range from
  // 1.0/numSegsBeingMerged (good) to 1.0 (poor). Heavily
  // lopsided merges (skew near 1.0) is no good; it means
  // O(N^2) merge cost over time:
  constexpr double skew;
  if (hitTooLarge) {
    // Pretend the merge has perfect skew; skew doesn't
    // matter in this case because this merge will not
    // "cascade" and so it cannot lead to N^2 merge cost
    // over time:
    skew = 1.0 / maxMergeAtOnce;
  } else {
    skew = (static_cast<double>(floorSize(sizeInBytes[candidate[0]]))) /
           totAfterMergeBytesFloored;
  }

  // Strongly favor merges with less skew (smaller
  // mergeScore is better):
  double mergeScore = skew;

  // Gently favor smaller merges over bigger ones.  We
  // don't want to make this exponent too large else we
  // can end up doing poor merges of small segments in
  // order to avoid the large merges:
  mergeScore *= pow(totAfterMergeBytes, 0.05);

  // Strongly favor merges that reclaim deletes:
  constexpr double nonDelRatio =
      (static_cast<double>(totAfterMergeBytes)) / totBeforeMergeBytes;
  mergeScore *= pow(nonDelRatio, reclaimDeletesWeight);

  constexpr double finalMergeScore = mergeScore;

  return make_shared<MergeScoreAnonymousInnerClass>(
      shared_from_this(), skew, nonDelRatio, finalMergeScore);
}

TieredMergePolicy::MergeScoreAnonymousInnerClass::MergeScoreAnonymousInnerClass(
    shared_ptr<TieredMergePolicy> outerInstance, double skew,
    double nonDelRatio, double finalMergeScore)
{
  this->outerInstance = outerInstance;
  this->skew = skew;
  this->nonDelRatio = nonDelRatio;
  this->finalMergeScore = finalMergeScore;
}

double TieredMergePolicy::MergeScoreAnonymousInnerClass::getScore()
{
  return finalMergeScore;
}

wstring TieredMergePolicy::MergeScoreAnonymousInnerClass::getExplanation()
{
  return L"skew=" + wstring::format(Locale::ROOT, L"%.3f", skew) +
         L" nonDelRatio=" + wstring::format(Locale::ROOT, L"%.3f", nonDelRatio);
}

shared_ptr<MergeSpecification> TieredMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> infos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  if (verbose(mergeContext)) {
    message(L"findForcedMerges maxSegmentCount=" + to_wstring(maxSegmentCount) +
                L" infos=" + segString(mergeContext, infos) +
                L" segmentsToMerge=" + segmentsToMerge,
            mergeContext);
  }

  deque<std::shared_ptr<SegmentCommitInfo>> eligible =
      deque<std::shared_ptr<SegmentCommitInfo>>();
  bool forceMergeRunning = false;
  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const merging =
      mergeContext->getMergingSegments();
  bool segmentIsOriginal = false;
  for (auto info : infos) {
    const optional<bool> isOriginal = segmentsToMerge[info];
    if (isOriginal) {
      segmentIsOriginal = isOriginal;
      if (merging->contains(info) == false) {
        eligible.push_back(info);
      } else {
        forceMergeRunning = true;
      }
    }
  }

  if (eligible.empty()) {
    return nullptr;
  }

  // The size can change concurrently while we are running here, because deletes
  // are now applied concurrently, and this can piss off TimSort!  So we
  // call size() once per segment and sort by that:
  unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t> sizeInBytes =
      getSegmentSizes(mergeContext, eligible);

  if ((maxSegmentCount > 1 && eligible.size() <= maxSegmentCount) ||
      (maxSegmentCount == 1 && eligible.size() == 1 &&
       (!segmentIsOriginal || isMerged(infos, eligible[0], mergeContext)))) {
    if (verbose(mergeContext)) {
      message(L"already merged", mergeContext);
    }
    return nullptr;
  }

  eligible.sort(
      make_shared<SegmentByteSizeDescending>(shared_from_this(), sizeInBytes));

  if (verbose(mergeContext)) {
    message(L"eligible=" + eligible, mergeContext);
    message(L"forceMergeRunning=" + StringHelper::toString(forceMergeRunning),
            mergeContext);
  }

  int end = eligible.size();

  shared_ptr<MergeSpecification> spec = nullptr;

  // Do full merges, first, backwards:
  while (end >= maxMergeAtOnceExplicit + maxSegmentCount - 1) {
    if (spec == nullptr) {
      spec = make_shared<MergeSpecification>();
    }
    shared_ptr<OneMerge> *const merge = make_shared<OneMerge>(
        eligible.subList(end - maxMergeAtOnceExplicit, end));
    if (verbose(mergeContext)) {
      message(L"add merge=" + segString(mergeContext, merge->segments),
              mergeContext);
    }
    spec->add(merge);
    end -= maxMergeAtOnceExplicit;
  }

  if (spec == nullptr && !forceMergeRunning) {
    // Do final merge
    constexpr int numToMerge = end - maxSegmentCount + 1;
    shared_ptr<OneMerge> *const merge =
        make_shared<OneMerge>(eligible.subList(end - numToMerge, end));
    if (verbose(mergeContext)) {
      message(L"add final merge=" + merge->segString(), mergeContext);
    }
    spec = make_shared<MergeSpecification>();
    spec->add(merge);
  }

  return spec;
}

shared_ptr<MergeSpecification> TieredMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> infos,
    shared_ptr<MergeContext> mergeContext) 
{
  if (verbose(mergeContext)) {
    message(L"findForcedDeletesMerges infos=" + segString(mergeContext, infos) +
                L" forceMergeDeletesPctAllowed=" +
                to_wstring(forceMergeDeletesPctAllowed),
            mergeContext);
  }
  const deque<std::shared_ptr<SegmentCommitInfo>> eligible =
      deque<std::shared_ptr<SegmentCommitInfo>>();
  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const merging =
      mergeContext->getMergingSegments();
  for (auto info : infos) {
    int delCount = mergeContext->numDeletesToMerge(info);
    assert((assertDelCount(delCount, info)));
    double pctDeletes =
        100.0 * (static_cast<double>(delCount)) / info->info->maxDoc();
    if (pctDeletes > forceMergeDeletesPctAllowed && !merging->contains(info)) {
      eligible.push_back(info);
    }
  }

  if (eligible.empty()) {
    return nullptr;
  }

  // The size can change concurrently while we are running here, because deletes
  // are now applied concurrently, and this can piss off TimSort!  So we
  // call size() once per segment and sort by that:
  unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t> sizeInBytes =
      getSegmentSizes(mergeContext, infos->asList());

  eligible.sort(
      make_shared<SegmentByteSizeDescending>(shared_from_this(), sizeInBytes));

  if (verbose(mergeContext)) {
    message(L"eligible=" + eligible, mergeContext);
  }

  int start = 0;
  shared_ptr<MergeSpecification> spec = nullptr;

  while (start < eligible.size()) {
    // Don't enforce max merged size here: app is explicitly
    // calling forceMergeDeletes, and knows this may take a
    // long time / produce big segments (like forceMerge):
    constexpr int end = min(start + maxMergeAtOnceExplicit, eligible.size());
    if (spec == nullptr) {
      spec = make_shared<MergeSpecification>();
    }

    shared_ptr<OneMerge> *const merge =
        make_shared<OneMerge>(eligible.subList(start, end));
    if (verbose(mergeContext)) {
      message(L"add merge=" + segString(mergeContext, merge->segments),
              mergeContext);
    }
    spec->add(merge);
    start = end;
  }

  return spec;
}

int64_t TieredMergePolicy::floorSize(int64_t bytes)
{
  return max(floorSegmentBytes, bytes);
}

wstring TieredMergePolicy::toString()
{
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(L"[" + getClass().getSimpleName() + L": ");
  sb->append(L"maxMergeAtOnce=")->append(maxMergeAtOnce)->append(L", ");
  sb->append(L"maxMergeAtOnceExplicit=")
      ->append(maxMergeAtOnceExplicit)
      ->append(L", ");
  sb->append(L"maxMergedSegmentMB=")
      ->append(maxMergedSegmentBytes / 1024 / 1024.0)
      ->append(L", ");
  sb->append(L"floorSegmentMB=")
      ->append(floorSegmentBytes / 1024 / 1024.0)
      ->append(L", ");
  sb->append(L"forceMergeDeletesPctAllowed=")
      ->append(forceMergeDeletesPctAllowed)
      ->append(L", ");
  sb->append(L"segmentsPerTier=")->append(segsPerTier)->append(L", ");
  sb->append(L"maxCFSSegmentSizeMB=")
      ->append(getMaxCFSSegmentSizeMB())
      ->append(L", ");
  sb->append(L"noCFSRatio=")->append(noCFSRatio);
  return sb->toString();
}
} // namespace org::apache::lucene::index