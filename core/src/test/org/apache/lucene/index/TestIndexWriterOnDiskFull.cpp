using namespace std;

#include "TestIndexWriterOnDiskFull.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterOnDiskFull::testAddDocumentOnDiskFull() 
{

  for (int pass = 0; pass < 2; pass++) {
    if (VERBOSE) {
      wcout << L"TEST: pass=" << pass << endl;
    }
    bool doAbort = pass == 1;
    int64_t diskFree = TestUtil::nextInt(random(), 100, 300);
    bool indexExists = false;
    while (true) {
      if (VERBOSE) {
        wcout << L"TEST: cycle: diskFree=" << diskFree << endl;
      }
      shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
          random(), make_shared<RAMDirectory>());
      dir->setMaxSizeInBytes(diskFree);
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
      shared_ptr<MergeScheduler> ms = writer->getConfig()->getMergeScheduler();
      if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
        // This test intentionally produces exceptions
        // in the threads that CMS launches; we don't
        // want to pollute test output with these.
        (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
            ->setSuppressExceptions();
      }

      bool hitError = false;
      try {
        for (int i = 0; i < 200; i++) {
          addDoc(writer);
        }
        if (VERBOSE) {
          wcout << L"TEST: done adding docs; now commit" << endl;
        }
        writer->commit();
        indexExists = true;
      } catch (const IOException &e) {
        if (VERBOSE) {
          wcout << L"TEST: exception on addDoc" << endl;
          e->printStackTrace(System::out);
        }
        hitError = true;
      }

      if (hitError) {
        if (doAbort) {
          if (VERBOSE) {
            wcout << L"TEST: now rollback" << endl;
          }
          writer->rollback();
        } else {
          try {
            if (VERBOSE) {
              wcout << L"TEST: now close" << endl;
            }
            delete writer;
          } catch (const IOException &e) {
            if (VERBOSE) {
              wcout << L"TEST: exception on close; retry w/ no disk space limit"
                    << endl;
              e->printStackTrace(System::out);
            }
            dir->setMaxSizeInBytes(0);
            try {
              delete writer;
            } catch (const AlreadyClosedException &ace) {
              // OK
            }
          }
        }

        //_TestUtil.syncConcurrentMerges(ms);

        if (indexExists) {
          // Make sure reader can open the index:
          DirectoryReader::open(dir)->close();
        }

        delete dir;
        // Now try again w/ more space:

        diskFree += TEST_NIGHTLY ? TestUtil::nextInt(random(), 400, 600)
                                 : TestUtil::nextInt(random(), 3000, 5000);
      } else {
        //_TestUtil.syncConcurrentMerges(writer);
        dir->setMaxSizeInBytes(0);
        delete writer;
        delete dir;
        break;
      }
    }
  }
}

