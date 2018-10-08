using namespace std;

#include "TestExtrasFS.h"

namespace org::apache::lucene::mockfile
{

shared_ptr<Path> TestExtrasFS::wrap(shared_ptr<Path> path)
{
  return wrap(path, random()->nextBoolean(), random()->nextBoolean());
}

shared_ptr<Path> TestExtrasFS::wrap(shared_ptr<Path> path, bool active,
                                    bool createDirectory)
{
  shared_ptr<FileSystem> fs =
      (make_shared<ExtrasFS>(path->getFileSystem(), active, createDirectory))
          ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestExtrasFS::testExtraFile() 
{
  shared_ptr<Path> dir = wrap(createTempDir(), true, false);
  Files::createDirectory(dir->resolve(L"foobar"));

  deque<wstring> seen = deque<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir.resolve("foobar")))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream =
            java::nio::file::Files::newDirectoryStream(dir->resolve(L"foobar"));
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      seen.push_back(path->getFileName()->toString());
    }
  }
  assertEquals(Arrays::asList(L"extra0"), seen);
  assertTrue(Files::isRegularFile(dir->resolve(L"foobar").resolve(L"extra0")));
}

void TestExtrasFS::testExtraDirectory() 
{
  shared_ptr<Path> dir = wrap(createTempDir(), true, true);
  Files::createDirectory(dir->resolve(L"foobar"));

  deque<wstring> seen = deque<wstring>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir.resolve("foobar")))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream =
            java::nio::file::Files::newDirectoryStream(dir->resolve(L"foobar"));
    for (auto path : stream) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      seen.push_back(path->getFileName()->toString());
    }
  }
  assertEquals(Arrays::asList(L"extra0"), seen);
  assertTrue(Files::isDirectory(dir->resolve(L"foobar").resolve(L"extra0")));
}

void TestExtrasFS::testNoExtras() 
{
  shared_ptr<Path> dir = wrap(createTempDir(), false, false);
  Files::createDirectory(dir->resolve(L"foobar"));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir.resolve("foobar")))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream =
            java::nio::file::Files::newDirectoryStream(dir->resolve(L"foobar"));
    for (auto path : stream) {
      fail(L"should not have found file: " + path);
    }
  }
}
} // namespace org::apache::lucene::mockfile