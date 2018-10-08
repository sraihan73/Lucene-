using namespace std;

#include "TestTransactionRollback.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestTransactionRollback::FIELD_RECORD_ID = L"record_id";

void TestTransactionRollback::rollBackLast(int id) 
{

  // System.out.println("Attempting to rollback to "+id);
  wstring ids = L"-" + to_wstring(id);
  shared_ptr<IndexCommit> last = nullptr;
  shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
      DirectoryReader::listCommits(dir);
  for (shared_ptr<deque<std::shared_ptr<IndexCommit>>::const_iterator>
           iterator = commits->begin();
       iterator != commits->end(); ++iterator) {
    shared_ptr<IndexCommit> commit = *iterator;
    unordered_map<wstring, wstring> ud = commit->getUserData();
    if (ud.size() > 0) {
      if (StringHelper::endsWith(ud[L"index"], ids)) {
        last = commit;
      }
    }
  }

  if (last == nullptr) {
    throw runtime_error(L"Couldn't find commit point " + to_wstring(id));
  }

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(make_shared<RollbackDeletionPolicy>(id))
               ->setIndexCommit(last));
  unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
  data.emplace(L"index", L"Rolled back to 1-" + to_wstring(id));
  w->setLiveCommitData(data.entrySet());
  delete w;
}

void TestTransactionRollback::testRepeatedRollBacks() 
{

  int expectedLastRecordId = 100;
  while (expectedLastRecordId > 10) {
    expectedLastRecordId -= 10;
    rollBackLast(expectedLastRecordId);

    shared_ptr<BitSet> expecteds = make_shared<BitSet>(100);
    expecteds->set(1, (expectedLastRecordId + 1), true);
    checkExpecteds(expecteds);
  }
}

void TestTransactionRollback::checkExpecteds(
    shared_ptr<BitSet> expecteds) 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);

  // Perhaps not the most efficient approach but meets our
  // needs here.
  shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(r);
  for (int i = 0; i < r->maxDoc(); i++) {
    if (liveDocs == nullptr || liveDocs->get(i)) {
      wstring sval = r->document(i)[FIELD_RECORD_ID];
      if (sval != L"") {
        int val = stoi(sval);
        assertTrue(L"Did not expect document #" + to_wstring(val),
                   expecteds->get(val));
        expecteds->set(val, false);
      }
    }
  }
  delete r;
  assertEquals(L"Should have 0 docs remaining ", 0, expecteds->cardinality());
}

void TestTransactionRollback::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();

  // Build index, of records 1 to 100, committing after each batch of 10
  shared_ptr<IndexDeletionPolicy> sdp = make_shared<KeepAllDeletionPolicy>();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setIndexDeletionPolicy(sdp));

  for (int currentRecordId = 1; currentRecordId <= 100; currentRecordId++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(
        FIELD_RECORD_ID, L"" + to_wstring(currentRecordId), Field::Store::YES));
    w->addDocument(doc);

    if (currentRecordId % 10 == 0) {
      unordered_map<wstring, wstring> data = unordered_map<wstring, wstring>();
      data.emplace(L"index", L"records 1-" + to_wstring(currentRecordId));
      w->setLiveCommitData(data.entrySet());
      w->commit();
    }
  }

  delete w;
}

void TestTransactionRollback::tearDown() 
{
  delete dir;
  LuceneTestCase::tearDown();
}

TestTransactionRollback::RollbackDeletionPolicy::RollbackDeletionPolicy(
    int rollbackPoint)
{
  this->rollbackPoint = rollbackPoint;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::RollbackDeletionPolicy::onCommit(
    deque<T1> commits) 
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::RollbackDeletionPolicy::onInit(
    deque<T1> commits) 
{
  for (auto commit : commits) {
    unordered_map<wstring, wstring> userData = commit->getUserData();
    if (userData.size() > 0) {
      // Label for a commit point is "Records 1-30"
      // This code reads the last id ("30" in this example) and deletes it
      // if it is after the desired rollback point
      wstring x = userData[L"index"];
      wstring lastVal = x.substr((int)x.rfind(L"-") + 1);
      int last = stoi(lastVal);
      if (last > rollbackPoint) {
        /*
        System.out.print("\tRolling back commit point:" +
                         " UserData="+commit.getUserData() +")
        ("+(commits.size()-1)+" commit points left) files="); deque files =
        commit.getFileNames(); for (Iterator iterator2 = files.iterator();
        iterator2.hasNext();) { System.out.print(" "+iterator2.next());
        }
        System.out.println();
        */

        commit->delete ();
      }
    }
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::DeleteLastCommitPolicy::onCommit(
    deque<T1> commits) 
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::DeleteLastCommitPolicy::onInit(
    deque<T1> commits) 
{
  commits[commits.size() - 1]->delete ();
}

void TestTransactionRollback::testRollbackDeletionPolicy() 
{

  for (int i = 0; i < 2; i++) {
    // Unless you specify a prior commit point, rollback
    // should not work:
    delete (make_shared<IndexWriter>(
        dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setIndexDeletionPolicy(make_shared<DeleteLastCommitPolicy>())));
    shared_ptr<IndexReader> r = DirectoryReader::open(dir);
    assertEquals(100, r->numDocs());
    delete r;
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::KeepAllDeletionPolicy::onCommit(
    deque<T1> commits) 
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits) throws java.io.IOException
void TestTransactionRollback::KeepAllDeletionPolicy::onInit(
    deque<T1> commits) 
{
}
} // namespace org::apache::lucene::index