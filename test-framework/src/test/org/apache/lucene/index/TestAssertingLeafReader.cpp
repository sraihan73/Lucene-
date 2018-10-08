using namespace std;

#include "TestAssertingLeafReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestAssertingLeafReader::testAssertBits() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  // Not deleted:
  w->addDocument(make_shared<Document>());

  // Does get deleted:
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"0", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  w->deleteDocuments({make_shared<Term>(L"id", L"0")});
  delete w;

  // Now we have index with 1 segment with 2 docs one of which is marked deleted

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  assertEquals(1, r->leaves().size());
  assertEquals(2, r->maxDoc());
  assertEquals(1, r->numDocs());

  r = make_shared<AssertingDirectoryReader>(
      std::static_pointer_cast<DirectoryReader>(r));
  shared_ptr<IndexReader> *const r2 = r;

  shared_ptr<Thread> thread =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this(), r2);
  thread->start();
  thread->join();

  IOUtils::close({r, dir});
}

TestAssertingLeafReader::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestAssertingLeafReader> outerInstance,
    shared_ptr<org::apache::lucene::index::IndexReader> r2)
{
  this->outerInstance = outerInstance;
  this->r2 = r2;
}

void TestAssertingLeafReader::ThreadAnonymousInnerClass::run()
{
  for (auto context : r2->leaves()) {
    context->reader()->getLiveDocs()->get(0);
  }
}
} // namespace org::apache::lucene::index