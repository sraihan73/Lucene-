using namespace std;

#include "TestDocument.h"

namespace org::apache::lucene::document
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDocument::testBinaryField() 
{
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setStored(true);
  shared_ptr<Field> stringFld = make_shared<Field>(L"string", binaryVal, ft);
  shared_ptr<StoredField> binaryFld = make_shared<StoredField>(
      L"binary", binaryVal.getBytes(StandardCharsets::UTF_8));
  shared_ptr<StoredField> binaryFld2 = make_shared<StoredField>(
      L"binary", binaryVal2.getBytes(StandardCharsets::UTF_8));

  doc->push_back(stringFld);
  doc->push_back(binaryFld);

  assertEquals(2, doc->getFields().size());

  assertTrue(binaryFld->binaryValue() != nullptr);
  assertTrue(binaryFld->fieldType()->stored());
  assertEquals(IndexOptions::NONE, binaryFld->fieldType()->indexOptions());

  wstring binaryTest = doc->getBinaryValue(L"binary")->utf8ToString();
  assertTrue(binaryTest == binaryVal);

  wstring stringTest = doc[L"string"];
  assertTrue(binaryTest == stringTest);

  doc->push_back(binaryFld2);

  assertEquals(3, doc->getFields().size());

  std::deque<std::shared_ptr<BytesRef>> binaryTests =
      doc->getBinaryValues(L"binary");

  assertEquals(2, binaryTests.size());

  binaryTest = binaryTests[0]->utf8ToString();
  wstring binaryTest2 = binaryTests[1]->utf8ToString();

  assertFalse(binaryTest == binaryTest2);

  assertTrue(binaryTest == binaryVal);
  assertTrue(binaryTest2 == binaryVal2);

  doc->removeField(L"string");
  assertEquals(2, doc->getFields().size());

  doc->removeFields(L"binary");
  assertEquals(0, doc->getFields().size());
}

void TestDocument::testRemoveForNewDocument() 
{
  shared_ptr<Document> doc = makeDocumentWithFields();
  assertEquals(10, doc->getFields().size());
  doc->removeFields(L"keyword");
  assertEquals(8, doc->getFields().size());
  doc->removeFields(L"doesnotexists"); // removing non-existing fields is
                                       // siltenlty ignored
  doc->removeFields(L"keyword");       // removing a field more than once
  assertEquals(8, doc->getFields().size());
  doc->removeField(L"text");
  assertEquals(7, doc->getFields().size());
  doc->removeField(L"text");
  assertEquals(6, doc->getFields().size());
  doc->removeField(L"text");
  assertEquals(6, doc->getFields().size());
  doc->removeField(L"doesnotexists"); // removing non-existing fields is
                                      // siltenlty ignored
  assertEquals(6, doc->getFields().size());
  doc->removeFields(L"unindexed");
  assertEquals(4, doc->getFields().size());
  doc->removeFields(L"unstored");
  assertEquals(2, doc->getFields().size());
  doc->removeFields(L"doesnotexists"); // removing non-existing fields is
                                       // siltenlty ignored
  assertEquals(2, doc->getFields().size());

  doc->removeFields(L"indexed_not_tokenized");
  assertEquals(0, doc->getFields().size());
}

void TestDocument::testConstructorExceptions() 
{
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setStored(true);
  make_shared<Field>(L"name", L"value", ft);                     // okay
  make_shared<StringField>(L"name", L"value", Field::Store::NO); // okay
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Field>(L"name", L"value", make_shared<FieldType>());
  });

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  make_shared<Field>(L"name", L"value", ft); // okay
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft2 = make_shared<FieldType>();
  ft2->setStored(true);
  ft2->setStoreTermVectors(true);
  doc->push_back(make_shared<Field>(L"name", L"value", ft2));
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  delete w;
  delete dir;
}

void TestDocument::testClearDocument()
{
  shared_ptr<Document> doc = makeDocumentWithFields();
  assertEquals(10, doc->getFields().size());
  doc->clear();
  assertEquals(0, doc->getFields().size());
}

void TestDocument::testGetFieldsImmutable()
{
  shared_ptr<Document> doc = makeDocumentWithFields();
  assertEquals(10, doc->getFields().size());
  deque<std::shared_ptr<IndexableField>> fields = doc->getFields();
  expectThrows(UnsupportedOperationException::typeid, [&]() {
    fields.add(make_shared<StringField>(L"name", L"value", Field::Store::NO));
  });

  expectThrows(UnsupportedOperationException::typeid,
               [&]() { fields.clear(); });
}

void TestDocument::testGetValuesForNewDocument() 
{
  doAssert(makeDocumentWithFields(), false);
}

void TestDocument::testGetValuesForIndexedDocument() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  writer->addDocument(makeDocumentWithFields());
  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // search for something that does exist
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"keyword", L"test1"));

  // ensure that queries return expected results without DateFilter first
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  doAssert(searcher->doc(hits[0]->doc), true);
  delete writer;
  delete reader;
  delete dir;
}

void TestDocument::testGetValues()
{
  shared_ptr<Document> doc = makeDocumentWithFields();
  assertEquals(std::deque<wstring>{L"test1", L"test2"},
               doc->getValues(L"keyword"));
  assertEquals(std::deque<wstring>{L"test1", L"test2"},
               doc->getValues(L"text"));
  assertEquals(std::deque<wstring>{L"test1", L"test2"},
               doc->getValues(L"unindexed"));
  assertEquals(std::deque<wstring>(0), doc->getValues(L"nope"));
}

