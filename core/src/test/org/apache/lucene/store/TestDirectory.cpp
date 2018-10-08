using namespace std;

#include "TestDirectory.h"

namespace org::apache::lucene::store
{
using ExtrasFS = org::apache::lucene::mockfile::ExtrasFS;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDirectory::testDirectInstantiation() 
{
  shared_ptr<Path> *const path = createTempDir(L"testDirectInstantiation");

  const std::deque<char> largeBuffer =
                              std::deque<char>(random()->nextInt(256 * 1024)),
                          largeReadBuffer =
                              std::deque<char>(largeBuffer.size());
  for (int i = 0; i < largeBuffer.size(); i++) {
    largeBuffer[i] = static_cast<char>(i); // automatically loops with modulo
  }

  const deque<std::shared_ptr<FSDirectory>> dirs0 =
      deque<std::shared_ptr<FSDirectory>>();
  dirs0.push_back(make_shared<SimpleFSDirectory>(path));
  dirs0.push_back(make_shared<NIOFSDirectory>(path));
  if (hasWorkingMMapOnWindows()) {
    dirs0.push_back(make_shared<MMapDirectory>(path));
  }
  std::deque<std::shared_ptr<FSDirectory>> dirs =
      dirs0.stream().toArray(FSDirectory[] ::new);

  for (int i = 0; i < dirs.size(); i++) {
    shared_ptr<FSDirectory> dir = dirs[i];
    dir->ensureOpen();
    wstring fname = L"foo." + to_wstring(i);
    wstring lockname = L"foo" + to_wstring(i) + L".lck";
    shared_ptr<IndexOutput> out =
        dir->createOutput(fname, newIOContext(random()));
    out->writeByte(static_cast<char>(i));
    out->writeBytes(largeBuffer, largeBuffer.size());
    delete out;

    for (int j = 0; j < dirs.size(); j++) {
      shared_ptr<FSDirectory> d2 = dirs[j];
      d2->ensureOpen();
      assertTrue(slowFileExists(d2, fname));
      assertEquals(1 + largeBuffer.size(), d2->fileLength(fname));

      // don't do read tests if unmapping is not supported!
      if (std::dynamic_pointer_cast<MMapDirectory>(d2) != nullptr &&
          !(std::static_pointer_cast<MMapDirectory>(d2))->getUseUnmap()) {
        continue;
      }

      shared_ptr<IndexInput> input =
          d2->openInput(fname, newIOContext(random()));
      assertEquals(static_cast<char>(i), input->readByte());
      // read array with buffering enabled
      Arrays::fill(largeReadBuffer, static_cast<char>(0));
      input->readBytes(largeReadBuffer, 0, largeReadBuffer.size(), true);
      assertArrayEquals(largeBuffer, largeReadBuffer);
      // read again without using buffer
      input->seek(1LL);
      Arrays::fill(largeReadBuffer, static_cast<char>(0));
      input->readBytes(largeReadBuffer, 0, largeReadBuffer.size(), false);
      assertArrayEquals(largeBuffer, largeReadBuffer);
      delete input;
    }

    // delete with a different dir
    dirs[(i + 1) % dirs.size()]->deleteFile(fname);

    for (int j = 0; j < dirs.size(); j++) {
      shared_ptr<FSDirectory> d2 = dirs[j];
      assertFalse(slowFileExists(d2, fname));
    }

    shared_ptr<Lock> lock = dir->obtainLock(lockname);

    for (auto other : dirs) {
      expectThrows(LockObtainFailedException::typeid,
                   [&]() { other->obtainLock(lockname); });
    }

    delete lock;

    // now lock with different dir
    lock = dirs[(i + 1) % dirs.size()]->obtainLock(lockname);
    delete lock;
  }

  for (int i = 0; i < dirs.size(); i++) {
    shared_ptr<FSDirectory> dir = dirs[i];
    dir->ensureOpen();
    delete dir;
    assertFalse(dir->isOpen);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("resource") public void testCopySubdir()
// throws Throwable
void TestDirectory::testCopySubdir() 
{
  shared_ptr<Path> path = createTempDir(L"testsubdir");
  Files::createDirectory(path->resolve(L"subdir"));
  shared_ptr<FSDirectory> fsDir = make_shared<SimpleFSDirectory>(path);
  shared_ptr<RAMDirectory> ramDir =
      make_shared<RAMDirectory>(fsDir, newIOContext(random()));
  deque<wstring> files = Arrays::asList(ramDir->listAll());
  assertFalse(find(files.begin(), files.end(), L"subdir") != files.end());
}

void TestDirectory::testNotDirectory() 
{
  shared_ptr<Path> path = createTempDir(L"testnotdir");
  shared_ptr<Directory> fsDir = make_shared<SimpleFSDirectory>(path);
  try {
    shared_ptr<IndexOutput> out =
        fsDir->createOutput(L"afile", newIOContext(random()));
    delete out;
    assertTrue(slowFileExists(fsDir, L"afile"));
    expectThrows(IOException::typeid, [&]() {
      make_shared<SimpleFSDirectory>(path->resolve(L"afile"));
    });
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete fsDir;
  }
}

void TestDirectory::testListAll() 
{
  shared_ptr<Path> dir = createTempDir(L"testdir");
  assumeFalse(L"this test does not expect extra files",
              std::dynamic_pointer_cast<ExtrasFS>(
                  dir->getFileSystem().provider()) != nullptr);
  shared_ptr<Path> file1 = Files::createFile(dir->resolve(L"tempfile1"));
  shared_ptr<Path> file2 = Files::createFile(dir->resolve(L"tempfile2"));
  shared_ptr<Set<wstring>> files =
      unordered_set<wstring>(Arrays::asList(FSDirectory::listAll(dir)));

  assertTrue(files->size() == 2);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(files->contains(file1->getFileName()->toString()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(files->contains(file2->getFileName()->toString()));
}
} // namespace org::apache::lucene::store