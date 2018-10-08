using namespace std;

#include "TestIndexWriterFromReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexWriterFromReader::testRightAfterCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());
  delete w;

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());

  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  r->close();

  assertEquals(1, w2->maxDoc());
  w2->addDocument(make_shared<Document>());
  assertEquals(2, w2->maxDoc());
  delete w2;

  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir);
  assertEquals(2, r2->maxDoc());
  delete r2;
  delete dir;
}

void TestIndexWriterFromReader::testFromNonNRTReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  delete w;

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  assertEquals(1, r->maxDoc());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());

  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  assertEquals(1, r->maxDoc());
  r->close();

  assertEquals(1, w2->maxDoc());
  w2->addDocument(make_shared<Document>());
  assertEquals(2, w2->maxDoc());
  delete w2;

  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir);
  assertEquals(2, r2->maxDoc());
  delete r2;
  delete dir;
}

void TestIndexWriterFromReader::testWithNoFirstCommit() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  w->rollback();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());

  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<IndexWriter>(dir, iwc); });
  assertEquals(
      L"cannot use IndexWriterConfig.setIndexCommit() when index has no commit",
      expected.what());

  r->close();
  delete dir;
}

void TestIndexWriterFromReader::testAfterCommitThenIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(2, r->maxDoc());
  delete w;

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());

  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<IndexWriter>(dir, iwc); });
  assertTrue(expected.what()->contains(
      L"the provided reader is stale: its prior commit file"));

  r->close();
  delete dir;
}

void TestIndexWriterFromReader::testNRTRollback() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());

  // Add another doc
  w->addDocument(make_shared<Document>());
  assertEquals(2, w->maxDoc());
  delete w;

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<IndexWriter>(dir, iwc); });
  assertTrue(expected.what()->contains(
      L"the provided reader is stale: its prior commit file"));

  r->close();
  delete dir;
}

void TestIndexWriterFromReader::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();

  int numOps = atLeast(100);

  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());

  // We must have a starting commit for this test because whenever we rollback
  // with an NRT reader, the commit before that NRT reader must exist
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  int nrtReaderNumDocs = 0;
  int writerNumDocs = 0;

  bool commitAfterNRT = false;

  shared_ptr<Set<int>> liveIDs = unordered_set<int>();
  shared_ptr<Set<int>> nrtLiveIDs = unordered_set<int>();

  for (int op = 0; op < numOps; op++) {
    if (VERBOSE) {
      wcout << L"\nITER op=" << op << L" nrtReaderNumDocs=" << nrtReaderNumDocs
            << L" writerNumDocs=" << writerNumDocs << L" r=" << r
            << L" r.numDocs()=" << r->numDocs() << endl;
    }

    assertEquals(nrtReaderNumDocs, r->numDocs());
    int x = random()->nextInt(5);

    switch (x) {

    case 0: {
      if (VERBOSE) {
        wcout << L"  add doc id=" << op << endl;
      }
      // add doc
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          newStringField(L"id", L"" + to_wstring(op), Field::Store::NO));
      w->addDocument(doc);
      liveIDs->add(op);
      writerNumDocs++;
      break;
    }
    case 1:
      if (VERBOSE) {
        wcout << L"  delete doc" << endl;
      }
      // delete docs
      if (liveIDs->size() > 0) {
        int id = random()->nextInt(op);
        if (VERBOSE) {
          wcout << L"    id=" << id << endl;
        }
        w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(id))});
        if (liveIDs->remove(id)) {
          if (VERBOSE) {
            wcout << L"    really deleted" << endl;
          }
          writerNumDocs--;
        }
      } else {
        if (VERBOSE) {
          wcout << L"    nothing to delete yet" << endl;
        }
      }
      break;

    case 2: {
      // reopen NRT reader
      if (VERBOSE) {
        wcout << L"  reopen NRT reader" << endl;
      }
      shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
      if (r2 != nullptr) {
        r->close();
        r = r2;
        if (VERBOSE) {
          wcout << L"    got new reader oldNumDocs=" << nrtReaderNumDocs
                << L" newNumDocs=" << writerNumDocs << endl;
        }
        nrtReaderNumDocs = writerNumDocs;
        nrtLiveIDs = unordered_set<>(liveIDs);
      } else {
        if (VERBOSE) {
          wcout << L"    reader is unchanged" << endl;
        }
        assertEquals(nrtReaderNumDocs, r->numDocs());
      }
      commitAfterNRT = false;
      break;
    }
    case 3:
      if (commitAfterNRT == false) {
        // rollback writer to last nrt reader
        if (random()->nextBoolean()) {
          if (VERBOSE) {
            wcout << L"  close writer and open new writer from non-NRT reader "
                     L"numDocs="
                  << w->numDocs() << endl;
          }
          delete w;
          r->close();
          r = DirectoryReader::open(dir);
          assertEquals(writerNumDocs, r->numDocs());
          nrtReaderNumDocs = writerNumDocs;
          nrtLiveIDs = unordered_set<>(liveIDs);
        } else {
          if (VERBOSE) {
            wcout << L"  rollback writer and open new writer from NRT reader "
                     L"numDocs="
                  << w->numDocs() << endl;
          }
          w->rollback();
        }
        shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
        iwc->setIndexCommit(r->getIndexCommit());
        w = make_shared<IndexWriter>(dir, iwc);
        writerNumDocs = nrtReaderNumDocs;
        liveIDs = unordered_set<>(nrtLiveIDs);
        r->close();
        r = DirectoryReader::open(w);
      }
      break;

    case 4:
      if (VERBOSE) {
        wcout << L"    commit" << endl;
      }
      w->commit();
      commitAfterNRT = true;
      break;
    }
  }

  IOUtils::close({w, r, dir});
}

