using namespace std;

#include "TestFixBrokenOffsets.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/CodecReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../java/org/apache/lucene/index/FixBrokenOffsets.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestFixBrokenOffsets::testFixBrokenOffsetsIndex() 
{
  shared_ptr<InputStream> resource =
      getClass().getResourceAsStream(L"index.630.brokenoffsets.zip");
  assertNotNull(L"Broken offsets index not found", resource);
  shared_ptr<Path> path = createTempDir(L"brokenoffsets");
  TestUtil::unzip(resource, path);
  shared_ptr<Directory> dir = newFSDirectory(path);

  // OK: index is 6.3.0 so offsets not checked:
  TestUtil::checkIndex(dir);

  shared_ptr<MockDirectoryWrapper> tmpDir = newMockDirectory();
  tmpDir->setCheckIndexOnClose(false);
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(tmpDir, make_shared<IndexWriterConfig>());
  shared_ptr<IndexWriter> finalW = w;
  invalid_argument e = expectThrows(invalid_argument::typeid,
                                    [&]() { finalW->addIndexes({dir}); });
  assertTrue(e.what(),
             e.what()->startsWith(
                 L"Cannot use addIndexes(Directory) with indexes that have "
                 L"been created by a different Lucene version."));
  delete w;
  // OK: addIndexes(Directory...) refuses to execute if the index creation
  // version is different so broken offsets are not carried over
  delete tmpDir;

  shared_ptr<MockDirectoryWrapper> *const tmpDir2 = newMockDirectory();
  tmpDir2->setCheckIndexOnClose(false);
  w = make_shared<IndexWriter>(tmpDir2, make_shared<IndexWriterConfig>());
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  std::deque<std::shared_ptr<CodecReader>> codecReaders(leaves.size());
  for (int i = 0; i < leaves.size(); i++) {
    codecReaders[i] =
        std::static_pointer_cast<CodecReader>(leaves[i]->reader());
  }
  shared_ptr<IndexWriter> finalW2 = w;
  e = expectThrows(invalid_argument::typeid,
                   [&]() { finalW2->addIndexes(codecReaders); });
  TestUtil::assertEquals(
      L"Cannot merge a segment that has been created with major version 6 into "
      L"this index which has been created by major version 7",
      e.what());
  reader->close();
  delete w;
  delete tmpDir2;

  // Now run the tool and confirm the broken offsets are fixed:
  shared_ptr<Path> path2 =
      createTempDir(L"fixedbrokenoffsets")->resolve(L"subdir");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  FixBrokenOffsets::main(
      std::deque<wstring>{path->toString(), path2->toString()});
  shared_ptr<Directory> tmpDir3 = FSDirectory::open(path2);
  TestUtil::checkIndex(tmpDir3);
  delete tmpDir3;

  delete dir;
}
} // namespace org::apache::lucene::index