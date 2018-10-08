using namespace std;

#include "TestIndexWriterDeleteByQuery.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexWriterDeleteByQuery::testDeleteMatchAllDocsQuery() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newMaybeVirusCheckingDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  // Norms are disabled:
  doc->push_back(newStringField(L"field", L"foo", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  shared_ptr<FieldInfo> fi =
      MultiFields::getMergedFieldInfos(r)->fieldInfo(L"field");
  assertNotNull(fi);
  assertFalse(fi->hasNorms());
  assertEquals(1, r->numDocs());
  assertEquals(1, r->maxDoc());

  w->deleteDocuments({make_shared<MatchAllDocsQuery>()});
  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  r->close();

  assertNotNull(r2);
  assertEquals(0, r2->numDocs());
  assertEquals(0, r2->maxDoc());

  // Confirm the omitNorms bit is in fact no longer set:
  doc = make_shared<Document>();
  // Norms are disabled:
  doc->push_back(newTextField(L"field", L"foo", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r3 = DirectoryReader::openIfChanged(r2);
  r2->close();
  assertNotNull(r3);
  assertEquals(1, r3->numDocs());
  assertEquals(1, r3->maxDoc());

  // Make sure norms can come back to life for a field after deleting by
  // MatchAllDocsQuery:
  fi = MultiFields::getMergedFieldInfos(r3)->fieldInfo(L"field");
  assertNotNull(fi);
  assertTrue(fi->hasNorms());
  r3->close();
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index