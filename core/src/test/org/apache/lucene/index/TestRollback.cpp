using namespace std;

#include "TestRollback.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestRollback::testRollbackIntegrityWithBufferFlush() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> rw =
      make_shared<RandomIndexWriter>(random(), dir);
  for (int i = 0; i < 5; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        newStringField(L"pk", Integer::toString(i), Field::Store::YES));
    rw->addDocument(doc);
  }
  delete rw;

  // If buffer size is small enough to cause a flush, errors ensue...
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2)
               ->setOpenMode(IndexWriterConfig::OpenMode::APPEND));

  for (int i = 0; i < 3; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring value = Integer::toString(i);
    doc->push_back(newStringField(L"pk", value, Field::Store::YES));
    doc->push_back(newStringField(L"text", L"foo", Field::Store::YES));
    w->updateDocument(make_shared<Term>(L"pk", value), doc);
  }
  w->rollback();

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  assertEquals(L"index should contain same number of docs post rollback", 5,
               r->numDocs());
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index