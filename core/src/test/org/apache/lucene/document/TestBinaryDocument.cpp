using namespace std;

#include "TestBinaryDocument.h"

namespace org::apache::lucene::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBinaryDocument::testBinaryFieldInIndex() 
{
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setStored(true);
  shared_ptr<StoredField> binaryFldStored = make_shared<StoredField>(
      L"binaryStored", binaryValStored.getBytes(StandardCharsets::UTF_8));
  shared_ptr<Field> stringFldStored =
      make_shared<Field>(L"stringStored", binaryValStored, ft);

  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(binaryFldStored);

  doc->push_back(stringFldStored);

  /** test for field count */
  assertEquals(2, doc->getFields().size());

  /** add the doc to a ram index */
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  writer->addDocument(doc);

  /** open a reader and fetch the document */
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<Document> docFromReader = reader->document(0);
  assertTrue(docFromReader->size() > 0);

  /** fetch the binary stored field and compare its content with the original
   * one */
  shared_ptr<BytesRef> bytes = docFromReader->getBinaryValue(L"binaryStored");
  assertNotNull(bytes);
  wstring binaryFldStoredTest = wstring(bytes->bytes, bytes->offset,
                                        bytes->length, StandardCharsets::UTF_8);
  assertTrue(binaryFldStoredTest == binaryValStored);

  /** fetch the string field and compare its content with the original one */
  wstring stringFldStoredTest = docFromReader[L"stringStored"];
  assertTrue(stringFldStoredTest == binaryValStored);

  delete writer;
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::document