void TestIndexWriterOnDiskFull::testAddIndexOnDiskFull() 
{
  // MemoryCodec, since it uses FST, is not necessarily
  // "additive", ie if you add up N small FSTs, then merge
  // them, the merged result can easily be larger than the
  // sum because the merged FST may use array encoding for
  // some arcs (which uses more space):

  const wstring idFormat = TestUtil::getPostingsFormat(L"id");
  const wstring contentFormat = TestUtil::getPostingsFormat(L"content");
  assumeFalse(L"This test cannot run with Memory codec",
              idFormat == L"Memory" || contentFormat == L"Memory");

  int START_COUNT = 57;
  int NUM_DIR = TEST_NIGHTLY ? 50 : 5;
  int END_COUNT = START_COUNT + NUM_DIR * (TEST_NIGHTLY ? 25 : 5);

  // Build up a bunch of dirs that have indexes which we
  // will then merge together by calling addIndexes(*):
  std::deque<std::shared_ptr<Directory>> dirs(NUM_DIR);
  int64_t inputDiskUsage = 0;
  for (int i = 0; i < NUM_DIR; i++) {
    dirs[i] = newDirectory();
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dirs[i], newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    for (int j = 0; j < 25; j++) {
      addDocWithIndex(writer, 25 * i + j);
    }
    delete writer;
    std::deque<wstring> files = dirs[i]->listAll();
    for (int j = 0; j < files.size(); j++) {
      inputDiskUsage += dirs[i]->fileLength(files[j]);
    }
  }

  // Now, build a starting index that has START_COUNT docs.  We
  // will then try to addIndexes into a copy of this:
  shared_ptr<MockDirectoryWrapper> startDir = newMockDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      startDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int j = 0; j < START_COUNT; j++) {
    addDocWithIndex(writer, j);
  }
  delete writer;

  // Make sure starting index seems to be working properly:
  shared_ptr<Term> searchTerm = make_shared<Term>(L"content", L"aaa");
  shared_ptr<IndexReader> reader = DirectoryReader::open(startDir);
  assertEquals(L"first docFreq", 57, reader->docFreq(searchTerm));

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(make_shared<TermQuery>(searchTerm), 1000)->scoreDocs;
  assertEquals(L"first number of hits", 57, hits.size());
  delete reader;

  // Iterate with larger and larger amounts of free
  // disk space.  With little free disk space,
  // addIndexes will certainly run out of space &
  // fail.  Verify that when this happens, index is
  // not corrupt and index in fact has added no
  // documents.  Then, we increase disk space by 2000
  // bytes each iteration.  At some point there is
  // enough free disk space and addIndexes should
  // succeed and index should show all documents were
  // added.

  // std::wstring[] files = startDir.listAll();
  int64_t diskUsage = startDir->sizeInBytes();

  int64_t startDiskUsage = 0;
  std::deque<wstring> files = startDir->listAll();
  for (int i = 0; i < files.size(); i++) {
    startDiskUsage += startDir->fileLength(files[i]);
  }

  for (int iter = 0; iter < 3; iter++) {

    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }

    // Start with 100 bytes more than we are currently using:
    int64_t diskFree = diskUsage + TestUtil::nextInt(random(), 50, 200);

    int method = iter;

    bool success = false;
    bool done = false;

    wstring methodName;
    if (0 == method) {
      methodName = L"addIndexes(Directory[]) + forceMerge(1)";
    } else if (1 == method) {
      methodName = L"addIndexes(IndexReader[])";
    } else {
      methodName = L"addIndexes(Directory[])";
    }

    while (!done) {
      if (VERBOSE) {
        wcout << L"TEST: cycle..." << endl;
      }

      // Make a new dir that will enforce disk usage:
      shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
          random(), TestUtil::ramCopyOf(startDir));
      shared_ptr<IndexWriterConfig> iwc =
          newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
              ->setOpenMode(OpenMode::APPEND)
              ->setMergePolicy(newLogMergePolicy(false));
      writer = make_shared<IndexWriter>(dir, iwc);
      runtime_error err = nullptr;

      for (int x = 0; x < 2; x++) {
        shared_ptr<MergeScheduler> ms =
            writer->getConfig()->getMergeScheduler();
        if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) !=
            nullptr) {
          // This test intentionally produces exceptions
          // in the threads that CMS launches; we don't
          // want to pollute test output with these.
          if (0 == x) {
            (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
                ->setSuppressExceptions();
          } else {
            (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
                ->clearSuppressExceptions();
          }
        }

        // Two loops: first time, limit disk space &
        // throw random IOExceptions; second time, no
        // disk space limit:

        double rate = 0.05;
        double diskRatio = (static_cast<double>(diskFree)) / diskUsage;
        int64_t thisDiskFree;

        wstring testName = L"";

        if (0 == x) {
          dir->setRandomIOExceptionRateOnOpen(random()->nextDouble() * 0.01);
          thisDiskFree = diskFree;
          if (diskRatio >= 2.0) {
            rate /= 2;
          }
          if (diskRatio >= 4.0) {
            rate /= 2;
          }
          if (diskRatio >= 6.0) {
            rate = 0.0;
          }
          if (VERBOSE) {
            testName = L"disk full test " + methodName +
                       L" with disk full at " + to_wstring(diskFree) +
                       L" bytes";
          }
        } else {
          dir->setRandomIOExceptionRateOnOpen(0.0);
          thisDiskFree = 0;
          rate = 0.0;
          if (VERBOSE) {
            testName =
                L"disk full test " + methodName + L" with unlimited disk space";
          }
        }

        if (VERBOSE) {
          wcout << L"\ncycle: " << testName << endl;
        }

        dir->setTrackDiskUsage(true);
        dir->setMaxSizeInBytes(thisDiskFree);
        dir->setRandomIOExceptionRate(rate);

        try {

          if (0 == method) {
            if (VERBOSE) {
              wcout << L"TEST: now addIndexes count=" << dirs.size() << endl;
            }
            writer->addIndexes(dirs);
            if (VERBOSE) {
              wcout << L"TEST: now forceMerge" << endl;
            }
            writer->forceMerge(1);
          } else if (1 == method) {
            std::deque<std::shared_ptr<DirectoryReader>> readers(dirs.size());
            for (int i = 0; i < dirs.size(); i++) {
              readers[i] = DirectoryReader::open(dirs[i]);
            }
            try {
              TestUtil::addIndexesSlowly(writer, readers);
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              for (int i = 0; i < dirs.size(); i++) {
                readers[i]->close();
              }
            }
          } else {
            writer->addIndexes(dirs);
          }

          success = true;
          if (VERBOSE) {
            wcout << L"  success!" << endl;
          }

          if (0 == x) {
            done = true;
          }

        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (IllegalStateException | IOException e) {
          success = false;
          err = e;
          if (VERBOSE) {
            wcout << L"  hit Exception: " << e << endl;
            e::printStackTrace(System::out);
          }

          if (1 == x) {
            e::printStackTrace(System::out);
            fail(methodName +
                 L" hit IOException after disk space was freed up");
          }
        }

        if (x == 1) {
          // Make sure all threads from ConcurrentMergeScheduler are done
          TestUtil::syncConcurrentMerges(writer);
        } else {
          dir->setRandomIOExceptionRateOnOpen(0.0);
          writer->rollback();
          writer = make_shared<IndexWriter>(
              dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                       ->setOpenMode(OpenMode::APPEND)
                       ->setMergePolicy(newLogMergePolicy(false)));
        }

        if (VERBOSE) {
          wcout << L"  now test readers" << endl;
        }

        // Finally, verify index is not corrupt, and, if
        // we succeeded, we see all docs added, and if we
        // failed, we see either all docs or no docs added
        // (transactional semantics):
        dir->setRandomIOExceptionRateOnOpen(0.0);
        try {
          reader = DirectoryReader::open(dir);
        } catch (const IOException &e) {
          e->printStackTrace(System::out);
          fail(testName + L": exception when creating IndexReader: " + e);
        }
        int result = reader->docFreq(searchTerm);
        if (success) {
          if (result != START_COUNT) {
            fail(testName +
                 L": method did not throw exception but docFreq('aaa') is " +
                 to_wstring(result) + L" instead of expected " +
                 to_wstring(START_COUNT));
          }
        } else {
          // On hitting exception we still may have added
          // all docs:
          if (result != START_COUNT && result != END_COUNT) {
            err.printStackTrace(System::out);
            fail(testName +
                 L": method did throw exception but docFreq('aaa') is " +
                 to_wstring(result) + L" instead of expected " +
                 to_wstring(START_COUNT) + L" or " + to_wstring(END_COUNT));
          }
        }

        searcher = newSearcher(reader);
        try {
          hits = searcher->search(make_shared<TermQuery>(searchTerm), END_COUNT)
                     ->scoreDocs;
        } catch (const IOException &e) {
          e->printStackTrace(System::out);
          fail(testName + L": exception when searching: " + e);
        }
        int result2 = hits.size();
        if (success) {
          if (result2 != result) {
            fail(testName +
                 L": method did not throw exception but hits.length for search "
                 L"on term 'aaa' is " +
                 to_wstring(result2) + L" instead of expected " +
                 to_wstring(result));
          }
        } else {
          // On hitting exception we still may have added
          // all docs:
          if (result2 != result) {
            err.printStackTrace(System::out);
            fail(testName +
                 L": method did throw exception but hits.length for search on "
                 L"term 'aaa' is " +
                 to_wstring(result2) + L" instead of expected " +
                 to_wstring(result));
          }
        }

        delete reader;
        if (VERBOSE) {
          wcout << L"  count is " << result << endl;
        }

        if (done || result == END_COUNT) {
          break;
        }
      }

      if (VERBOSE) {
        wcout << L"  start disk = " << startDiskUsage << L"; input disk = "
              << inputDiskUsage << L"; max used = "
              << dir->getMaxUsedSizeInBytes() << endl;
      }

      if (done) {
        // Javadocs state that temp free Directory space
        // required is at most 2X total input size of
        // indices so let's make sure:
        assertTrue(
            L"max free Directory space required exceeded 1X the total input "
            L"index sizes during " +
                methodName + L": max temp usage = " +
                to_wstring(dir->getMaxUsedSizeInBytes() - startDiskUsage) +
                L" bytes vs limit=" +
                to_wstring(2 * (startDiskUsage + inputDiskUsage)) +
                L"; starting disk usage = " + to_wstring(startDiskUsage) +
                L" bytes; " + L"input index disk usage = " +
                to_wstring(inputDiskUsage) + L" bytes",
            (dir->getMaxUsedSizeInBytes() - startDiskUsage) <
                2 * (startDiskUsage + inputDiskUsage));
      }

      // Make sure we don't hit disk full during close below:
      dir->setMaxSizeInBytes(0);
      dir->setRandomIOExceptionRate(0.0);
      dir->setRandomIOExceptionRateOnOpen(0.0);

      delete writer;

      delete dir;

      // Try again with more free space:
      diskFree += TEST_NIGHTLY ? TestUtil::nextInt(random(), 4000, 8000)
                               : TestUtil::nextInt(random(), 40000, 80000);
    }
  }

  delete startDir;
  for (auto dir : dirs) {
    dir->close();
  }
}

