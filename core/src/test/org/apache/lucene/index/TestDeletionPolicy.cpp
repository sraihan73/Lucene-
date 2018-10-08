using namespace std;

#include "TestDeletionPolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private void verifyCommitOrder(java.util.List<?
// extends IndexCommit> commits)
void TestDeletionPolicy::verifyCommitOrder(deque<T1> commits)
{
  if (commits.empty()) {
    return;
  }
  shared_ptr<IndexCommit> *const firstCommit = commits[0];
  int64_t last = SegmentInfos::generationFromSegmentsFileName(
      firstCommit->getSegmentsFileName());
  TestUtil::assertEquals(last, firstCommit->getGeneration());
  for (int i = 1; i < commits.size(); i++) {
    shared_ptr<IndexCommit> *const commit = commits[i];
    int64_t now = SegmentInfos::generationFromSegmentsFileName(
        commit->getSegmentsFileName());
    assertTrue(L"SegmentInfos commits are out-of-order", now > last);
    TestUtil::assertEquals(now, commit->getGeneration());
    last = now;
  }
}

TestDeletionPolicy::KeepAllDeletionPolicy::KeepAllDeletionPolicy(
    shared_ptr<TestDeletionPolicy> outerInstance, shared_ptr<Directory> dir)
    : outerInstance(outerInstance)
{
  this->dir = dir;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepAllDeletionPolicy::onInit(
    deque<T1> commits) 
{
  outerInstance->verifyCommitOrder(commits);
  numOnInit++;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepAllDeletionPolicy::onCommit(
    deque<T1> commits) 
{
  shared_ptr<IndexCommit> lastCommit = commits[commits.size() - 1];
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  assertEquals(L"lastCommit.segmentCount()=" +
                   to_wstring(lastCommit->getSegmentCount()) +
                   L" vs IndexReader.segmentCount=" + r->leaves()->size(),
               r->leaves()->size(), lastCommit->getSegmentCount());
  r->close();
  outerInstance->verifyCommitOrder(commits);
  numOnCommit++;
}

TestDeletionPolicy::KeepNoneOnInitDeletionPolicy::KeepNoneOnInitDeletionPolicy(
    shared_ptr<TestDeletionPolicy> outerInstance)
    : outerInstance(outerInstance)
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepNoneOnInitDeletionPolicy::onInit(
    deque<T1> commits) 
{
  outerInstance->verifyCommitOrder(commits);
  numOnInit++;
  // On init, delete all commit points:
  for (auto commit : commits) {
    commit->delete ();
    assertTrue(commit->isDeleted());
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepNoneOnInitDeletionPolicy::onCommit(
    deque<T1> commits) 
{
  outerInstance->verifyCommitOrder(commits);
  int size = commits.size();
  // Delete all but last one:
  for (int i = 0; i < size - 1; i++) {
    (std::static_pointer_cast<IndexCommit>(commits[i]))->delete ();
  }
  numOnCommit++;
}

TestDeletionPolicy::KeepLastNDeletionPolicy::KeepLastNDeletionPolicy(
    shared_ptr<TestDeletionPolicy> outerInstance, int numToKeep)
    : outerInstance(outerInstance)
{
  this->numToKeep = numToKeep;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepLastNDeletionPolicy::onInit(
    deque<T1> commits) 
{
  if (VERBOSE) {
    wcout << L"TEST: onInit" << endl;
  }
  outerInstance->verifyCommitOrder(commits);
  numOnInit++;
  // do no deletions on init
  doDeletes(commits, false);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::KeepLastNDeletionPolicy::onCommit(
    deque<T1> commits) 
{
  if (VERBOSE) {
    wcout << L"TEST: onCommit" << endl;
  }
  outerInstance->verifyCommitOrder(commits);
  doDeletes(commits, true);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private void doDeletes(java.util.List<? extends
// IndexCommit> commits, bool isCommit)
void TestDeletionPolicy::KeepLastNDeletionPolicy::doDeletes(deque<T1> commits,
                                                            bool isCommit)
{

  // Assert that we really are only called for each new
  // commit:
  if (isCommit) {
    wstring fileName =
        (std::static_pointer_cast<IndexCommit>(commits[commits.size() - 1]))
            ->getSegmentsFileName();
    if (seen->contains(fileName)) {
      throw runtime_error(
          L"onCommit was called twice on the same commit point: " + fileName);
    }
    seen->add(fileName);
    numOnCommit++;
  }
  int size = commits.size();
  for (int i = 0; i < size - numToKeep; i++) {
    (std::static_pointer_cast<IndexCommit>(commits[i]))->delete ();
    numDelete++;
  }
}

int64_t TestDeletionPolicy::getCommitTime(
    shared_ptr<IndexCommit> commit) 
{
  return StringHelper::fromString<int64_t>(
      commit->getUserData()[L"commitTime"]);
}

TestDeletionPolicy::ExpirationTimeDeletionPolicy::ExpirationTimeDeletionPolicy(
    shared_ptr<TestDeletionPolicy> outerInstance, shared_ptr<Directory> dir,
    double seconds)
    : outerInstance(outerInstance)
{
  this->dir = dir;
  this->expirationTimeSeconds = seconds;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::ExpirationTimeDeletionPolicy::onInit(
    deque<T1> commits) 
{
  if (commits.empty()) {
    return;
  }
  outerInstance->verifyCommitOrder(commits);
  onCommit(commits);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestDeletionPolicy::ExpirationTimeDeletionPolicy::onCommit(
    deque<T1> commits) 
{
  outerInstance->verifyCommitOrder(commits);

  shared_ptr<IndexCommit> lastCommit = commits[commits.size() - 1];

  // Any commit older than expireTime should be deleted:
  double expireTime =
      getCommitTime(lastCommit) / 1000.0 - expirationTimeSeconds;

  for (auto commit : commits) {
    double modTime = getCommitTime(commit) / 1000.0;
    if (commit != lastCommit && modTime < expireTime) {
      commit->delete ();
      numDelete += 1;
    }
  }
}

void TestDeletionPolicy::testExpirationTimeDeletionPolicy() throw(
    IOException, InterruptedException)
{

  constexpr double SECONDS = 2.0;

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setIndexDeletionPolicy(make_shared<ExpirationTimeDeletionPolicy>(
              shared_from_this(), dir, SECONDS));
  shared_ptr<MergePolicy> mp = conf->getMergePolicy();
  mp->setNoCFSRatio(1.0);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<ExpirationTimeDeletionPolicy> policy =
      std::static_pointer_cast<ExpirationTimeDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  unordered_map<wstring, wstring> commitData =
      unordered_map<wstring, wstring>();
  commitData.emplace(L"commitTime",
                     wstring::valueOf(System::currentTimeMillis()));
  writer->setLiveCommitData(commitData.entrySet());
  writer->commit();
  delete writer;

  int64_t lastDeleteTime = 0;
  constexpr int targetNumDelete = TestUtil::nextInt(random(), 1, 5);
  while (policy->numDelete < targetNumDelete) {
    // Record last time when writer performed deletes of
    // past commits
    lastDeleteTime = System::currentTimeMillis();
    conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setIndexDeletionPolicy(policy);
    mp = conf->getMergePolicy();
    mp->setNoCFSRatio(1.0);
    writer = make_shared<IndexWriter>(dir, conf);
    policy = std::static_pointer_cast<ExpirationTimeDeletionPolicy>(
        writer->getConfig()->getIndexDeletionPolicy());
    for (int j = 0; j < 17; j++) {
      addDoc(writer);
    }
    commitData = unordered_map<>();
    commitData.emplace(L"commitTime",
                       wstring::valueOf(System::currentTimeMillis()));
    writer->setLiveCommitData(commitData.entrySet());
    writer->commit();
    delete writer;

    delay(static_cast<int>(1000.0 * (SECONDS / 5.0)));
  }

  // Then simplistic check: just verify that the
  // segments_N's that still exist are in fact within SECONDS
  // seconds of the last one's mod time, and, that I can
  // open a reader on each:
  int64_t gen = SegmentInfos::getLastCommitGeneration(dir);

  wstring fileName = IndexFileNames::fileNameFromGeneration(
      IndexFileNames::SEGMENTS, L"", gen);
  bool oneSecondResolution = true;

  while (gen > 0) {
    try {
      shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
      delete reader;
      fileName = IndexFileNames::fileNameFromGeneration(
          IndexFileNames::SEGMENTS, L"", gen);

      // if we are on a filesystem that seems to have only
      // 1 second resolution, allow +1 second in commit
      // age tolerance:
      shared_ptr<SegmentInfos> sis = SegmentInfos::readCommit(dir, fileName);
      TestUtil::assertEquals(Version::LATEST, sis->getCommitLuceneVersion());
      TestUtil::assertEquals(Version::LATEST,
                             sis->getMinSegmentLuceneVersion());
      int64_t modTime = StringHelper::fromString<int64_t>(
          sis->getUserData()[L"commitTime"]);
      oneSecondResolution &= (modTime % 1000) == 0;
      constexpr int64_t leeway = static_cast<int64_t>(
          (SECONDS + (oneSecondResolution ? 1.0 : 0.0)) * 1000);

      assertTrue(L"commit point was older than " + to_wstring(SECONDS) +
                     L" seconds (" + to_wstring(lastDeleteTime - modTime) +
                     L" msec) but did not get deleted ",
                 lastDeleteTime - modTime <= leeway);
    } catch (const IOException &e) {
      // OK
      break;
    }

    dir->deleteFile(IndexFileNames::fileNameFromGeneration(
        IndexFileNames::SEGMENTS, L"", gen));
    gen--;
  }

  delete dir;
}

void TestDeletionPolicy::testKeepAllDeletionPolicy() 
{
  for (int pass = 0; pass < 2; pass++) {

    if (VERBOSE) {
      wcout << L"TEST: cycle pass=" << pass << endl;
    }

    bool useCompoundFile = (pass % 2) != 0;

    shared_ptr<Directory> dir = newDirectory();

    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setIndexDeletionPolicy(
                make_shared<KeepAllDeletionPolicy>(shared_from_this(), dir))
            ->setMaxBufferedDocs(10)
            ->setMergeScheduler(make_shared<SerialMergeScheduler>());
    shared_ptr<MergePolicy> mp = conf->getMergePolicy();
    mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
    shared_ptr<KeepAllDeletionPolicy> policy =
        std::static_pointer_cast<KeepAllDeletionPolicy>(
            writer->getConfig()->getIndexDeletionPolicy());
    for (int i = 0; i < 107; i++) {
      addDoc(writer);
    }
    delete writer;

    constexpr bool needsMerging;
    {
      shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
      needsMerging = r->leaves()->size() != 1;
      r->close();
    }
    if (needsMerging) {
      conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND)
                 ->setIndexDeletionPolicy(policy);
      mp = conf->getMergePolicy();
      mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
      if (VERBOSE) {
        wcout << L"TEST: open writer for forceMerge" << endl;
      }
      writer = make_shared<IndexWriter>(dir, conf);
      policy = std::static_pointer_cast<KeepAllDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
      writer->forceMerge(1);
      delete writer;
    }

    TestUtil::assertEquals(needsMerging ? 2 : 1, policy->numOnInit);

    // If we are not auto committing then there should
    // be exactly 2 commits (one per close above):
    TestUtil::assertEquals(1 + (needsMerging ? 1 : 0), policy->numOnCommit);

    // Test listCommits
    shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
        DirectoryReader::listCommits(dir);
    // 2 from closing writer
    TestUtil::assertEquals(1 + (needsMerging ? 1 : 0), commits->size());

    // Make sure we can open a reader on each commit:
    for (auto commit : commits) {
      shared_ptr<IndexReader> r = DirectoryReader::open(commit);
      delete r;
    }

    // Simplistic check: just verify all segments_N's still
    // exist, and, I can open a reader on each:
    int64_t gen = SegmentInfos::getLastCommitGeneration(dir);
    while (gen > 0) {
      shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
      delete reader;
      dir->deleteFile(IndexFileNames::fileNameFromGeneration(
          IndexFileNames::SEGMENTS, L"", gen));
      gen--;

      if (gen > 0) {
        // Now that we've removed a commit point, which
        // should have orphan'd at least one index file.
        // Open & close a writer and assert that it
        // actually removed something:
        int preCount = dir->listAll().size();
        writer = make_shared<IndexWriter>(
            dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setOpenMode(OpenMode::APPEND)
                     ->setIndexDeletionPolicy(policy));
        delete writer;
        int postCount = dir->listAll().size();
        assertTrue(postCount < preCount);
      }
    }

    delete dir;
  }
}

void TestDeletionPolicy::testOpenPriorSnapshot() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(
                   make_shared<KeepAllDeletionPolicy>(shared_from_this(), dir))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(newLogMergePolicy(10)));
  shared_ptr<KeepAllDeletionPolicy> policy =
      std::static_pointer_cast<KeepAllDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  for (int i = 0; i < 10; i++) {
    addDoc(writer);
    if ((1 + i) % 2 == 0) {
      writer->commit();
    }
  }
  delete writer;

  shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
      DirectoryReader::listCommits(dir);
  TestUtil::assertEquals(5, commits->size());
  shared_ptr<IndexCommit> lastCommit = nullptr;
  for (auto commit : commits) {
    if (lastCommit == nullptr ||
        commit->getGeneration() > lastCommit->getGeneration()) {
      lastCommit = commit;
    }
  }
  assertTrue(lastCommit != nullptr);

  // Now add 1 doc and merge
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(policy));
  addDoc(writer);
  TestUtil::assertEquals(11, writer->numDocs());
  writer->forceMerge(1);
  delete writer;

  TestUtil::assertEquals(6, DirectoryReader::listCommits(dir).size());

  // Now open writer on the commit just before merge:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(policy)
               ->setIndexCommit(lastCommit));
  TestUtil::assertEquals(10, writer->numDocs());

  // Should undo our rollback:
  writer->rollback();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  // Still merged, still 11 docs
  TestUtil::assertEquals(1, r->leaves()->size());
  TestUtil::assertEquals(11, r->numDocs());
  r->close();

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(policy)
               ->setIndexCommit(lastCommit));
  TestUtil::assertEquals(10, writer->numDocs());
  // Commits the rollback:
  delete writer;

  // Now 8 because we made another commit
  TestUtil::assertEquals(7, DirectoryReader::listCommits(dir).size());

  r = DirectoryReader::open(dir);
  // Not fully merged because we rolled it back, and now only
  // 10 docs
  assertTrue(r->leaves()->size() > 1);
  TestUtil::assertEquals(10, r->numDocs());
  r->close();

  // Re-merge
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(policy));
  writer->forceMerge(1);
  delete writer;

  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->leaves()->size());
  TestUtil::assertEquals(10, r->numDocs());
  r->close();

  // Now open writer on the commit just before merging,
  // but this time keeping only the last commit:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexCommit(lastCommit));
  TestUtil::assertEquals(10, writer->numDocs());

  // Reader still sees fully merged index, because writer
  // opened on the prior commit has not yet committed:
  r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->leaves()->size());
  TestUtil::assertEquals(10, r->numDocs());
  r->close();

  delete writer;

  // Now reader sees not-fully-merged index:
  r = DirectoryReader::open(dir);
  assertTrue(r->leaves()->size() > 1);
  TestUtil::assertEquals(10, r->numDocs());
  r->close();

  delete dir;
}

void TestDeletionPolicy::testKeepNoneOnInitDeletionPolicy() 
{
  for (int pass = 0; pass < 2; pass++) {

    bool useCompoundFile = (pass % 2) != 0;

    shared_ptr<Directory> dir = newDirectory();

    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setOpenMode(OpenMode::CREATE)
            ->setIndexDeletionPolicy(
                make_shared<KeepNoneOnInitDeletionPolicy>(shared_from_this()))
            ->setMaxBufferedDocs(10);
    shared_ptr<MergePolicy> mp = conf->getMergePolicy();
    mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
    shared_ptr<KeepNoneOnInitDeletionPolicy> policy =
        std::static_pointer_cast<KeepNoneOnInitDeletionPolicy>(
            writer->getConfig()->getIndexDeletionPolicy());
    for (int i = 0; i < 107; i++) {
      addDoc(writer);
    }
    delete writer;

    conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setIndexDeletionPolicy(policy);
    mp = conf->getMergePolicy();
    mp->setNoCFSRatio(1.0);
    writer = make_shared<IndexWriter>(dir, conf);
    policy = std::static_pointer_cast<KeepNoneOnInitDeletionPolicy>(
        writer->getConfig()->getIndexDeletionPolicy());
    writer->forceMerge(1);
    delete writer;

    TestUtil::assertEquals(2, policy->numOnInit);
    // If we are not auto committing then there should
    // be exactly 2 commits (one per close above):
    TestUtil::assertEquals(2, policy->numOnCommit);

    // Simplistic check: just verify the index is in fact
    // readable:
    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    delete reader;

    delete dir;
  }
}

void TestDeletionPolicy::testKeepLastNDeletionPolicy() 
{
  constexpr int N = 5;

  for (int pass = 0; pass < 2; pass++) {

    bool useCompoundFile = (pass % 2) != 0;

    shared_ptr<Directory> dir = newDirectory();

    shared_ptr<KeepLastNDeletionPolicy> policy =
        make_shared<KeepLastNDeletionPolicy>(shared_from_this(), N);
    for (int j = 0; j < N + 1; j++) {
      shared_ptr<IndexWriterConfig> conf =
          newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
              ->setOpenMode(OpenMode::CREATE)
              ->setIndexDeletionPolicy(policy)
              ->setMaxBufferedDocs(10);
      shared_ptr<MergePolicy> mp = conf->getMergePolicy();
      mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
      policy = std::static_pointer_cast<KeepLastNDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
      for (int i = 0; i < 17; i++) {
        addDoc(writer);
      }
      writer->forceMerge(1);
      delete writer;
    }

    assertTrue(policy->numDelete > 0);
    TestUtil::assertEquals(N + 1, policy->numOnInit);
    TestUtil::assertEquals(N + 1, policy->numOnCommit);

    // Simplistic check: just verify only the past N segments_N's still
    // exist, and, I can open a reader on each:
    int64_t gen = SegmentInfos::getLastCommitGeneration(dir);
    for (int i = 0; i < N + 1; i++) {
      try {
        shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
        delete reader;
        if (i == N) {
          fail(L"should have failed on commits prior to last " + to_wstring(N));
        }
      } catch (const IOException &e) {
        if (i != N) {
          throw e;
        }
      }
      if (i < N) {
        dir->deleteFile(IndexFileNames::fileNameFromGeneration(
            IndexFileNames::SEGMENTS, L"", gen));
      }
      gen--;
    }

    delete dir;
  }
}

void TestDeletionPolicy::testKeepLastNDeletionPolicyWithCreates() throw(
    IOException)
{

  constexpr int N = 10;

  for (int pass = 0; pass < 2; pass++) {

    bool useCompoundFile = (pass % 2) != 0;

    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setOpenMode(OpenMode::CREATE)
            ->setIndexDeletionPolicy(
                make_shared<KeepLastNDeletionPolicy>(shared_from_this(), N))
            ->setMaxBufferedDocs(10);
    shared_ptr<MergePolicy> mp = conf->getMergePolicy();
    mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
    shared_ptr<KeepLastNDeletionPolicy> policy =
        std::static_pointer_cast<KeepLastNDeletionPolicy>(
            writer->getConfig()->getIndexDeletionPolicy());
    delete writer;
    shared_ptr<Term> searchTerm = make_shared<Term>(L"content", L"aaa");
    shared_ptr<Query> query = make_shared<TermQuery>(searchTerm);

    for (int i = 0; i < N + 1; i++) {

      conf = newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND)
                 ->setIndexDeletionPolicy(policy)
                 ->setMaxBufferedDocs(10);
      mp = conf->getMergePolicy();
      mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
      writer = make_shared<IndexWriter>(dir, conf);
      policy = std::static_pointer_cast<KeepLastNDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
      for (int j = 0; j < 17; j++) {
        addDocWithID(writer, i * (N + 1) + j);
      }
      // this is a commit
      delete writer;
      conf =
          (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
              ->setIndexDeletionPolicy(policy)
              ->setMergePolicy(NoMergePolicy::INSTANCE);
      writer = make_shared<IndexWriter>(dir, conf);
      policy = std::static_pointer_cast<KeepLastNDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
      writer->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(i * (N + 1) + 3))});
      // this is a commit
      delete writer;
      shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
      shared_ptr<IndexSearcher> searcher = newSearcher(reader);
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          searcher->search(query, 1000)->scoreDocs;
      TestUtil::assertEquals(16, hits.size());
      delete reader;

      writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setOpenMode(OpenMode::CREATE)
                   ->setIndexDeletionPolicy(policy));
      policy = std::static_pointer_cast<KeepLastNDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
      // This will not commit: there are no changes
      // pending because we opened for "create":
      delete writer;
    }

    TestUtil::assertEquals(3 * (N + 1) + 1, policy->numOnInit);
    TestUtil::assertEquals(3 * (N + 1) + 1, policy->numOnCommit);

    shared_ptr<IndexReader> rwReader = DirectoryReader::open(dir);
    shared_ptr<IndexSearcher> searcher = newSearcher(rwReader);
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        searcher->search(query, 1000)->scoreDocs;
    TestUtil::assertEquals(0, hits.size());

    // Simplistic check: just verify only the past N segments_N's still
    // exist, and, I can open a reader on each:
    int64_t gen = SegmentInfos::getLastCommitGeneration(dir);

    int expectedCount = 0;

    delete rwReader;

    for (int i = 0; i < N + 1; i++) {
      try {
        shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

        // Work backwards in commits on what the expected
        // count should be.
        searcher = newSearcher(reader);
        hits = searcher->search(query, 1000)->scoreDocs;
        TestUtil::assertEquals(expectedCount, hits.size());
        if (expectedCount == 0) {
          expectedCount = 16;
        } else if (expectedCount == 16) {
          expectedCount = 17;
        } else if (expectedCount == 17) {
          expectedCount = 0;
        }
        delete reader;
        if (i == N) {
          fail(L"should have failed on commits before last " + to_wstring(N));
        }
      } catch (const IOException &e) {
        if (i != N) {
          throw e;
        }
      }
      if (i < N) {
        dir->deleteFile(IndexFileNames::fileNameFromGeneration(
            IndexFileNames::SEGMENTS, L"", gen));
      }
      gen--;
    }

    delete dir;
  }
}

void TestDeletionPolicy::addDocWithID(shared_ptr<IndexWriter> writer,
                                      int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  doc->push_back(newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
  writer->addDocument(doc);
}

void TestDeletionPolicy::addDoc(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}
} // namespace org::apache::lucene::index