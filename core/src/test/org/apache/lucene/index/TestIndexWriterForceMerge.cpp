using namespace std;

#include "TestIndexWriterForceMerge.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterForceMerge::testPartialMerge() 
{

  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<Document> *const doc = make_shared<Document>();
  doc->push_back(newStringField(L"content", L"aaa", Field::Store::NO));
  constexpr int incrMin = TEST_NIGHTLY ? 15 : 40;
  for (int numDocs = 10; numDocs < 500;
       numDocs += TestUtil::nextInt(random(), incrMin, 5 * incrMin)) {
    shared_ptr<LogDocMergePolicy> ldmp = make_shared<LogDocMergePolicy>();
    ldmp->setMinMergeDocs(1);
    ldmp->setMergeFactor(5);
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::CREATE)
                 ->setMaxBufferedDocs(2)
                 ->setMergePolicy(ldmp));
    for (int j = 0; j < numDocs; j++) {
      writer->addDocument(doc);
    }
    delete writer;

    shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
    constexpr int segCount = sis->size();

    ldmp = make_shared<LogDocMergePolicy>();
    ldmp->setMergeFactor(5);
    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMergePolicy(ldmp));
    writer->forceMerge(3);
    delete writer;

    sis = SegmentInfos::readLatestCommit(dir);
    constexpr int optSegCount = sis->size();

    if (segCount < 3) {
      TestUtil::assertEquals(segCount, optSegCount);
    } else {
      TestUtil::assertEquals(3, optSegCount);
    }
  }
  delete dir;
}

void TestIndexWriterForceMerge::testMaxNumSegments2() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<Document> *const doc = make_shared<Document>();
  doc->push_back(newStringField(L"content", L"aaa", Field::Store::NO));

  shared_ptr<LogDocMergePolicy> ldmp = make_shared<LogDocMergePolicy>();
  ldmp->setMinMergeDocs(1);
  ldmp->setMergeFactor(4);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergePolicy(ldmp)
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));

  for (int iter = 0; iter < 10; iter++) {
    for (int i = 0; i < 19; i++) {
      writer->addDocument(doc);
    }

    writer->commit();
    writer->waitForMerges();
    writer->commit();

    shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);

    constexpr int segCount = sis->size();
    writer->forceMerge(7);
    writer->commit();
    writer->waitForMerges();

    sis = SegmentInfos::readLatestCommit(dir);
    constexpr int optSegCount = sis->size();

    if (segCount < 7) {
      TestUtil::assertEquals(segCount, optSegCount);
    } else {
      assertEquals(L"seg: " + to_wstring(segCount), 7, optSegCount);
    }
  }
  delete writer;
  delete dir;
}