void TestIndexWriterOnDiskFull::FailTwiceDuringMerge::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (!doFail) {
    return;
  }
  // C++ TODO: This exception's constructor requires an argument:
  // ORIGINAL LINE: StackTraceElement[] trace = new Exception().getStackTrace();
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      (runtime_error())->getStackTrace();
  for (int i = 0; i < trace.size(); i++) {
    if (SegmentMerger::typeid->getName().equals(trace[i]->getClassName()) &&
        L"mergeTerms" == trace[i]->getMethodName() && !didFail1) {
      didFail1 = true;
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"fake disk full during mergeTerms");
    }
    if (LiveDocsFormat::typeid->getName().equals(trace[i]->getClassName()) &&
        L"writeLiveDocs" == trace[i]->getMethodName() && !didFail2) {
      didFail2 = true;
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"fake disk full while writing LiveDocs");
    }
  }
}

void TestIndexWriterOnDiskFull::testCorruptionAfterDiskFullDuringMerge() throw(
    IOException)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  // IndexWriter w = new IndexWriter(dir, newIndexWriterConfig(new
  // MockAnalyzer(random)).setReaderPooling(true));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergeScheduler(make_shared<SerialMergeScheduler>())
          ->setReaderPooling(true)
          ->setMergePolicy(make_shared<FilterMergePolicyAnonymousInnerClass>(
              shared_from_this(), newLogMergePolicy(2))));
  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(newTextField(L"f", L"doctor who", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  w->deleteDocuments({make_shared<Term>(L"f", L"who")});
  w->addDocument(doc);

  // disk fills up!
  shared_ptr<FailTwiceDuringMerge> ftdm = make_shared<FailTwiceDuringMerge>();
  ftdm->setDoFail();
  dir->failOn(ftdm);

  expectThrows(IOException::typeid, [&]() { w->commit(); });
  assertTrue(ftdm->didFail1 || ftdm->didFail2);

  TestUtil::checkIndex(dir);
  ftdm->clearDoFail();
  expectThrows(AlreadyClosedException::typeid, [&]() { w->addDocument(doc); });

  delete dir;
}

TestIndexWriterOnDiskFull::FilterMergePolicyAnonymousInnerClass::
    FilterMergePolicyAnonymousInnerClass(
        shared_ptr<TestIndexWriterOnDiskFull> outerInstance,
        shared_ptr<org::apache::lucene::index::LogMergePolicy>
            newLogMergePolicy)
    : FilterMergePolicy(newLogMergePolicy)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterOnDiskFull::FilterMergePolicyAnonymousInnerClass::
    keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                                readerIOSupplier) 
{
  // we can do this because we add/delete/add (and dont merge to "nothing")
  return true;
}

