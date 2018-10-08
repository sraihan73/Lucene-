using namespace std;

#include "TestIndexCommit.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEqualsHashCode() throws Exception
void TestIndexCommit::testEqualsHashCode() 
{
  // LUCENE-2417: equals and hashCode() impl was inconsistent
  shared_ptr<Directory> *const dir = newDirectory();

  shared_ptr<IndexCommit> ic1 =
      make_shared<IndexCommitAnonymousInnerClass>(shared_from_this(), dir);

  shared_ptr<IndexCommit> ic2 =
      make_shared<IndexCommitAnonymousInnerClass2>(shared_from_this(), dir);

  assertEquals(ic1, ic2);
  assertEquals(L"hash codes are not equals", ic1->hashCode(), ic2->hashCode());
  delete dir;
}

TestIndexCommit::IndexCommitAnonymousInnerClass::IndexCommitAnonymousInnerClass(
    shared_ptr<TestIndexCommit> outerInstance, shared_ptr<Directory> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

wstring TestIndexCommit::IndexCommitAnonymousInnerClass::getSegmentsFileName()
{
  return L"a";
}

shared_ptr<Directory>
TestIndexCommit::IndexCommitAnonymousInnerClass::getDirectory()
{
  return dir;
}

shared_ptr<deque<wstring>>
TestIndexCommit::IndexCommitAnonymousInnerClass::getFileNames()
{
  return nullptr;
}

void TestIndexCommit::IndexCommitAnonymousInnerClass::delete_() {}

int64_t TestIndexCommit::IndexCommitAnonymousInnerClass::getGeneration()
{
  return 0;
}

unordered_map<wstring, wstring>
TestIndexCommit::IndexCommitAnonymousInnerClass::getUserData()
{
  return nullptr;
}

bool TestIndexCommit::IndexCommitAnonymousInnerClass::isDeleted()
{
  return false;
}

int TestIndexCommit::IndexCommitAnonymousInnerClass::getSegmentCount()
{
  return 2;
}

TestIndexCommit::IndexCommitAnonymousInnerClass2::
    IndexCommitAnonymousInnerClass2(shared_ptr<TestIndexCommit> outerInstance,
                                    shared_ptr<Directory> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

wstring TestIndexCommit::IndexCommitAnonymousInnerClass2::getSegmentsFileName()
{
  return L"b";
}

shared_ptr<Directory>
TestIndexCommit::IndexCommitAnonymousInnerClass2::getDirectory()
{
  return dir;
}

shared_ptr<deque<wstring>>
TestIndexCommit::IndexCommitAnonymousInnerClass2::getFileNames()
{
  return nullptr;
}

void TestIndexCommit::IndexCommitAnonymousInnerClass2::delete_() {}

int64_t TestIndexCommit::IndexCommitAnonymousInnerClass2::getGeneration()
{
  return 0;
}

unordered_map<wstring, wstring>
TestIndexCommit::IndexCommitAnonymousInnerClass2::getUserData()
{
  return nullptr;
}

bool TestIndexCommit::IndexCommitAnonymousInnerClass2::isDeleted()
{
  return false;
}

int TestIndexCommit::IndexCommitAnonymousInnerClass2::getSegmentCount()
{
  return 2;
}
} // namespace org::apache::lucene::index