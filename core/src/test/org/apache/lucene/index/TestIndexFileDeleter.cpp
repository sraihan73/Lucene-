using namespace std;

#include "TestIndexFileDeleter.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using SimpleTextCodec =
    org::apache::lucene::codecs::simpletext::SimpleTextCodec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexFileDeleter::testDeleteLeftoverFiles() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<MergePolicy> mergePolicy = newLogMergePolicy(true, 10);

  // This test expects all of its segments to be in CFS
  mergePolicy->setNoCFSRatio(1.0);
  mergePolicy->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(mergePolicy)
               ->setUseCompoundFile(true));

  int i;
  for (i = 0; i < 35; i++) {
    addDoc(writer, i);
  }
  writer->getConfig()->getMergePolicy()->setNoCFSRatio(0.0);
  writer->getConfig()->setUseCompoundFile(false);
  for (; i < 45; i++) {
    addDoc(writer, i);
  }
  delete writer;

  // Delete one doc so we get a .del file:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE)
               ->setUseCompoundFile(true));
  shared_ptr<Term> searchTerm = make_shared<Term>(L"id", L"7");
  writer->deleteDocuments({searchTerm});
  delete writer;

  // read in index to try to not depend on codec-specific filenames so much
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  shared_ptr<SegmentInfo> si0 = sis->info(0)->info;
  shared_ptr<SegmentInfo> si1 = sis->info(1)->info;
  shared_ptr<SegmentInfo> si3 = sis->info(3)->info;

  // Now, artificially create an extra .del file & extra
  // .s0 file:
  std::deque<wstring> files = dir->listAll();

  /*
  for(int j=0;j<files.length;j++) {
    System.out.println(j + ": " + files[j]);
  }
  */

  // TODO: fix this test better
  wstring ext = L".liv";

  // Create a bogus separate del file for a
  // segment that already has a separate del file:
  copyFile(dir, L"_0_1" + ext, L"_0_2" + ext);

  // Create a bogus separate del file for a
  // segment that does not yet have a separate del file:
  copyFile(dir, L"_0_1" + ext, L"_1_1" + ext);

  // Create a bogus separate del file for a
  // non-existent segment:
  copyFile(dir, L"_0_1" + ext, L"_188_1" + ext);

  std::deque<wstring> cfsFiles0 =
      std::dynamic_pointer_cast<SimpleTextCodec>(si0->getCodec()) != nullptr
          ? std::deque<wstring>{L"_0.scf"}
          : std::deque<wstring>{L"_0.cfs", L"_0.cfe"};

  // Create a bogus segment file:
  copyFile(dir, cfsFiles0[0], L"_188.cfs");

  // Create a bogus fnm file when the CFS already exists:
  copyFile(dir, cfsFiles0[0], L"_0.fnm");

  // Create a bogus cfs file shadowing a non-cfs segment:

  // TODO: assert is bogus (relies upon codec-specific filenames)
  assertTrue(slowFileExists(dir, L"_3.fdt") || slowFileExists(dir, L"_3.fld"));

  std::deque<wstring> cfsFiles3 =
      std::dynamic_pointer_cast<SimpleTextCodec>(si3->getCodec()) != nullptr
          ? std::deque<wstring>{L"_3.scf"}
          : std::deque<wstring>{L"_3.cfs", L"_3.cfe"};
  for (auto f : cfsFiles3) {
    assertTrue(!slowFileExists(dir, f));
  }

  std::deque<wstring> cfsFiles1 =
      std::dynamic_pointer_cast<SimpleTextCodec>(si1->getCodec()) != nullptr
          ? std::deque<wstring>{L"_1.scf"}
          : std::deque<wstring>{L"_1.cfs", L"_1.cfe"};
  copyFile(dir, cfsFiles1[0], L"_3.cfs");

  std::deque<wstring> filesPre = dir->listAll();

  // Open & close a writer: it should delete the above files and nothing more:
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND));
  delete writer;

  std::deque<wstring> files2 = dir->listAll();
  delete dir;

  Arrays::sort(files);
  Arrays::sort(files2);

  shared_ptr<Set<wstring>> dif = difFiles(files, files2);

  if (!Arrays::equals(files, files2)) {
    fail(L"IndexFileDeleter failed to delete unreferenced extra files: should "
         L"have deleted " +
         (filesPre.size() - files.size()) + L" files but only deleted " +
         (filesPre.size() - files2.size()) + L"; expected files:\n    " +
         asString(files) + L"\n  actual files:\n    " + asString(files2) +
         L"\ndiff: " + dif);
  }
}