void TestIndexWriterOnDiskFull::testImmediateDiskFull() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               ->setCommitOnClose(false));
  writer->commit(); // empty commit, to not create confusing situation with
                    // first commit
  dir->setMaxSizeInBytes(max(1, dir->getRecomputedActualSizeInBytes()));
  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  doc->push_back(newField(L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj",
                          customType));
  expectThrows(IOException::typeid, [&]() { writer->addDocument(doc); });
  assertTrue(writer->deleter->isClosed());
  assertTrue(writer->isClosed());

  delete dir;
}

void TestIndexWriterOnDiskFull::addDoc(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"numericdv", 1));
  doc->push_back(make_shared<IntPoint>(L"point", 1));
  doc->push_back(make_shared<IntPoint>(L"point2d", 1, 1));
  writer->addDocument(doc);
}

void TestIndexWriterOnDiskFull::addDocWithIndex(shared_ptr<IndexWriter> writer,
                                                int index) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"content", L"aaa " + to_wstring(index), Field::Store::NO));
  doc->push_back(
      newTextField(L"id", L"" + to_wstring(index), Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"numericdv", 1));
  doc->push_back(make_shared<IntPoint>(L"point", 1));
  doc->push_back(make_shared<IntPoint>(L"point2d", 1, 1));
  writer->addDocument(doc);
}
} // namespace org::apache::lucene::index