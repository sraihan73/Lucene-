using namespace std;

#include "TestIndexWriterOnOldIndex.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"

namespace org::apache::lucene::index
{
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestIndexWriterOnOldIndex::testOpenModeAndCreatedVersion() throw(
    IOException)
{
  shared_ptr<InputStream> resource =
      getClass().getResourceAsStream(L"index.single-empty-doc.630.zip");
  assertNotNull(resource);
  shared_ptr<Path> path = createTempDir();
  TestUtil::unzip(resource, path);
  shared_ptr<Directory> dir = newFSDirectory(path);
  for (OpenMode openMode : OpenMode::values()) {
    shared_ptr<Directory> tmpDir = newDirectory(dir);
    TestUtil::assertEquals(
        6,
        SegmentInfos::readLatestCommit(tmpDir)->getIndexCreatedVersionMajor());
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        tmpDir, newIndexWriterConfig()->setOpenMode(openMode));
    w->commit();
    delete w;
    switch (openMode) {
    case OpenMode::CREATE:
      TestUtil::assertEquals(Version::LATEST->major,
                             SegmentInfos::readLatestCommit(tmpDir)
                                 ->getIndexCreatedVersionMajor());
      break;
    default:
      TestUtil::assertEquals(6, SegmentInfos::readLatestCommit(tmpDir)
                                    ->getIndexCreatedVersionMajor());
    }
    delete tmpDir;
  }
  delete dir;
}
} // namespace org::apache::lucene::index