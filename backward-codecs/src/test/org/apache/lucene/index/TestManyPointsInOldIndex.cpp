using namespace std;

#include "TestManyPointsInOldIndex.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/IntPoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using IntPoint = org::apache::lucene::document::IntPoint;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestManyPointsInOldIndex::main(std::deque<wstring> &args) throw(
    IOException)
{
  shared_ptr<Directory> dir = FSDirectory::open(Paths->get(L"manypointsindex"));
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>());
  for (int i = 0; i < 1025; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"intpoint", 1025 - i));
    w->addDocument(doc);
  }
  delete w;
  delete dir;
}

void TestManyPointsInOldIndex::testCheckOldIndex() 
{
  shared_ptr<Path> path = createTempDir(L"manypointsindex");
  shared_ptr<InputStream> resource =
      getClass().getResourceAsStream(L"manypointsindex.zip");
  assertNotNull(L"manypointsindex not found", resource);
  TestUtil::unzip(resource, path);
  shared_ptr<BaseDirectoryWrapper> dir = newFSDirectory(path);
  // disable default checking...
  dir->setCheckIndexOnClose(false);

  // ... because we check ourselves here:
  TestUtil::checkIndex(dir, false, true, nullptr);
  delete dir;
}
} // namespace org::apache::lucene::index