using namespace std;

#include "TestHardLinkCopyDirectoryWrapper.h"

namespace org::apache::lucene::store
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FilterPath = org::apache::lucene::mockfile::FilterPath;
using WindowsFS = org::apache::lucene::mockfile::WindowsFS;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<Directory> TestHardLinkCopyDirectoryWrapper::getDirectory(
    shared_ptr<Path> file) 
{
  shared_ptr<Directory> open;
  if (random()->nextBoolean()) {
    open = make_shared<RAMDirectory>();
  } else {
    open = FSDirectory::open(file);
  }
  return make_shared<HardlinkCopyDirectoryWrapper>(open);
}

void TestHardLinkCopyDirectoryWrapper::testCopyHardLinks() 
{
  shared_ptr<Path> tempDir = createTempDir();
  shared_ptr<Path> dir_1 = tempDir->resolve(L"dir_1");
  shared_ptr<Path> dir_2 = tempDir->resolve(L"dir_2");
  Files::createDirectories(dir_1);
  Files::createDirectories(dir_2);

  shared_ptr<Directory> luceneDir_1 = newFSDirectory(dir_1);
  shared_ptr<Directory> luceneDir_2 = newFSDirectory(dir_2);
  try {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexOutput output =
    // luceneDir_1.createOutput("foo.bar", IOContext.DEFAULT))
    {
      IndexOutput output =
          luceneDir_1->createOutput(L"foo.bar", IOContext::DEFAULT);
      CodecUtil::writeHeader(output, L"foo", 0);
      output->writeString(L"hey man, nice shot!");
      CodecUtil::writeFooter(output);
    }
    // In case luceneDir_1 has an NRTCachingDirectory
    luceneDir_1->sync(Collections::singleton(L"foo.bar"));
    try {
      Files::createLink(tempDir->resolve(L"test"), dir_1->resolve(L"foo.bar"));
      shared_ptr<BasicFileAttributes> destAttr = Files::readAttributes(
          tempDir->resolve(L"test"), BasicFileAttributes::typeid);
      shared_ptr<BasicFileAttributes> sourceAttr = Files::readAttributes(
          dir_1->resolve(L"foo.bar"), BasicFileAttributes::typeid);
      assumeTrue(L"hardlinks are not supported",
                 destAttr->fileKey() != nullptr &&
                     destAttr->fileKey().equals(sourceAttr->fileKey()));
    } catch (const UnsupportedOperationException &ex) {
      assumeFalse(L"hardlinks are not supported", true);
    }

    shared_ptr<HardlinkCopyDirectoryWrapper> wrapper =
        make_shared<HardlinkCopyDirectoryWrapper>(luceneDir_2);
    wrapper->copyFrom(luceneDir_1, L"foo.bar", L"bar.foo", IOContext::DEFAULT);
    assertTrue(Files::exists(dir_2->resolve(L"bar.foo")));
    shared_ptr<BasicFileAttributes> destAttr = Files::readAttributes(
        dir_2->resolve(L"bar.foo"), BasicFileAttributes::typeid);
    shared_ptr<BasicFileAttributes> sourceAttr = Files::readAttributes(
        dir_1->resolve(L"foo.bar"), BasicFileAttributes::typeid);
    assertEquals(destAttr->fileKey(), sourceAttr->fileKey());
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (ChecksumIndexInput indexInput =
    // wrapper.openChecksumInput("bar.foo", IOContext.DEFAULT))
    {
      ChecksumIndexInput indexInput =
          wrapper->openChecksumInput(L"bar.foo", IOContext::DEFAULT);
      CodecUtil::checkHeader(indexInput, L"foo", 0, 0);
      assertEquals(L"hey man, nice shot!", indexInput->readString());
      CodecUtil::checkFooter(indexInput);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // close them in a finally block we might run into an assume here
    IOUtils::close({luceneDir_1, luceneDir_2});
  }
}

void TestHardLinkCopyDirectoryWrapper::testRenameWithHardLink() throw(
    runtime_error)
{
  // irony: currently we don't emulate windows well enough to work on windows!
  assumeFalse(L"windows is not supported", Constants::WINDOWS);
  shared_ptr<Path> path = createTempDir();
  shared_ptr<FileSystem> fs = (make_shared<WindowsFS>(path->getFileSystem()))
                                  ->getFileSystem(URI::create(L"file:///"));
  shared_ptr<Directory> dir1 =
      make_shared<SimpleFSDirectory>(make_shared<FilterPath>(path, fs));
  shared_ptr<Directory> dir2 = make_shared<SimpleFSDirectory>(
      make_shared<FilterPath>(path->resolve(L"link"), fs));

  shared_ptr<IndexOutput> target =
      dir1->createOutput(L"target.txt", IOContext::DEFAULT);
  target->writeInt(1);
  delete target;

  shared_ptr<HardlinkCopyDirectoryWrapper> wrapper =
      make_shared<HardlinkCopyDirectoryWrapper>(dir2);
  wrapper->copyFrom(dir1, L"target.txt", L"link.txt", IOContext::DEFAULT);

  shared_ptr<IndexOutput> source =
      dir1->createOutput(L"source.txt", IOContext::DEFAULT);
  source->writeInt(2);
  delete source;

  shared_ptr<IndexInput> link =
      dir2->openInput(L"link.txt", IOContext::DEFAULT);
  // Rename while opening a hard-link file
  dir1->rename(L"source.txt", L"target.txt");
  delete link;

  shared_ptr<IndexInput> in_ =
      dir1->openInput(L"target.txt", IOContext::DEFAULT);
  assertEquals(2, in_->readInt());
  delete in_;

  IOUtils::close({dir1, dir2});
}
} // namespace org::apache::lucene::store