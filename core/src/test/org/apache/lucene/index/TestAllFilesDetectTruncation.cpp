using namespace std;

#include "TestAllFilesDetectTruncation.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::SuppressFileSystems;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestAllFilesDetectTruncation::test() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setCodec(TestUtil::getDefaultCodec());

  // Disable CFS 80% of the time so we can truncate individual files, but the
  // other 20% of the time we test truncation of .cfs/.cfe too:
  if (random()->nextInt(5) != 1) {
    conf->setUseCompoundFile(false);
    conf->getMergePolicy()->setNoCFSRatio(0.0);
  }

  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  // Use LineFileDocs so we (hopefully) get most Lucene features
  // tested, e.g. IntPoint was recently added to it:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  for (int i = 0; i < 100; i++) {
    riw->addDocument(docs->nextDoc());
    if (random()->nextInt(7) == 0) {
      riw->commit();
    }
    if (random()->nextInt(20) == 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      riw->deleteDocuments(make_shared<Term>(L"docid", Integer::toString(i)));
    }
    if (random()->nextInt(15) == 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      riw->updateNumericDocValue(
          make_shared<Term>(L"docid", Integer::toString(i)), L"docid_intDV",
          static_cast<int64_t>(i));
    }
  }
  if (TEST_NIGHTLY == false) {
    riw->forceMerge(1);
  }
  delete riw;
  checkTruncation(dir);
  delete dir;
}

void TestAllFilesDetectTruncation::checkTruncation(
    shared_ptr<Directory> dir) 
{
  for (auto name : dir->listAll()) {
    if (name.equals(IndexWriter::WRITE_LOCK_NAME) == false) {
      truncateOneFile(dir, name);
    }
  }
}

void TestAllFilesDetectTruncation::truncateOneFile(
    shared_ptr<Directory> dir, const wstring &victim) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.BaseDirectoryWrapper
  // dirCopy = newDirectory())
  {
    org::apache::lucene::store::BaseDirectoryWrapper dirCopy = newDirectory();
    dirCopy->setCheckIndexOnClose(false);
    int64_t victimLength = dir->fileLength(victim);
    int lostBytes = TestUtil::nextInt(random(), 1,
                                      static_cast<int>(min(100, victimLength)));
    assert(victimLength > 0);

    if (VERBOSE) {
      wcout << L"TEST: now truncate file " << victim << L" by removing "
            << lostBytes << L" of " << victimLength << L" bytes" << endl;
    }

    for (auto name : dir->listAll()) {
      if (name.equals(victim) == false) {
        dirCopy->copyFrom(dir, name, name, IOContext::DEFAULT);
      } else {
        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try(org.apache.lucene.store.IndexOutput
        // out = dirCopy.createOutput(name,
        // org.apache.lucene.store.IOContext.DEFAULT);
        // org.apache.lucene.store.IndexInput in = dir.openInput(name,
        // org.apache.lucene.store.IOContext.DEFAULT))
        {
          org::apache::lucene::store::IndexOutput out = dirCopy->createOutput(
              name, org::apache::lucene::store::IOContext::DEFAULT);
          org::apache::lucene::store::IndexInput in_ = dir->openInput(
              name, org::apache::lucene::store::IOContext::DEFAULT);
          out->copyBytes(in_, victimLength - lostBytes);
        }
      }
      dirCopy->sync(Collections::singleton(name));
    }

    try {
      // NOTE: we .close so that if the test fails (truncation not detected) we
      // don't also get all these confusing errors about open files:
      DirectoryReader::open(dirCopy)->close();
      fail(L"truncation not detected after removing " + to_wstring(lostBytes) +
           L" bytes out of " + to_wstring(victimLength) + L" for file " +
           victim);
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (CorruptIndexException | EOFException e) {
      // expected
    }

    // CheckIndex should also fail:
    try {
      TestUtil::checkIndex(dirCopy, true, true, nullptr);
      fail(L"truncation not detected after removing " + to_wstring(lostBytes) +
           L" bytes out of " + to_wstring(victimLength) + L" for file " +
           victim);
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (CorruptIndexException | EOFException e) {
      // expected
    }
  }
}
} // namespace org::apache::lucene::index