void TestIndexWriterForceMerge::testForceMergeTempSpaceUsage() throw(
    IOException)
{

  shared_ptr<MockDirectoryWrapper> *const dir = newMockDirectory();
  // don't use MockAnalyzer, variable length payloads can cause merge to make
  // things bigger, since things are optimized for fixed length case. this is a
  // problem for MemoryPF's encoding. (it might have other problems too)
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10)->setMergePolicy(
          newLogMergePolicy()));

  if (VERBOSE) {
    wcout << L"TEST: config1=" << writer->getConfig() << endl;
  }

  for (int j = 0; j < 500; j++) {
    TestIndexWriter::addDocWithIndex(writer, j);
  }
  // force one extra segment w/ different doc store so
  // we see the doc stores get merged
  writer->commit();
  TestIndexWriter::addDocWithIndex(writer, 500);
  delete writer;

  int64_t startDiskUsage = 0;
  for (auto f : dir->listAll()) {
    startDiskUsage += dir->fileLength(f);
    if (VERBOSE) {
      wcout << f << L": " << dir->fileLength(f) << endl;
    }
  }
  if (VERBOSE) {
    wcout << L"TEST: start disk usage = " << startDiskUsage << endl;
  }
  wstring startListing = listFiles(dir);

  dir->resetMaxUsedSizeInBytes();
  dir->setTrackDiskUsage(true);

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(newLogMergePolicy()));

  if (VERBOSE) {
    wcout << L"TEST: config2=" << writer->getConfig() << endl;
  }

  writer->forceMerge(1);
  delete writer;

  int64_t finalDiskUsage = 0;
  for (auto f : dir->listAll()) {
    finalDiskUsage += dir->fileLength(f);
    if (VERBOSE) {
      wcout << f << L": " << dir->fileLength(f) << endl;
    }
  }
  if (VERBOSE) {
    wcout << L"TEST: final disk usage = " << finalDiskUsage << endl;
  }

  // The result of the merged index is often smaller, but sometimes it could
  // be bigger (compression slightly changes, Codec changes etc.). Therefore
  // we compare the temp space used to the max of the initial and final index
  // size
  int64_t maxStartFinalDiskUsage = max(startDiskUsage, finalDiskUsage);
  int64_t maxDiskUsage = dir->getMaxUsedSizeInBytes();
  assertTrue(L"forceMerge used too much temporary space: starting usage was " +
                 to_wstring(startDiskUsage) + L" bytes; final usage was " +
                 to_wstring(finalDiskUsage) + L" bytes; max temp usage was " +
                 to_wstring(maxDiskUsage) + L" but should have been at most " +
                 to_wstring(4 * maxStartFinalDiskUsage) +
                 L" (= 4X starting usage), BEFORE=" + startListing + L"AFTER=" +
                 listFiles(dir),
             maxDiskUsage <= 4 * maxStartFinalDiskUsage);
  delete dir;
}

TestIndexWriterForceMerge::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterForceMerge> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterForceMerge::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true));
}

wstring TestIndexWriterForceMerge::listFiles(shared_ptr<Directory> dir) throw(
    IOException)
{
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"\r\n");
  for (auto info : infos) {
    for (auto file : info->files()) {
      sb->append(wstring::format(Locale::ROOT, L"%-20s%d%n", file,
                                 dir->fileLength(file)));
    }
    if (info->info->getUseCompoundFile()) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory
      // cfs = info.info.getCodec().compoundFormat().getCompoundReader(dir,
      // info.info, org.apache.lucene.store.IOContext.DEFAULT))
      {
        org::apache::lucene::store::Directory cfs =
            info->info->getCodec()->compoundFormat()->getCompoundReader(
                dir, info->info,
                org::apache::lucene::store::IOContext::DEFAULT);
        for (auto file : cfs->listAll()) {
          sb->append(wstring::format(Locale::ROOT,
                                     L" |- (inside compound file) %-20s%d%n",
                                     file, cfs->fileLength(file)));
        }
      }
    }
  }
  sb->append(L"\r\n");
  return sb->toString();
}

void TestIndexWriterForceMerge::testBackgroundForceMerge() 
{

  shared_ptr<Directory> dir = newDirectory();
  for (int pass = 0; pass < 2; pass++) {
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::CREATE)
                 ->setMaxBufferedDocs(2)
                 ->setMergePolicy(newLogMergePolicy(51)));
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"field", L"aaa", Field::Store::NO));
    for (int i = 0; i < 100; i++) {
      writer->addDocument(doc);
    }
    writer->forceMerge(1, false);

    if (0 == pass) {
      delete writer;
      shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
      TestUtil::assertEquals(1, reader->leaves()->size());
      reader->close();
    } else {
      // Get another segment to flush so we can verify it is
      // NOT included in the merging
      writer->addDocument(doc);
      writer->addDocument(doc);
      delete writer;

      shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
      assertTrue(reader->leaves()->size() > 1);
      reader->close();

      shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
      TestUtil::assertEquals(2, infos->size());
    }
  }

  delete dir;
}
} // namespace org::apache::lucene::index