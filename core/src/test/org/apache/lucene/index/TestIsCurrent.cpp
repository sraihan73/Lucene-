using namespace std;

#include "TestIsCurrent.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using namespace org::apache::lucene::util;
using namespace org::apache::lucene::store;
using org::junit::Test;

void TestIsCurrent::setUp() 
{
  LuceneTestCase::setUp();

  // initialize directory
  directory = newDirectory();
  writer = make_shared<RandomIndexWriter>(random(), directory);

  // write document
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"UUID", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();
}

void TestIsCurrent::tearDown() 
{
  LuceneTestCase::tearDown();
  delete writer;
  delete directory;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeleteByTermIsCurrent() throws
// java.io.IOException
void TestIsCurrent::testDeleteByTermIsCurrent() 
{

  // get reader
  shared_ptr<DirectoryReader> reader = writer->getReader();

  // assert index has a document and reader is up2date
  assertEquals(L"One document should be in the index", 1, writer->numDocs());
  assertTrue(L"One document added, reader should be current",
             reader->isCurrent());

  // remove document
  shared_ptr<Term> idTerm = make_shared<Term>(L"UUID", L"1");
  writer->deleteDocuments(idTerm);
  writer->commit();

  // assert document has been deleted (index changed), reader is stale
  assertEquals(L"Document should be removed", 0, writer->numDocs());
  assertFalse(L"Reader should be stale", reader->isCurrent());

  reader->close();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDeleteAllIsCurrent() throws
// java.io.IOException
void TestIsCurrent::testDeleteAllIsCurrent() 
{

  // get reader
  shared_ptr<DirectoryReader> reader = writer->getReader();

  // assert index has a document and reader is up2date
  assertEquals(L"One document should be in the index", 1, writer->numDocs());
  assertTrue(L"Document added, reader should be stale ", reader->isCurrent());

  // remove all documents
  writer->deleteAll();
  writer->commit();

  // assert document has been deleted (index changed), reader is stale
  assertEquals(L"Document should be removed", 0, writer->numDocs());
  assertFalse(L"Reader should be stale", reader->isCurrent());

  reader->close();
}
} // namespace org::apache::lucene::index