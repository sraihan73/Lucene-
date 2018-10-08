using namespace std;

#include "TestSimpleFSDirectory.h"

namespace org::apache::lucene::store
{
using FilterPath = org::apache::lucene::mockfile::FilterPath;
using WindowsFS = org::apache::lucene::mockfile::WindowsFS;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<Directory>
TestSimpleFSDirectory::getDirectory(shared_ptr<Path> path) 
{
  return make_shared<SimpleFSDirectory>(path);
}

void TestSimpleFSDirectory::testRenameWithPendingDeletes() 
{
  shared_ptr<Path> path = createTempDir();
  // irony: currently we don't emulate windows well enough to work on windows!
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  // Use WindowsFS to prevent open files from being deleted:
  shared_ptr<FileSystem> fs = (make_shared<WindowsFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  shared_ptr<Path> root = make_shared<FilterPath>(path, fs);
  shared_ptr<Directory> directory = getDirectory(root);
  shared_ptr<IndexOutput> output =
      directory->createOutput(L"target.txt", IOContext::DEFAULT);
  output->writeInt(1);
  delete output;
  shared_ptr<IndexOutput> output1 =
      directory->createOutput(L"source.txt", IOContext::DEFAULT);
  output1->writeInt(2);
  delete output1;

  shared_ptr<IndexInput> input =
      directory->openInput(L"target.txt", IOContext::DEFAULT);
  directory->deleteFile(L"target.txt");
  directory->rename(L"source.txt", L"target.txt");
  shared_ptr<IndexInput> input1 =
      directory->openInput(L"target.txt", IOContext::DEFAULT);
  assertTrue(directory->getPendingDeletions()->isEmpty());
  assertEquals(1, input->readInt());
  assertEquals(2, input1->readInt());
  IOUtils::close({input1, input, directory});
}

void TestSimpleFSDirectory::testCreateOutputWithPendingDeletes() throw(
    IOException)
{
  // irony: currently we don't emulate windows well enough to work on windows!
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> path = createTempDir();
  // Use WindowsFS to prevent open files from being deleted:
  shared_ptr<FileSystem> fs = (make_shared<WindowsFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  shared_ptr<Path> root = make_shared<FilterPath>(path, fs);
  shared_ptr<Directory> directory = getDirectory(root);
  shared_ptr<IndexOutput> output =
      directory->createOutput(L"file.txt", IOContext::DEFAULT);
  output->writeInt(1);
  delete output;
  shared_ptr<IndexInput> input =
      directory->openInput(L"file.txt", IOContext::DEFAULT);
  directory->deleteFile(L"file.txt");
  expectThrows(IOException::typeid, [&]() {
    directory->createOutput(L"file.txt", IOContext::DEFAULT);
  });
  assertTrue(directory->getPendingDeletions()->isEmpty());
  assertEquals(1, input->readInt());
  IOUtils::close({input, directory});
}
} // namespace org::apache::lucene::store