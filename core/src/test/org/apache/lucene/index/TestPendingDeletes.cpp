using namespace std;

#include "TestPendingDeletes.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using IOContext = org::apache::lucene::store::IOContext;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

shared_ptr<PendingDeletes>
TestPendingDeletes::newPendingDeletes(shared_ptr<SegmentCommitInfo> commitInfo)
{
  return make_shared<PendingDeletes>(commitInfo);
}

void TestPendingDeletes::testDeleteDoc() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<SegmentInfo> si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"test", 10, false,
      Codec::getDefault(), Collections::emptyMap(), StringHelper::randomId(),
      unordered_map<>(), nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(si, 0, 0, -1, -1, -1);
  shared_ptr<PendingDeletes> deletes = newPendingDeletes(commitInfo);
  assertNull(deletes->getLiveDocs());
  int docToDelete = TestUtil::nextInt(random(), 0, 7);
  assertTrue(deletes->delete (docToDelete));
  assertNotNull(deletes->getLiveDocs());
  TestUtil::assertEquals(1, deletes->numPendingDeletes());

  shared_ptr<Bits> liveDocs = deletes->getLiveDocs();
  assertFalse(liveDocs->get(docToDelete));
  assertFalse(deletes->delete (docToDelete)); // delete again

  assertTrue(liveDocs->get(8));
  assertTrue(deletes->delete (8));
  assertTrue(liveDocs->get(8)); // we have a snapshot
  TestUtil::assertEquals(2, deletes->numPendingDeletes());

  assertTrue(liveDocs->get(9));
  assertTrue(deletes->delete (9));
  assertTrue(liveDocs->get(9));

  // now make sure new live docs see the deletions
  liveDocs = deletes->getLiveDocs();
  assertFalse(liveDocs->get(9));
  assertFalse(liveDocs->get(8));
  assertFalse(liveDocs->get(docToDelete));
  TestUtil::assertEquals(3, deletes->numPendingDeletes());
  delete dir;
}

void TestPendingDeletes::testWriteLiveDocs() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<SegmentInfo> si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"test", 6, false,
      Codec::getDefault(), Collections::emptyMap(), StringHelper::randomId(),
      unordered_map<>(), nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(si, 0, 0, -1, -1, -1);
  shared_ptr<PendingDeletes> deletes = newPendingDeletes(commitInfo);
  assertFalse(deletes->writeLiveDocs(dir));
  TestUtil::assertEquals(0, dir->listAll().size());
  bool secondDocDeletes = random()->nextBoolean();
  deletes->delete (5);
  if (secondDocDeletes) {
    deletes->getLiveDocs();
    deletes->delete (2);
  }
  TestUtil::assertEquals(-1, commitInfo->getDelGen());
  TestUtil::assertEquals(0, commitInfo->getDelCount());

  TestUtil::assertEquals(secondDocDeletes ? 2 : 1,
                         deletes->numPendingDeletes());
  assertTrue(deletes->writeLiveDocs(dir));
  TestUtil::assertEquals(1, dir->listAll().size());
  shared_ptr<Bits> liveDocs =
      Codec::getDefault()->liveDocsFormat()->readLiveDocs(dir, commitInfo,
                                                          IOContext::DEFAULT);
  assertFalse(liveDocs->get(5));
  if (secondDocDeletes) {
    assertFalse(liveDocs->get(2));
  } else {
    assertTrue(liveDocs->get(2));
  }
  assertTrue(liveDocs->get(0));
  assertTrue(liveDocs->get(1));
  assertTrue(liveDocs->get(3));
  assertTrue(liveDocs->get(4));

  TestUtil::assertEquals(0, deletes->numPendingDeletes());
  TestUtil::assertEquals(secondDocDeletes ? 2 : 1, commitInfo->getDelCount());
  TestUtil::assertEquals(1, commitInfo->getDelGen());

  deletes->delete (0);
  assertTrue(deletes->writeLiveDocs(dir));
  TestUtil::assertEquals(2, dir->listAll().size());
  liveDocs = Codec::getDefault()->liveDocsFormat()->readLiveDocs(
      dir, commitInfo, IOContext::DEFAULT);
  assertFalse(liveDocs->get(5));
  if (secondDocDeletes) {
    assertFalse(liveDocs->get(2));
  } else {
    assertTrue(liveDocs->get(2));
  }
  assertFalse(liveDocs->get(0));
  assertTrue(liveDocs->get(1));
  assertTrue(liveDocs->get(3));
  assertTrue(liveDocs->get(4));

  TestUtil::assertEquals(0, deletes->numPendingDeletes());
  TestUtil::assertEquals(secondDocDeletes ? 3 : 2, commitInfo->getDelCount());
  TestUtil::assertEquals(2, commitInfo->getDelGen());
  delete dir;
}

void TestPendingDeletes::testIsFullyDeleted() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<SegmentInfo> si = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"test", 3, false,
      Codec::getDefault(), Collections::emptyMap(), StringHelper::randomId(),
      unordered_map<>(), nullptr);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(si, 0, 0, -1, -1, -1);
  shared_ptr<FieldInfos> fieldInfos =
      make_shared<FieldInfos>(std::deque<std::shared_ptr<FieldInfo>>(0));
  si->getCodec()->fieldInfosFormat()->write(dir, si, L"", fieldInfos,
                                            IOContext::DEFAULT);
  shared_ptr<PendingDeletes> deletes = newPendingDeletes(commitInfo);
  for (int i = 0; i < 3; i++) {
    assertTrue(deletes->delete (i));
    if (random()->nextBoolean()) {
      assertTrue(deletes->writeLiveDocs(dir));
    }
    TestUtil::assertEquals(i == 2, deletes->isFullyDeleted([&]() { nullptr; }));
  }
}
} // namespace org::apache::lucene::index