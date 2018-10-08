using namespace std;

#include "TestBinaryTerms.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBinaryTerms::testBinary() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<BytesRef> bytes = make_shared<BytesRef>(2);

  for (int i = 0; i < 256; i++) {
    bytes->bytes[0] = static_cast<char>(i);
    bytes->bytes[1] = static_cast<char>(255 - i);
    bytes->length = 2;
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> customType = make_shared<FieldType>();
    customType->setStored(true);
    doc->push_back(newField(L"id", L"" + to_wstring(i), customType));
    doc->push_back(newStringField(L"bytes", bytes, Field::Store::NO));
    iw->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (int i = 0; i < 256; i++) {
    bytes->bytes[0] = static_cast<char>(i);
    bytes->bytes[1] = static_cast<char>(255 - i);
    bytes->length = 2;
    shared_ptr<TopDocs> docs = is->search(
        make_shared<TermQuery>(make_shared<Term>(L"bytes", bytes)), 5);
    assertEquals(1, docs->totalHits);
    assertEquals(L"" + to_wstring(i), is->doc(docs->scoreDocs[0]->doc)[L"id"]);
  }

  delete ir;
  delete dir;
}

void TestBinaryTerms::testToString()
{
  shared_ptr<Term> term = make_shared<Term>(
      L"foo", make_shared<BytesRef>(std::deque<char>{
                  static_cast<char>(0xff), static_cast<char>(0xfe)}));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"foo:[ff fe]", term->toString());
}
} // namespace org::apache::lucene::index