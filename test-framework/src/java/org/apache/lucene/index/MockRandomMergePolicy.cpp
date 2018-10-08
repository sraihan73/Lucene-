using namespace std;

#include "MockRandomMergePolicy.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

MockRandomMergePolicy::MockRandomMergePolicy(shared_ptr<Random> random)
    : random(make_shared<Random>(random->nextLong()))
{
  // fork a private random, since we are called
  // unpredictably from threads:
}

void MockRandomMergePolicy::setDoNonBulkMerges(bool v) { doNonBulkMerges = v; }

shared_ptr<MergeSpecification>
MockRandomMergePolicy::findMerges(MergeTrigger mergeTrigger,
                                  shared_ptr<SegmentInfos> segmentInfos,
                                  shared_ptr<MergeContext> mergeContext)
{
  shared_ptr<MergeSpecification> mergeSpec = nullptr;
  // System.out.println("MRMP: findMerges sis=" + segmentInfos);

  deque<std::shared_ptr<SegmentCommitInfo>> segments =
      deque<std::shared_ptr<SegmentCommitInfo>>();
  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> *const merging =
      mergeContext->getMergingSegments();

  for (auto sipc : segmentInfos) {
    if (!merging->contains(sipc)) {
      segments.push_back(sipc);
    }
  }

  int numSegments = segments.size();

  if (numSegments > 1 && (numSegments > 30 || random->nextInt(5) == 3)) {

    Collections::shuffle(segments, random);

    // TODO: sometimes make more than 1 merge?
    mergeSpec = make_shared<MergeSpecification>();
    constexpr int segsToMerge = TestUtil::nextInt(random, 1, numSegments);
    if (doNonBulkMerges && random->nextBoolean()) {
      mergeSpec->add(make_shared<MockRandomOneMerge>(
          segments.subList(0, segsToMerge), random->nextLong()));
    } else {
      mergeSpec->add(make_shared<OneMerge>(segments.subList(0, segsToMerge)));
    }
  }

  return mergeSpec;
}

shared_ptr<MergeSpecification> MockRandomMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{

  const deque<std::shared_ptr<SegmentCommitInfo>> eligibleSegments =
      deque<std::shared_ptr<SegmentCommitInfo>>();
  for (auto info : segmentInfos) {
    if (segmentsToMerge.find(info) != segmentsToMerge.end()) {
      eligibleSegments.push_back(info);
    }
  }

  // System.out.println("MRMP: findMerges sis=" + segmentInfos + " eligible=" +
  // eligibleSegments);
  shared_ptr<MergeSpecification> mergeSpec = nullptr;
  if (eligibleSegments.size() > 1 ||
      (eligibleSegments.size() == 1 &&
       isMerged(segmentInfos, eligibleSegments[0], mergeContext) == false)) {
    mergeSpec = make_shared<MergeSpecification>();
    // Already shuffled having come out of a set but
    // shuffle again for good measure:
    Collections::shuffle(eligibleSegments, random);
    int upto = 0;
    while (upto < eligibleSegments.size()) {
      int max = min(10, eligibleSegments.size() - upto);
      int inc = max <= 2 ? max : TestUtil::nextInt(random, 2, max);
      if (doNonBulkMerges && random->nextBoolean()) {
        mergeSpec->add(make_shared<MockRandomOneMerge>(
            eligibleSegments.subList(upto, upto + inc), random->nextLong()));
      } else {
        mergeSpec->add(
            make_shared<OneMerge>(eligibleSegments.subList(upto, upto + inc)));
      }
      upto += inc;
    }
  }

  if (mergeSpec != nullptr) {
    for (auto merge : mergeSpec->merges) {
      for (auto info : merge->segments) {
        assert(segmentsToMerge.find(info) != segmentsToMerge.end());
      }
    }
  }
  return mergeSpec;
}

shared_ptr<MergeSpecification> MockRandomMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return findMerges(nullptr, segmentInfos, mergeContext);
}

bool MockRandomMergePolicy::useCompoundFile(
    shared_ptr<SegmentInfos> infos, shared_ptr<SegmentCommitInfo> mergedInfo,
    shared_ptr<MergeContext> mergeContext) 
{
  // 80% of the time we create CFS:
  return random->nextInt(5) != 1;
}

MockRandomMergePolicy::MockRandomOneMerge::MockRandomOneMerge(
    deque<std::shared_ptr<SegmentCommitInfo>> &segments, int64_t seed)
    : OneMerge(segments), r(make_shared<Random>(seed))
{
}

shared_ptr<CodecReader> MockRandomMergePolicy::MockRandomOneMerge::wrapForMerge(
    shared_ptr<CodecReader> reader) 
{

  // wrap it (e.g. prevent bulk merge etc)
  // TODO: cut this over to FilterCodecReader api, we can explicitly
  // enable/disable bulk merge for portions of the index we want.
  int thingToDo = r->nextInt(7);
  if (thingToDo == 0) {
    // simple no-op FilterReader
    if (LuceneTestCase::VERBOSE) {
      wcout << L"NOTE: MockRandomMergePolicy now swaps in a "
               L"SlowCodecReaderWrapper for merging reader="
            << reader << endl;
    }
    return SlowCodecReaderWrapper::wrap(
        make_shared<FilterLeafReaderAnonymousInnerClass>(
            shared_from_this(), make_shared<MergeReaderWrapper>(reader)));
  } else if (thingToDo == 1) {
    // renumber fields
    // NOTE: currently this only "blocks" bulk merges just by
    // being a FilterReader. But it might find bugs elsewhere,
    // and maybe the situation can be improved in the future.
    if (LuceneTestCase::VERBOSE) {
      wcout << L"NOTE: MockRandomMergePolicy now swaps in a "
               L"MismatchedLeafReader for merging reader="
            << reader << endl;
    }
    return SlowCodecReaderWrapper::wrap(make_shared<MismatchedLeafReader>(
        make_shared<MergeReaderWrapper>(reader), r));
  } else {
    // otherwise, reader is unchanged
    return reader;
  }
}

MockRandomMergePolicy::MockRandomOneMerge::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<MockRandomOneMerge> outerInstance,
        shared_ptr<MergeReaderWrapper> new) new
    : FilterLeafReader(new MergeReaderWrapper)
{
  this->outerInstance = outerInstance;
}

shared_ptr<CacheHelper> MockRandomMergePolicy::MockRandomOneMerge::
    FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return in_::getCoreCacheHelper();
}

shared_ptr<CacheHelper> MockRandomMergePolicy::MockRandomOneMerge::
    FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return in_::getReaderCacheHelper();
}
} // namespace org::apache::lucene::index