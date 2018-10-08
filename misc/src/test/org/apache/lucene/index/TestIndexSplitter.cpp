using namespace std;

#include "TestIndexSplitter.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexSplitter::test() 
{
  shared_ptr<Path> dir = createTempDir(LuceneTestCase::getTestClass().name());
  shared_ptr<Path> destDir =
      createTempDir(LuceneTestCase::getTestClass().name());
  shared_ptr<Directory> fsDir = newFSDirectory(dir);
  // IndexSplitter.split makes its own commit directly with SIPC/SegmentInfos,
  // so the unreferenced files are expected.
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(fsDir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(fsDir))
        ->setAssertNoUnrefencedFilesOnClose(false);
  }

  shared_ptr<MergePolicy> mergePolicy = make_shared<LogByteSizeMergePolicy>();
  mergePolicy->setNoCFSRatio(1.0);
  mergePolicy->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      fsDir,
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setOpenMode(OpenMode::CREATE)
          ->setMergePolicy(mergePolicy));
  for (int x = 0; x < 100; x++) {
    shared_ptr<Document> doc = DocHelper::createDocument(x, L"index", 5);
    iw->addDocument(doc);
  }
  iw->commit();
  for (int x = 100; x < 150; x++) {
    shared_ptr<Document> doc = DocHelper::createDocument(x, L"index2", 5);
    iw->addDocument(doc);
  }
  iw->commit();
  for (int x = 150; x < 200; x++) {
    shared_ptr<Document> doc = DocHelper::createDocument(x, L"index3", 5);
    iw->addDocument(doc);
  }
  iw->commit();
  shared_ptr<DirectoryReader> iwReader = iw->getReader();
  assertEquals(3, iwReader->leaves()->size());
  iwReader->close();
  delete iw;
  // we should have 2 segments now
  shared_ptr<IndexSplitter> is = make_shared<IndexSplitter>(dir);
  wstring splitSegName = is->infos->info(1)->info->name;
  is->split(destDir, std::deque<wstring>{splitSegName});
  shared_ptr<Directory> fsDirDest = newFSDirectory(destDir);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(fsDirDest);
  assertEquals(50, r->maxDoc());
  r->close();
  delete fsDirDest;

  // now test cmdline
  shared_ptr<Path> destDir2 =
      createTempDir(LuceneTestCase::getTestClass().name());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  IndexSplitter::main(std::deque<wstring>{
      dir->toAbsolutePath()->toString(), destDir2->toAbsolutePath()->toString(),
      splitSegName});
  shared_ptr<Directory> fsDirDest2 = newFSDirectory(destDir2);
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(fsDirDest2);
  assertEquals(1, sis->size());
  r = DirectoryReader::open(fsDirDest2);
  assertEquals(50, r->maxDoc());
  r->close();
  delete fsDirDest2;

  // now remove the copied segment from src
  // C++ TODO: There is no native C++ equivalent to 'toString':
  IndexSplitter::main(std::deque<wstring>{dir->toAbsolutePath()->toString(),
                                           L"-d", splitSegName});
  r = DirectoryReader::open(fsDir);
  assertEquals(2, r->leaves()->size());
  r->close();
  delete fsDir;
}
} // namespace org::apache::lucene::index