shared_ptr<Set<wstring>>
TestIndexFileDeleter::difFiles(std::deque<wstring> &files1,
                               std::deque<wstring> &files2)
{
  shared_ptr<Set<wstring>> set1 = unordered_set<wstring>();
  shared_ptr<Set<wstring>> set2 = unordered_set<wstring>();
  shared_ptr<Set<wstring>> extra = unordered_set<wstring>();

  for (int x = 0; x < files1.size(); x++) {
    set1->add(files1[x]);
  }
  for (int x = 0; x < files2.size(); x++) {
    set2->add(files2[x]);
  }
  Set<wstring>::const_iterator i1 = set1->begin();
  while (i1 != set1->end()) {
    wstring o = *i1;
    if (!set2->contains(o)) {
      extra->add(o);
    }
    i1++;
  }
  Set<wstring>::const_iterator i2 = set2->begin();
  while (i2 != set2->end()) {
    wstring o = *i2;
    if (!set1->contains(o)) {
      extra->add(o);
    }
    i2++;
  }
  return extra;
}

wstring TestIndexFileDeleter::asString(std::deque<wstring> &l)
{
  wstring s = L"";
  for (int i = 0; i < l.size(); i++) {
    if (i > 0) {
      s += L"\n    ";
    }
    s += l[i];
  }
  return s;
}

void TestIndexFileDeleter::copyFile(shared_ptr<Directory> dir,
                                    const wstring &src,
                                    const wstring &dest) 
{
  shared_ptr<IndexInput> in_ = dir->openInput(src, newIOContext(random()));
  shared_ptr<IndexOutput> out = dir->createOutput(dest, newIOContext(random()));
  std::deque<char> b(1024);
  int64_t remainder = in_->length();
  while (remainder > 0) {
    int len = static_cast<int>(min(b.size(), remainder));
    in_->readBytes(b, 0, len);
    out->writeBytes(b, len);
    remainder -= len;
  }
  delete in_;
  delete out;
}

void TestIndexFileDeleter::addDoc(shared_ptr<IndexWriter> writer,
                                  int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  doc->push_back(
      newStringField(L"id", Integer::toString(id), Field::Store::NO));
  writer->addDocument(doc);
}

void TestIndexFileDeleter::testVirusScannerDoesntCorruptIndex() throw(
    IOException)
{
  shared_ptr<Path> path = createTempDir();
  shared_ptr<Directory> dir = newFSDirectory(addVirusChecker(path));
  TestUtil::disableVirusChecker(dir);

  // add empty commit
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));
  // add a trash unreferenced file
  delete dir->createOutput(L"_0.si", IOContext::DEFAULT);

  // start virus scanner
  TestUtil::enableVirusChecker(dir);

  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  iw->addDocument(make_shared<Document>());
  // stop virus scanner
  TestUtil::disableVirusChecker(dir);
  iw->commit();
  delete iw;
  delete dir;
}

void TestIndexFileDeleter::testNoSegmentsDotGenInflation() 
{
  shared_ptr<Directory> dir = newMockDirectory();

  // empty commit
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(1, sis->getGeneration());

  // no inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(1, sis->getGeneration());

  delete dir;
}

void TestIndexFileDeleter::testSegmentsInflation() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->setCheckIndexOnClose(
      false); // TODO: allow falling back more than one commit

  // empty commit
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(1, sis->getGeneration());

  // add trash commit
  delete dir->createOutput(IndexFileNames::SEGMENTS + L"_2",
                           IOContext::DEFAULT);

  // ensure inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(2, sis->getGeneration());

  // add another trash commit
  delete dir->createOutput(IndexFileNames::SEGMENTS + L"_4",
                           IOContext::DEFAULT);
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(4, sis->getGeneration());

  delete dir;
}

void TestIndexFileDeleter::testSegmentNameInflation() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  // empty commit
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(0, sis->counter);

  // no inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(0, sis->counter);

  // add trash per-segment file
  delete dir->createOutput(IndexFileNames::segmentFileName(L"_0", L"", L"foo"),
                           IOContext::DEFAULT);

  // ensure inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(1, sis->counter);

  // add trash per-segment file
  delete dir->createOutput(IndexFileNames::segmentFileName(L"_3", L"", L"foo"),
                           IOContext::DEFAULT);
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(4, sis->counter);

  // ensure we write _4 segment next
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  iw->addDocument(make_shared<Document>());
  iw->commit();
  delete iw;
  sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(L"_4", sis->info(0)->info->name);
  TestUtil::assertEquals(5, sis->counter);

  delete dir;
}

