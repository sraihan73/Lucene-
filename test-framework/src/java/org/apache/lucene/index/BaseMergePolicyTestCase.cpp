using namespace std;

#include "BaseMergePolicyTestCase.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NullInfoStream = org::apache::lucene::util::NullInfoStream;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void BaseMergePolicyTestCase::testForceMergeNotNeeded() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    shared_ptr<AtomicBoolean> *const mayMerge =
        make_shared<AtomicBoolean>(true);
    shared_ptr<MergeScheduler> *const mergeScheduler =
        make_shared<SerialMergeSchedulerAnonymousInnerClass>(shared_from_this(),
                                                             mayMerge);

    shared_ptr<MergePolicy> mp = mergePolicy();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assumeFalse(L"this test cannot tolerate random forceMerges",
                mp->toString()->contains(L"MockRandomMergePolicy"));
    mp->setNoCFSRatio(random()->nextBoolean() ? 0 : 1);

    shared_ptr<IndexWriterConfig> iwc =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    iwc->setMergeScheduler(mergeScheduler);
    iwc->setMergePolicy(mp);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
    constexpr int numSegments = TestUtil::nextInt(random(), 2, 20);
    for (int i = 0; i < numSegments; ++i) {
      constexpr int numDocs = TestUtil::nextInt(random(), 1, 5);
      for (int j = 0; j < numDocs; ++j) {
        writer->addDocument(make_shared<Document>());
      }
      writer->getReader()->close();
    }
    for (int i = 5; i >= 0; --i) {
      constexpr int segmentCount = writer->getSegmentCount();
      constexpr int maxNumSegments =
          i == 0 ? 1 : TestUtil::nextInt(random(), 1, 10);
      mayMerge->set(segmentCount > maxNumSegments);
      if (VERBOSE) {
        wcout << L"TEST: now forceMerge(maxNumSegments=" << maxNumSegments
              << L") vs segmentCount=" << segmentCount << endl;
      }
      writer->forceMerge(maxNumSegments);
    }
    delete writer;
  }
}

BaseMergePolicyTestCase::SerialMergeSchedulerAnonymousInnerClass::
    SerialMergeSchedulerAnonymousInnerClass(
        shared_ptr<BaseMergePolicyTestCase> outerInstance,
        shared_ptr<AtomicBoolean> mayMerge)
{
  this->outerInstance = outerInstance;
  this->mayMerge = mayMerge;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void BaseMergePolicyTestCase::SerialMergeSchedulerAnonymousInnerClass::merge(
    shared_ptr<IndexWriter> writer, MergeTrigger trigger,
    bool newMergesFound) 
{
  if (mayMerge->get() == false) {
    shared_ptr<MergePolicy::OneMerge> merge = writer->getNextMerge();
    if (merge != nullptr) {
      wcout << L"TEST: we should not need any merging, yet merge policy "
               L"returned merge "
            << merge << endl;
      throw make_shared<AssertionError>();
    }
  }

  outerInstance->super->merge(writer, trigger, newMergesFound);
}

void BaseMergePolicyTestCase::testFindForcedDeletesMerges() 
{
  shared_ptr<MergePolicy> mp = mergePolicy();
  if (std::dynamic_pointer_cast<FilterMergePolicy>(mp) != nullptr) {
    assumeFalse(L"test doesn't work with MockRandomMP",
                std::dynamic_pointer_cast<MockRandomMergePolicy>(
                    (std::static_pointer_cast<FilterMergePolicy>(mp))->in_) !=
                    nullptr);
  }
  shared_ptr<SegmentInfos> infos =
      make_shared<SegmentInfos>(Version::LATEST->major);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory directory
  // = newDirectory())
  {
    org::apache::lucene::store::Directory directory = newDirectory();
    shared_ptr<MergePolicy::MergeContext> context =
        make_shared<MockMergeContext>([&](any s) { 0; });
    int numSegs = random()->nextInt(10);
    for (int i = 0; i < numSegs; i++) {
      shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
          directory, Version::LATEST, Version::LATEST,
          TestUtil::randomSimpleString(random()),
          random()->nextInt(numeric_limits<int>::max()),
          random()->nextBoolean(), nullptr, Collections::emptyMap(),
          TestUtil::randomSimpleString(random(), StringHelper::ID_LENGTH,
                                       StringHelper::ID_LENGTH)
              .getBytes(StandardCharsets::US_ASCII),
          Collections::emptyMap(), nullptr);
      info->setFiles(Collections::emptyList());
      infos->push_back(make_shared<SegmentCommitInfo>(
          info, random()->nextInt(1), 0, -1, -1, -1));
    }
    shared_ptr<MergePolicy::MergeSpecification> forcedDeletesMerges =
        mp->findForcedDeletesMerges(infos, context);
    if (forcedDeletesMerges != nullptr) {
      TestUtil::assertEquals(0, forcedDeletesMerges->merges.size());
    }
  }
}

BaseMergePolicyTestCase::MockMergeContext::MockMergeContext(
    function<int(SegmentCommitInfo *)> &numDeletesFunc)
    : numDeletesFunc(numDeletesFunc)
{
}

int BaseMergePolicyTestCase::MockMergeContext::numDeletesToMerge(
    shared_ptr<SegmentCommitInfo> info)
{
  return numDeletesFunc->applyAsInt(info);
}

int BaseMergePolicyTestCase::MockMergeContext::numDeletedDocs(
    shared_ptr<SegmentCommitInfo> info)
{
  return numDeletesToMerge(info);
}

shared_ptr<InfoStream>
BaseMergePolicyTestCase::MockMergeContext::getInfoStream()
{
  return infoStream;
}

shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
BaseMergePolicyTestCase::MockMergeContext::getMergingSegments()
{
  return Collections::emptySet();
}
} // namespace org::apache::lucene::index