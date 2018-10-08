using namespace std;

#include "TestShuffleFS.h"

namespace org::apache::lucene::mockfile
{

shared_ptr<Path> TestShuffleFS::wrap(shared_ptr<Path> path)
{
  return wrap(path, random()->nextLong());
}

shared_ptr<Path> TestShuffleFS::wrap(shared_ptr<Path> path, int64_t seed)
{
  shared_ptr<FileSystem> fs =
      (make_shared<ShuffleFS>(path->getFileSystem(), seed))
          ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestShuffleFS::testShuffleWorks() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  Files::createFile(dir->resolve(L"file1"));
  Files::createFile(dir->resolve(L"file2"));
  Files::createFile(dir->resolve(L"file3"));

  deque<std::shared_ptr<Path>> seen = deque<std::shared_ptr<Path>>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir);
    for (auto path : stream) {
      seen.push_back(path);
    }
  }

  assertEquals(3, seen.size());
      assertTrue(find(seen.begin(), seen.end(), dir->resolve(L"file1")) != seen.end()));
      assertTrue(find(seen.begin(), seen.end(), dir->resolve(L"file2")) != seen.end()));
      assertTrue(find(seen.begin(), seen.end(), dir->resolve(L"file3")) != seen.end()));
}

void TestShuffleFS::testActuallyShuffles() 
{
  shared_ptr<Path> dir = createTempDir();
  for (int i = 0; i < 100; i++) {
    Files::createFile(dir->resolve(L"file" + to_wstring(i)));
  }
  deque<wstring> expected = deque<wstring>();

  // get the raw listing from the actual filesystem
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir);
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      expected.push_back(path->getFileName()->toString());
    }
  }

  // shuffle until the order changes.
  for (int i = 0; i < 10000; i++) {
    shared_ptr<Path> wrapped = wrap(dir, random()->nextLong());

    deque<wstring> seen = deque<wstring>();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try
    // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
    // java.nio.file.Files.newDirectoryStream(wrapped))
    {
      shared_ptr<java::nio::file::DirectoryStream<
          std::shared_ptr<java::nio::file::Path>>>
          stream = java::nio::file::Files::newDirectoryStream(wrapped);
      for (auto path : stream) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        seen.push_back(path->getFileName()->toString());
      }
    }

    // we should always see the same files.
    assertEquals(unordered_set<>(expected), unordered_set<>(seen));
    if (!expected.equals(seen)) {
      return;
    }
  }
  fail(L"ordering never changed");
}

void TestShuffleFS::testConsistentOrder() 
{
  shared_ptr<Path> raw = createTempDir();
  for (int i = 0; i < 100; i++) {
    Files::createFile(raw->resolve(L"file" + to_wstring(i)));
  }

  int64_t seed = random()->nextLong();
  shared_ptr<Path> dirExpected = wrap(raw, seed);

  // get the shuffled listing for the seed.
  deque<wstring> expected = deque<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dirExpected))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dirExpected);
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      expected.push_back(path->getFileName()->toString());
    }
  }

  // shuffle wrapping a different scrambled ordering each time, it should always
  // be the same.
  for (int i = 0; i < 100; i++) {
    shared_ptr<Path> scrambled = wrap(raw, random()->nextLong());
    shared_ptr<Path> ordered = wrap(scrambled, seed);

    deque<wstring> seen = deque<wstring>();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try
    // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
    // java.nio.file.Files.newDirectoryStream(ordered))
    {
      shared_ptr<java::nio::file::DirectoryStream<
          std::shared_ptr<java::nio::file::Path>>>
          stream = java::nio::file::Files::newDirectoryStream(ordered);
      for (auto path : stream) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        seen.push_back(path->getFileName()->toString());
      }
    }

    // we should always see the same files in the same order
    assertEquals(expected, seen);
  }
}

void TestShuffleFS::testFileNameOnly() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  Files::createFile(dir->resolve(L"file1"));
  Files::createFile(dir->resolve(L"file2"));
  Files::createFile(dir->resolve(L"file3"));

  deque<wstring> expected = deque<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir);
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      expected.push_back(path->getFileName()->toString());
    }
  }

  shared_ptr<Path> subdir = dir->resolve(L"subdir");
  Files::createDirectory(subdir);
  Files::createFile(subdir->resolve(L"file3"));
  Files::createFile(subdir->resolve(L"file2"));
  Files::createFile(subdir->resolve(L"file1"));

  deque<wstring> actual = deque<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(subdir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(subdir);
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      actual.push_back(path->getFileName()->toString());
    }
  }

  assertEquals(expected, actual);
}
} // namespace org::apache::lucene::mockfile