void TestIndexFileDeleter::testGenerationInflation() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  // initial commit
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  iw->addDocument(make_shared<Document>());
  iw->commit();
  delete iw;

  // no deletes: start at 1
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(1, sis->info(0)->getNextDelGen());

  // no inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(1, sis->info(0)->getNextDelGen());

  // add trash per-segment deletes file
  delete dir->createOutput(
      IndexFileNames::fileNameFromGeneration(L"_0", L"del", 2),
      IOContext::DEFAULT);

  // ensure inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(3, sis->info(0)->getNextDelGen());

  delete dir;
}

void TestIndexFileDeleter::testTrashyFile() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->setCheckIndexOnClose(
      false); // TODO: maybe handle such trash better elsewhere...

  // empty commit
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(1, sis->getGeneration());

  // add trash file
  delete dir->createOutput(IndexFileNames::SEGMENTS + L"_", IOContext::DEFAULT);

  // no inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(1, sis->getGeneration());

  delete dir;
}

void TestIndexFileDeleter::testTrashyGenFile() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  // initial commit
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  iw->addDocument(make_shared<Document>());
  iw->commit();
  delete iw;

  // no deletes: start at 1
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(1, sis->info(0)->getNextDelGen());

  // add trash file
  delete dir->createOutput(L"_1_A", IOContext::DEFAULT);

  // no inflation
  inflateGens(sis, Arrays::asList(dir->listAll()), InfoStream::getDefault());
  TestUtil::assertEquals(1, sis->info(0)->getNextDelGen());

  delete dir;
}

void TestIndexFileDeleter::inflateGens(shared_ptr<SegmentInfos> sis,
                                       shared_ptr<deque<wstring>> files,
                                       shared_ptr<InfoStream> stream)
{
  deque<wstring> filtered = deque<wstring>();
  for (auto file : files) {
    if (IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches() ||
        file.startsWith(IndexFileNames::SEGMENTS) ||
        file.startsWith(IndexFileNames::PENDING_SEGMENTS)) {
      filtered.push_back(file);
    }
  }
  IndexFileDeleter::inflateGens(sis, filtered, stream);
}

void TestIndexFileDeleter::testExcInDecRef() 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  // disable slow things: we don't rely upon sleeps here.
  dir->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  dir->setUseSlowOpenClosers(false);

  shared_ptr<AtomicBoolean> *const doFailExc = make_shared<AtomicBoolean>();

  dir->failOn(make_shared<FailureAnonymousInnerClass>(shared_from_this(), dir,
                                                      doFailExc));

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // iwc.setMergeScheduler(new SerialMergeScheduler());
  shared_ptr<MergeScheduler> ms = iwc->getMergeScheduler();
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    shared_ptr<ConcurrentMergeScheduler> *const suppressFakeFail =
        make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this(), dir);
    shared_ptr<ConcurrentMergeScheduler> *const cms =
        std::static_pointer_cast<ConcurrentMergeScheduler>(ms);
    suppressFakeFail->setMaxMergesAndThreads(cms->getMaxMergeCount(),
                                             cms->getMaxThreadCount());
    iwc->setMergeScheduler(suppressFakeFail);
  }

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  // Since we hit exc during merging, a partial
  // forceMerge can easily return when there are still
  // too many segments in the index:
  w->setDoRandomForceMergeAssert(false);

  doFailExc->set(true);
  int ITERS = atLeast(1000);
  for (int iter = 0; iter < ITERS; iter++) {
    try {
      if (random()->nextInt(10) == 5) {
        w->commit();
      } else if (random()->nextInt(10) == 7) {
        w->getReader()->close();
      } else {
        shared_ptr<Document> doc = make_shared<Document>();
        doc->push_back(newTextField(L"field", L"some text", Field::Store::NO));
        w->addDocument(doc);
      }
    } catch (const runtime_error &t) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (t.what()->contains(L"fake fail") ||
          (t.getCause() != nullptr &&
           t.getCause()->toString()->contains(L"fake fail"))) {
        // ok
      } else {
        throw t;
      }
    }
  }

  doFailExc->set(false);
  delete w;
  delete dir;
}

