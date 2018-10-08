using namespace std;

#include "TestVirusCheckingFS.h"

namespace org::apache::lucene::mockfile
{

shared_ptr<Path> TestVirusCheckingFS::wrap(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = (make_shared<VirusCheckingFS>(
                                   path->getFileSystem(), random()->nextLong()))
                                  ->getFileSystem(URI::create(L"file:///"));
  return make_shared<FilterPath>(path, fs);
}

void TestVirusCheckingFS::testDeleteSometimesFails() 
{
  shared_ptr<Path> dir = wrap(createTempDir());

  int counter = 0;
  while (true) {
    shared_ptr<Path> path = dir->resolve(L"file" + to_wstring(counter));
    counter++;

    shared_ptr<OutputStream> file = Files::newOutputStream(path);
    file->write(5);
    file->close();

    // File is now closed, we attempt delete:
    try {
      Files::delete (path);
    } catch (const AccessDeniedException &ade) {
      // expected (sometimes)
      assertTrue(ade->getMessage()->contains(
          L"VirusCheckingFS is randomly refusing to delete file "));
      break;
    }

    assertFalse(Files::exists(path));
  }
}
} // namespace org::apache::lucene::mockfile