void TestIndexWriterFromReader::testConsistentFieldNumbers() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  // Empty first commit:
  w->commit();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"f0", L"foo", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"f1", L"foo", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNotNull(r2);
  r->close();
  assertEquals(2, r2->maxDoc());
  w->rollback();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r2->getIndexCommit());

  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  r2->close();

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"f1", L"foo", Field::Store::NO));
  doc->push_back(newStringField(L"f0", L"foo", Field::Store::NO));
  w2->addDocument(doc);
  delete w2;
  delete dir;
}

void TestIndexWriterFromReader::testInvalidOpenMode() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());
  delete w;

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  iwc->setIndexCommit(r->getIndexCommit());
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { make_shared<IndexWriter>(dir, iwc); });
  assertEquals(
      L"cannot use IndexWriterConfig.setIndexCommit() with OpenMode.CREATE",
      expected.what());

  IOUtils::close({r, dir});
}

void TestIndexWriterFromReader::testOnClosedReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());
  shared_ptr<IndexCommit> commit = r->getIndexCommit();
  r->close();
  delete w;

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(commit);
  expectThrows(AlreadyClosedException::typeid,
               [&]() { make_shared<IndexWriter>(dir, iwc); });

  IOUtils::close({r, dir});
}

void TestIndexWriterFromReader::testStaleNRTReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(1, r->maxDoc());
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNotNull(r2);
  r2->close();
  w->rollback();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());
  w = make_shared<IndexWriter>(dir, iwc);
  assertEquals(1, w->numDocs());

  r->close();
  shared_ptr<DirectoryReader> r3 = DirectoryReader::open(w);
  assertEquals(1, r3->numDocs());

  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> r4 = DirectoryReader::openIfChanged(r3);
  r3->close();
  assertEquals(2, r4->numDocs());
  r4->close();
  delete w;

  IOUtils::close({r, dir});
}

void TestIndexWriterFromReader::testAfterRollback() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  w->commit();
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(2, r->maxDoc());
  w->rollback();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());
  w = make_shared<IndexWriter>(dir, iwc);
  assertEquals(2, w->numDocs());

  r->close();
  delete w;

  shared_ptr<DirectoryReader> r2 = DirectoryReader::open(dir);
  assertEquals(2, r2->numDocs());
  IOUtils::close({r2, dir});
}

void TestIndexWriterFromReader::testAfterCommitThenIndexKeepCommits() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();

  // Keep all commits:
  iwc->setIndexDeletionPolicy(
      make_shared<IndexDeletionPolicyAnonymousInnerClass>(shared_from_this()));

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  w->addDocument(make_shared<Document>());
  w->commit();
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  assertEquals(2, r->maxDoc());
  w->addDocument(make_shared<Document>());

  shared_ptr<DirectoryReader> r2 = DirectoryReader::open(w);
  assertEquals(3, r2->maxDoc());
  IOUtils::close({r2, w});

  // r is not stale because, even though we've committed the original writer
  // since it was open, we are keeping all commit points:
  iwc = newIndexWriterConfig();
  iwc->setIndexCommit(r->getIndexCommit());
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir, iwc);
  assertEquals(2, w2->maxDoc());
  IOUtils::close({r, w2, dir});
}

TestIndexWriterFromReader::IndexDeletionPolicyAnonymousInnerClass::
    IndexDeletionPolicyAnonymousInnerClass(
        shared_ptr<TestIndexWriterFromReader> outerInstance)
{
  this->outerInstance = outerInstance;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits)
void TestIndexWriterFromReader::IndexDeletionPolicyAnonymousInnerClass::onInit(
    deque<T1> commits)
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits)
void TestIndexWriterFromReader::IndexDeletionPolicyAnonymousInnerClass::
    onCommit(deque<T1> commits)
{
}
} // namespace org::apache::lucene::index