TestIndexFileDeleter::FailureAnonymousInnerClass::FailureAnonymousInnerClass(
    shared_ptr<TestIndexFileDeleter> outerInstance,
    shared_ptr<MockDirectoryWrapper> dir, shared_ptr<AtomicBoolean> doFailExc)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->doFailExc = doFailExc;
}

void TestIndexFileDeleter::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFailExc->get() && LuceneTestCase::random()->nextInt(4) == 1) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: Exception e = new Exception();
    runtime_error e = runtime_error();
    std::deque<std::shared_ptr<StackTraceElement>> stack = e.getStackTrace();
    for (int i = 0; i < stack.size(); i++) {
      if (stack[i]->getClassName().equals(
              IndexFileDeleter::typeid->getName()) &&
          stack[i]->getMethodName().equals(L"decRef")) {
        throw runtime_error(L"fake fail");
      }
    }
  }
}

TestIndexFileDeleter::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass(
        shared_ptr<TestIndexFileDeleter> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

void TestIndexFileDeleter::ConcurrentMergeSchedulerAnonymousInnerClass::
    handleMergeException(shared_ptr<Directory> dir, runtime_error exc)
{
  // suppress only FakeIOException:
  if (dynamic_cast<runtime_error>(exc) != nullptr &&
      exc.what().equals(L"fake fail")) {
    // ok to ignore
  } else if ((std::dynamic_pointer_cast<AlreadyClosedException>(exc) !=
                  nullptr ||
              std::dynamic_pointer_cast<IllegalStateException>(exc) !=
                  nullptr) &&
             exc.getCause() != nullptr &&
             L"fake fail" == exc.getCause().getMessage()) {
    // also ok to ignore
  } else {
    outerInstance->super->handleMergeException(dir, exc);
  }
}

void TestIndexFileDeleter::testExcInDeleteFile() 
{
  int iters = atLeast(10);
  for (int iter = 0; iter < iters; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

    shared_ptr<AtomicBoolean> *const doFailExc = make_shared<AtomicBoolean>();

    dir->failOn(make_shared<FailureAnonymousInnerClass2>(shared_from_this(),
                                                         dir, doFailExc));

    shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
    iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir, iwc);
    w->addDocument(make_shared<Document>());

    // makes segments_1
    if (VERBOSE) {
      wcout << L"TEST: now commit" << endl;
    }
    w->commit();

    w->addDocument(make_shared<Document>());
    doFailExc->set(true);
    if (VERBOSE) {
      wcout << L"TEST: now close" << endl;
    }
    try {
      delete w;
      if (VERBOSE) {
        wcout << L"TEST: no exception (ok)" << endl;
      }
    } catch (const runtime_error &re) {
      assertTrue(
          std::dynamic_pointer_cast<MockDirectoryWrapper::FakeIOException>(
              re.getCause()) != nullptr);
      // good
      if (VERBOSE) {
        wcout << L"TEST: got expected exception:" << endl;
        re.printStackTrace(System::out);
      }
    } catch (const MockDirectoryWrapper::FakeIOException &fioe) {
      // good
      if (VERBOSE) {
        wcout << L"TEST: got expected exception:" << endl;
        fioe->printStackTrace(System::out);
      }
    }
    doFailExc->set(false);
    assertFalse(w->w->isOpen());

    for (auto name : dir->listAll()) {
      if (name.startsWith(IndexFileNames::SEGMENTS)) {
        if (VERBOSE) {
          wcout << L"TEST: now read " << name << endl;
        }
        SegmentInfos::readCommit(dir, name);
      }
    }
    delete dir;
  }
}

TestIndexFileDeleter::FailureAnonymousInnerClass2::FailureAnonymousInnerClass2(
    shared_ptr<TestIndexFileDeleter> outerInstance,
    shared_ptr<MockDirectoryWrapper> dir, shared_ptr<AtomicBoolean> doFailExc)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->doFailExc = doFailExc;
}

void TestIndexFileDeleter::FailureAnonymousInnerClass2::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFailExc->get() && LuceneTestCase::random()->nextInt(4) == 1) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: Exception e = new Exception();
    runtime_error e = runtime_error();
    std::deque<std::shared_ptr<StackTraceElement>> stack = e.getStackTrace();
    for (int i = 0; i < stack.size(); i++) {
      if (stack[i]->getClassName().equals(
              MockDirectoryWrapper::typeid->getName()) &&
          stack[i]->getMethodName().equals(L"deleteFile")) {
        throw make_shared<MockDirectoryWrapper::FakeIOException>();
      }
    }
  }
}
} // namespace org::apache::lucene::index