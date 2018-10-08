using namespace std;

#include "TestSwappedIndexFiles.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::SuppressFileSystems;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSwappedIndexFiles::test() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<Directory> dir2 = newDirectory();

  // Disable CFS 80% of the time so we can truncate individual files, but the
  // other 20% of the time we test truncation of .cfs/.cfe too:
  bool useCFS = random()->nextInt(5) == 1;

  // Use LineFileDocs so we (hopefully) get most Lucene features
  // tested, e.g. IntPoint was recently added to it:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  shared_ptr<Document> doc = docs->nextDoc();
  int64_t seed = random()->nextLong();

  indexOneDoc(seed, dir1, doc, useCFS);
  indexOneDoc(seed, dir2, doc, useCFS);

  swapFiles(dir1, dir2);
  delete dir1;
  delete dir2;
}

void TestSwappedIndexFiles::indexOneDoc(int64_t seed,
                                        shared_ptr<Directory> dir,
                                        shared_ptr<Document> doc,
                                        bool useCFS) 
{
  shared_ptr<Random> random = make_shared<Random>(seed);
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(random, make_shared<MockAnalyzer>(random));
  conf->setCodec(TestUtil::getDefaultCodec());

  if (useCFS == false) {
    conf->setUseCompoundFile(false);
    conf->getMergePolicy()->setNoCFSRatio(0.0);
  } else {
    conf->setUseCompoundFile(true);
    conf->getMergePolicy()->setNoCFSRatio(1.0);
  }

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random, dir, conf);
  w->addDocument(doc);
  delete w;
}

void TestSwappedIndexFiles::swapFiles(
    shared_ptr<Directory> dir1, shared_ptr<Directory> dir2) 
{
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"TEST: dir1 files: " << Arrays->toString(dir1->listAll()) << endl;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"TEST: dir2 files: " << Arrays->toString(dir2->listAll()) << endl;
  }
  for (auto name : dir1->listAll()) {
    if (name.equals(IndexWriter::WRITE_LOCK_NAME)) {
      continue;
    }
    swapOneFile(dir1, dir2, name);
  }
}

void TestSwappedIndexFiles::swapOneFile(
    shared_ptr<Directory> dir1, shared_ptr<Directory> dir2,
    const wstring &victim) 
{
  if (VERBOSE) {
    wcout << L"TEST: swap file " << victim << endl;
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.BaseDirectoryWrapper
  // dirCopy = newDirectory())
  {
    org::apache::lucene::store::BaseDirectoryWrapper dirCopy = newDirectory();
    dirCopy->setCheckIndexOnClose(false);

    // Copy all files from dir1 to dirCopy, except victim which we copy from
    // dir2:
    for (auto name : dir1->listAll()) {
      if (name.equals(victim) == false) {
        dirCopy->copyFrom(dir1, name, name, IOContext::DEFAULT);
      } else {
        dirCopy->copyFrom(dir2, name, name, IOContext::DEFAULT);
      }
      dirCopy->sync(Collections::singleton(name));
    }

    try {
      // NOTE: we .close so that if the test fails (truncation not detected) we
      // don't also get all these confusing errors about open files:
      DirectoryReader::open(dirCopy)->close();
      fail(L"wrong file " + victim + L" not detected");
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (CorruptIndexException | EOFException |
           IndexFormatTooOldException e) {
      // expected
    }

    // CheckIndex should also fail:
    try {
      TestUtil::checkIndex(dirCopy, true, true, nullptr);
      fail(L"wrong file " + victim + L" not detected");
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (CorruptIndexException | EOFException |
           IndexFormatTooOldException e) {
      // expected
    }
  }
}
} // namespace org::apache::lucene::index