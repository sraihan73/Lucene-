using namespace std;

#include "TestIndexWriterOutOfFileDescriptors.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterOutOfFileDescriptors::test() 
{
  shared_ptr<MockDirectoryWrapper> dir =
      newMockFSDirectory(createTempDir(L"TestIndexWriterOutOfFileDescriptors"));
  double rate = random()->nextDouble() * 0.01;
  // System.out.println("rate=" + rate);
  dir->setRandomIOExceptionRateOnOpen(rate);
  int iters = atLeast(20);
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  shared_ptr<DirectoryReader> r = nullptr;
  shared_ptr<DirectoryReader> r2 = nullptr;
  bool any = false;
  shared_ptr<MockDirectoryWrapper> dirCopy = nullptr;
  int lastNumDocs = 0;
  for (int iter = 0; iter < iters; iter++) {

    shared_ptr<IndexWriter> w = nullptr;
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    try {
      shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
      analyzer->setMaxTokenLength(
          TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
      shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);

      if (VERBOSE) {
        // Do this ourselves instead of relying on LTC so
        // we see incrementing messageID:
        iwc->setInfoStream(make_shared<PrintStreamInfoStream>(System::out));
      }
      shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
      if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
        (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
            ->setSuppressExceptions();
      }
      w = make_shared<IndexWriter>(dir, iwc);
      if (r != nullptr && random()->nextInt(5) == 3) {
        if (random()->nextBoolean()) {
          if (VERBOSE) {
            wcout << L"TEST: addIndexes LR[]" << endl;
          }
          TestUtil::addIndexesSlowly(w, {r});
        } else {
          if (VERBOSE) {
            wcout << L"TEST: addIndexes Directory[]" << endl;
          }
          w->addIndexes(std::deque<std::shared_ptr<Directory>>{dirCopy});
        }
      } else {
        if (VERBOSE) {
          wcout << L"TEST: addDocument" << endl;
        }
        w->addDocument(docs->nextDoc());
      }
      dir->setRandomIOExceptionRateOnOpen(0.0);
      if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
        (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))->sync();
      }
      // If exc hit CMS then writer will be tragically closed:
      if (w->getTragicException() == nullptr) {
        delete w;
      }
      w.reset();

      // NOTE: This is O(N^2)!  Only enable for temporary debugging:
      // dir.setRandomIOExceptionRateOnOpen(0.0);
      //_TestUtil.checkIndex(dir);
      // dir.setRandomIOExceptionRateOnOpen(rate);

      // Verify numDocs only increases, to catch IndexWriter
      // accidentally deleting the index:
      dir->setRandomIOExceptionRateOnOpen(0.0);
      assertTrue(DirectoryReader::indexExists(dir));
      if (r2 == nullptr) {
        r2 = DirectoryReader::open(dir);
      } else {
        shared_ptr<DirectoryReader> r3 = DirectoryReader::openIfChanged(r2);
        if (r3 != nullptr) {
          r2->close();
          r2 = r3;
        }
      }
      assertTrue(L"before=" + to_wstring(lastNumDocs) + L" after=" +
                     to_wstring(r2->numDocs()),
                 r2->numDocs() >= lastNumDocs);
      lastNumDocs = r2->numDocs();
      // System.out.println("numDocs=" + lastNumDocs);
      dir->setRandomIOExceptionRateOnOpen(rate);

      any = true;
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L": success" << endl;
      }
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (AssertionError | IOException ioe) {
      if (VERBOSE) {
        wcout << L"TEST: iter=" << iter << L": exception" << endl;
        ioe::printStackTrace();
      }
      if (w != nullptr) {
        // NOTE: leave random IO exceptions enabled here,
        // to verify that rollback does not try to write
        // anything:
        w->rollback();
      }
    }

    if (any && r == nullptr && random()->nextBoolean()) {
      // Make a copy of a non-empty index so we can use
      // it to addIndexes later:
      dir->setRandomIOExceptionRateOnOpen(0.0);
      r = DirectoryReader::open(dir);
      dirCopy = newMockFSDirectory(
          createTempDir(L"TestIndexWriterOutOfFileDescriptors.copy"));
      shared_ptr<Set<wstring>> files = unordered_set<wstring>();
      for (auto file : dir->listAll()) {
        if (file.startsWith(IndexFileNames::SEGMENTS) ||
            IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches()) {
          dirCopy->copyFrom(dir, file, file, IOContext::DEFAULT);
          files->add(file);
        }
      }
      dirCopy->sync(files);
      // Have IW kiss the dir so we remove any leftover
      // files ... we can easily have leftover files at
      // the time we take a copy because we are holding
      // open a reader:
      delete (make_shared<IndexWriter>(
          dirCopy, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))));
      dirCopy->setRandomIOExceptionRate(rate);
      dir->setRandomIOExceptionRateOnOpen(rate);
    }
  }

  if (r2 != nullptr) {
    r2->close();
  }
  if (r != nullptr) {
    r->close();
    delete dirCopy;
  }
  delete dir;
}
} // namespace org::apache::lucene::index