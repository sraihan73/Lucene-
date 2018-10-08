using namespace std;

#include "TestOneMergeWrappingMergePolicy.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
using org::junit::Test;

TestOneMergeWrappingMergePolicy::PredeterminedMergePolicy::
    PredeterminedMergePolicy(
        shared_ptr<MergePolicy::MergeSpecification> merges,
        shared_ptr<MergePolicy::MergeSpecification> forcedMerges,
        shared_ptr<MergePolicy::MergeSpecification> forcedDeletesMerges)
    : merges(merges), forcedMerges(forcedMerges),
      forcedDeletesMerges(forcedDeletesMerges)
{
}

shared_ptr<MergePolicy::MergeSpecification>
TestOneMergeWrappingMergePolicy::PredeterminedMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return merges;
}

shared_ptr<MergePolicy::MergeSpecification>
TestOneMergeWrappingMergePolicy::PredeterminedMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  return forcedMerges;
}

shared_ptr<MergePolicy::MergeSpecification> TestOneMergeWrappingMergePolicy::
    PredeterminedMergePolicy::findForcedDeletesMerges(
        shared_ptr<SegmentInfos> segmentInfos,
        shared_ptr<MergeContext> mergeContext) 
{
  return forcedDeletesMerges;
}

TestOneMergeWrappingMergePolicy::WrappedOneMerge::WrappedOneMerge(
    shared_ptr<MergePolicy::OneMerge> original)
    : MergePolicy::OneMerge(original->segments), original(original)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSegmentsAreWrapped() throws
// java.io.IOException
void TestOneMergeWrappingMergePolicy::testSegmentsAreWrapped() throw(
    IOException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory dir
  // = newDirectory())
  {
    shared_ptr<org::apache::lucene::store::Directory> *const dir =
        newDirectory();
    // first create random merge specs
    shared_ptr<MergePolicy::MergeSpecification> *const msM =
        createRandomMergeSpecification(dir);
    shared_ptr<MergePolicy::MergeSpecification> *const msF =
        createRandomMergeSpecification(dir);
    shared_ptr<MergePolicy::MergeSpecification> *const msD =
        createRandomMergeSpecification(dir);
    // secondly, pass them to the predetermined merge policy constructor
    shared_ptr<MergePolicy> *const originalMP =
        make_shared<PredeterminedMergePolicy>(msM, msF, msD);
    // thirdly wrap the predetermined merge policy
    shared_ptr<MergePolicy> *const oneMergeWrappingMP =
        make_shared<OneMergeWrappingMergePolicy>(originalMP, [&](any merge) {
          make_shared<WrappedOneMerge>(merge);
        });
    // finally, ask for merges and check what we got
    implTestSegmentsAreWrapped(
        msM, oneMergeWrappingMP->findMerges(nullptr, nullptr, nullptr));
    implTestSegmentsAreWrapped(msF, oneMergeWrappingMP->findForcedMerges(
                                        nullptr, 0, nullptr, nullptr));
    implTestSegmentsAreWrapped(
        msD, oneMergeWrappingMP->findForcedDeletesMerges(nullptr, nullptr));
  }
}

void TestOneMergeWrappingMergePolicy::implTestSegmentsAreWrapped(
    shared_ptr<MergePolicy::MergeSpecification> originalMS,
    shared_ptr<MergePolicy::MergeSpecification> testMS)
{
  // wrapping does not add or remove merge specs
  TestUtil::assertEquals((originalMS == nullptr), (testMS == nullptr));
  if (originalMS == nullptr) {
    return;
  }
  TestUtil::assertEquals(originalMS->merges.size(), testMS->merges.size());
  // wrapping does not re-order merge specs
  for (int ii = 0; ii < originalMS->merges.size(); ++ii) {
    shared_ptr<MergePolicy::OneMerge> *const originalOM =
        originalMS->merges[ii];
    shared_ptr<MergePolicy::OneMerge> *const testOM = testMS->merges[ii];
    // wrapping wraps
    assertTrue(std::dynamic_pointer_cast<WrappedOneMerge>(testOM) != nullptr);
    shared_ptr<WrappedOneMerge> *const wrappedOM =
        std::static_pointer_cast<WrappedOneMerge>(testOM);
    // and what is wrapped is what was originally passed in
    TestUtil::assertEquals(originalOM, wrappedOM->original);
  }
}

shared_ptr<MergePolicy::MergeSpecification>
TestOneMergeWrappingMergePolicy::createRandomMergeSpecification(
    shared_ptr<Directory> dir)
{
  shared_ptr<MergePolicy::MergeSpecification> ms;
  if (0 < random()->nextInt(10)) { // ~ 1 in 10 times return null
    ms = make_shared<MergePolicy::MergeSpecification>();
    // append up to 10 (random non-sensical) one merge objects
    for (int ii = 0; ii < random()->nextInt(10); ++ii) {
      shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
          dir, Version::LATEST, Version::LATEST,
          TestUtil::randomSimpleString(random()), random()->nextInt(),
          random()->nextBoolean(), nullptr, Collections::emptyMap(),
          TestUtil::randomSimpleString(random(), StringHelper::ID_LENGTH,
                                       StringHelper::ID_LENGTH)
              .getBytes(StandardCharsets::US_ASCII),
          Collections::emptyMap(), nullptr);
      const deque<std::shared_ptr<SegmentCommitInfo>> segments =
          deque<std::shared_ptr<SegmentCommitInfo>>();
      segments.push_back(make_shared<SegmentCommitInfo>(si, 0, 0, 0, 0, 0));
      ms->add(make_shared<MergePolicy::OneMerge>(segments));
    }
  }
  return nullptr;
}
} // namespace org::apache::lucene::index