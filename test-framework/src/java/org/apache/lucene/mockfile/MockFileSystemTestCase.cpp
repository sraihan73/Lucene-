using namespace std;

#include "MockFileSystemTestCase.h"

namespace org::apache::lucene::mockfile
{
using Constants = org::apache::lucene::util::Constants;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressFileSystems;

void MockFileSystemTestCase::testHashCodeEquals() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<Path> f1 = dir->resolve(L"file1");
  shared_ptr<Path> f1Again = dir->resolve(L"file1");
  shared_ptr<Path> f2 = dir->resolve(L"file2");

  assertEquals(f1, f1);
  assertFalse(f1->equals(nullptr));
  assertEquals(f1, f1Again);
  assertEquals(f1->hashCode(), f1Again->hashCode());
  assertFalse(f1->equals(f2));
  dir->getFileSystem()->close();
}

void MockFileSystemTestCase::testURI() 
{
  implTestURI(L"file1"); // plain ASCII
}

void MockFileSystemTestCase::testURIumlaute() 
{
  implTestURI(L"äÄöÖüÜß"); // Umlaute and s-zet
}

void MockFileSystemTestCase::testURIchinese() 
{
  implTestURI(L"中国"); // chinese
}

void MockFileSystemTestCase::implTestURI(const wstring &fileName) throw(
    IOException)
{
  assumeFalse(
      L"broken on J9: see https://issues.apache.org/jira/browse/LUCENE-6517",
      StringHelper::startsWith(Constants::JAVA_VENDOR, L"IBM"));
  shared_ptr<Path> dir = wrap(createTempDir());

  try {
    dir->resolve(fileName);
  } catch (const InvalidPathException &ipe) {
    assumeNoException(L"couldn't resolve '" + fileName + L"'", ipe);
  }

  shared_ptr<Path> f1 = dir->resolve(fileName);
  shared_ptr<URI> uri = f1->toUri();
  shared_ptr<Path> f2 = dir->getFileSystem().provider().getPath(uri);
  assertEquals(f1, f2);

  dir->getFileSystem()->close();
}

void MockFileSystemTestCase::testDirectoryStreamFiltered() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file =
      Files::newOutputStream(dir->resolve(L"file1"));
  file->write(5);
  file->close();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir);
    int count = 0;
    for (auto path : stream) {
      assertTrue(std::dynamic_pointer_cast<FilterPath>(path) != nullptr);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      if (!path->getFileName()->toString()->startsWith(L"extra")) {
        count++;
      }
    }
    assertEquals(1, count);
  }
  dir->getFileSystem()->close();
}

void MockFileSystemTestCase::testDirectoryStreamGlobFiltered() throw(
    IOException)
{
  shared_ptr<Path> dir = wrap(createTempDir());

  shared_ptr<OutputStream> file = Files::newOutputStream(dir->resolve(L"foo"));
  file->write(5);
  file->close();
  file = Files::newOutputStream(dir->resolve(L"bar"));
  file->write(5);
  file->close();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(dir, "f*"))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(dir, L"f*");
    int count = 0;
    for (auto path : stream) {
      assertTrue(std::dynamic_pointer_cast<FilterPath>(path) != nullptr);
      ++count;
    }
    assertEquals(1, count);
  }
  dir->getFileSystem()->close();
}
} // namespace org::apache::lucene::mockfile