void TestDocument::testPositionIncrementMultiFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  writer->addDocument(makeDocumentWithFields());
  shared_ptr<IndexReader> reader = writer->getReader();

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<PhraseQuery> query =
      make_shared<PhraseQuery>(L"indexed_not_tokenized", L"test1", L"test2");

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  doAssert(searcher->doc(hits[0]->doc), true);
  delete writer;
  delete reader;
  delete dir;
}

shared_ptr<Document> TestDocument::makeDocumentWithFields()
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> stored = make_shared<FieldType>();
  stored->setStored(true);
  shared_ptr<FieldType> indexedNotTokenized = make_shared<FieldType>();
  indexedNotTokenized->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  indexedNotTokenized->setTokenized(false);
  doc->push_back(
      make_shared<StringField>(L"keyword", L"test1", Field::Store::YES));
  doc->push_back(
      make_shared<StringField>(L"keyword", L"test2", Field::Store::YES));
  doc->push_back(make_shared<TextField>(L"text", L"test1", Field::Store::YES));
  doc->push_back(make_shared<TextField>(L"text", L"test2", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"unindexed", L"test1", stored));
  doc->push_back(make_shared<Field>(L"unindexed", L"test2", stored));
  doc->push_back(
      make_shared<TextField>(L"unstored", L"test1", Field::Store::NO));
  doc->push_back(
      make_shared<TextField>(L"unstored", L"test2", Field::Store::NO));
  doc->push_back(make_shared<Field>(L"indexed_not_tokenized", L"test1",
                                    indexedNotTokenized));
  doc->push_back(make_shared<Field>(L"indexed_not_tokenized", L"test2",
                                    indexedNotTokenized));
  return doc;
}

void TestDocument::doAssert(shared_ptr<Document> doc, bool fromIndex)
{
  std::deque<std::shared_ptr<IndexableField>> keywordFieldValues =
      doc->getFields(L"keyword");
  std::deque<std::shared_ptr<IndexableField>> textFieldValues =
      doc->getFields(L"text");
  std::deque<std::shared_ptr<IndexableField>> unindexedFieldValues =
      doc->getFields(L"unindexed");
  std::deque<std::shared_ptr<IndexableField>> unstoredFieldValues =
      doc->getFields(L"unstored");

  assertTrue(keywordFieldValues.size() == 2);
  assertTrue(textFieldValues.size() == 2);
  assertTrue(unindexedFieldValues.size() == 2);
  // this test cannot work for documents retrieved from the index
  // since unstored fields will obviously not be returned
  if (!fromIndex) {
    assertTrue(unstoredFieldValues.size() == 2);
  }

  assertTrue(keywordFieldValues[0]->stringValue() == L"test1");
  assertTrue(keywordFieldValues[1]->stringValue() == L"test2");
  assertTrue(textFieldValues[0]->stringValue() == L"test1");
  assertTrue(textFieldValues[1]->stringValue() == L"test2");
  assertTrue(unindexedFieldValues[0]->stringValue() == L"test1");
  assertTrue(unindexedFieldValues[1]->stringValue() == L"test2");
  // this test cannot work for documents retrieved from the index
  // since unstored fields will obviously not be returned
  if (!fromIndex) {
    assertTrue(unstoredFieldValues[0]->stringValue() == L"test1");
    assertTrue(unstoredFieldValues[1]->stringValue() == L"test2");
  }
}

void TestDocument::testFieldSetValue() 
{

  shared_ptr<Field> field =
      make_shared<StringField>(L"id", L"id1", Field::Store::YES);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(field);
  doc->push_back(
      make_shared<StringField>(L"keyword", L"test", Field::Store::YES));

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  writer->addDocument(doc);
  field->setStringValue(L"id2");
  writer->addDocument(doc);
  field->setStringValue(L"id3");
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"keyword", L"test"));

  // ensure that queries return expected results without DateFilter first
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;
  assertEquals(3, hits.size());
  int result = 0;
  for (int i = 0; i < 3; i++) {
    shared_ptr<Document> doc2 = searcher->doc(hits[i]->doc);
    shared_ptr<Field> f =
        std::static_pointer_cast<Field>(doc2->getField(L"id"));
    if (f->stringValue() == L"id1") {
      result |= 1;
    } else if (f->stringValue() == L"id2") {
      result |= 2;
    } else if (f->stringValue() == L"id3") {
      result |= 4;
    } else {
      fail(L"unexpected id field");
    }
  }
  delete writer;
  delete reader;
  delete dir;
  assertEquals(L"did not see all IDs", 7, result);
}

void TestDocument::testInvalidFields()
{
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<Tokenizer> tok = make_shared<MockTokenizer>();
    tok->setReader(make_shared<StringReader>(L""));
    make_shared<Field>(L"foo", tok, StringField::TYPE_STORED);
  });
}

void TestDocument::testNumericFieldAsString() 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StoredField>(L"int", 5));
  assertEquals(L"5", doc[L"int"]);
  assertNull(doc[L"somethingElse"]);
  doc->push_back(make_shared<StoredField>(L"int", 4));
  assertArrayEquals(std::deque<wstring>{L"5", L"4"}, doc->getValues(L"int"));

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  iw->addDocument(doc);
  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<Document> sdoc = ir->document(0);
  assertEquals(L"5", sdoc[L"int"]);
  assertNull(sdoc[L"somethingElse"]);
  assertArrayEquals(std::deque<wstring>{L"5", L"4"}, sdoc->getValues(L"int"));
  ir->close();
  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::document