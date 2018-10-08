using namespace std;

#include "TestNewestSegment.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNewestSegment::testNewestSegment() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  assertNull(writer->newestSegment());
  delete writer;
  delete directory;
}
} // namespace org::apache::lucene::index