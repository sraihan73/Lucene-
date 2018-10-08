using namespace std;

#include "Test2BPoints.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using LongPoint = org::apache::lucene::document::LongPoint;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;

void Test2BPoints::test1D() 
{
  shared_ptr<Directory> dir = FSDirectory::open(createTempDir(L"2BPoints1D"));

  shared_ptr<IndexWriterConfig> iwc =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setCodec(getCodec())
          ->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
          ->setRAMBufferSizeMB(256.0)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
          ->setMergePolicy(newLogMergePolicy(false, 10))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE);

  (std::static_pointer_cast<ConcurrentMergeScheduler>(iwc->getMergeScheduler()))
      ->setMaxMergesAndThreads(6, 3);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    // 1 petabyte:
    (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
        ->setMaxMergeMB(1024 * 1024 * 1024);
  }

  constexpr int numDocs = (numeric_limits<int>::max() / 26) + 1;
  int counter = 0;
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (int j = 0; j < 26; j++) {
      int64_t x = ((static_cast<int64_t>(random()->nextInt()) << 32)) |
                    static_cast<int64_t>(counter);
      doc->push_back(make_shared<LongPoint>(L"long", x));
      counter++;
    }
    w->addDocument(doc);
    if (VERBOSE && i % 100000 == 0) {
      wcout << i << L" of " << numDocs << L"..." << endl;
    }
  }
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(numDocs, s->count(LongPoint::newRangeQuery(
                                      L"long", numeric_limits<int64_t>::min(),
                                      numeric_limits<int64_t>::max())));
  assertTrue(r->leaves()->get(0).reader().getPointValues(L"long")->size() >
             numeric_limits<int>::max());
  r->close();
  delete w;
  wcout << L"TEST: now CheckIndex" << endl;
  TestUtil::checkIndex(dir);
  delete dir;
}

void Test2BPoints::test2D() 
{
  shared_ptr<Directory> dir = FSDirectory::open(createTempDir(L"2BPoints2D"));

  shared_ptr<IndexWriterConfig> iwc =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setCodec(getCodec())
          ->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
          ->setRAMBufferSizeMB(256.0)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
          ->setMergePolicy(newLogMergePolicy(false, 10))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE);

  (std::static_pointer_cast<ConcurrentMergeScheduler>(iwc->getMergeScheduler()))
      ->setMaxMergesAndThreads(6, 3);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    // 1 petabyte:
    (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
        ->setMaxMergeMB(1024 * 1024 * 1024);
  }

  constexpr int numDocs = (numeric_limits<int>::max() / 26) + 1;
  int counter = 0;
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (int j = 0; j < 26; j++) {
      int64_t x = ((static_cast<int64_t>(random()->nextInt()) << 32)) |
                    static_cast<int64_t>(counter);
      int64_t y = ((static_cast<int64_t>(random()->nextInt()) << 32)) |
                    static_cast<int64_t>(random()->nextInt());
      doc->push_back(make_shared<LongPoint>(L"long", x, y));
      counter++;
    }
    w->addDocument(doc);
    if (VERBOSE && i % 100000 == 0) {
      wcout << i << L" of " << numDocs << L"..." << endl;
    }
  }
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  TestUtil::assertEquals(
      numDocs, s->count(LongPoint::newRangeQuery(
                   L"long",
                   std::deque<int64_t>{numeric_limits<int64_t>::min(),
                                          numeric_limits<int64_t>::min()},
                   std::deque<int64_t>{numeric_limits<int64_t>::max(),
                                          numeric_limits<int64_t>::max()})));
  assertTrue(r->leaves()->get(0).reader().getPointValues(L"long")->size() >
             numeric_limits<int>::max());
  r->close();
  delete w;
  wcout << L"TEST: now CheckIndex" << endl;
  TestUtil::checkIndex(dir);
  delete dir;
}

shared_ptr<Codec> Test2BPoints::getCodec()
{
  return Codec::forName(L"Lucene70");
}
} // namespace org::apache::lucene::index