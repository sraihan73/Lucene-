using namespace std;

#include "TestSort.h"

namespace org::apache::lucene::search
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSort::assertEquals(shared_ptr<Sort> a, shared_ptr<Sort> b)
{
  LuceneTestCase::assertEquals(a, b);
  LuceneTestCase::assertEquals(b, a);
  LuceneTestCase::assertEquals(a->hashCode(), b->hashCode());
}

void TestSort::assertDifferent(shared_ptr<Sort> a, shared_ptr<Sort> b)
{
  assertFalse(a->equals(b));
  assertFalse(b->equals(a));
  assertFalse(a->hashCode() == b->hashCode());
}

void TestSort::testEquals()
{
  shared_ptr<SortField> sortField1 =
      make_shared<SortField>(L"foo", SortField::Type::STRING);
  shared_ptr<SortField> sortField2 =
      make_shared<SortField>(L"foo", SortField::Type::STRING);
  assertEquals(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));

  sortField2 = make_shared<SortField>(L"bar", SortField::Type::STRING);
  assertDifferent(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));

  sortField2 = make_shared<SortField>(L"foo", SortField::Type::LONG);
  assertDifferent(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));

  sortField2 = make_shared<SortField>(L"foo", SortField::Type::STRING);
  sortField2->setMissingValue(SortField::STRING_FIRST);
  assertDifferent(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));

  sortField2 = make_shared<SortField>(L"foo", SortField::Type::STRING, false);
  assertEquals(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));

  sortField2 = make_shared<SortField>(L"foo", SortField::Type::STRING, true);
  assertDifferent(make_shared<Sort>(sortField1), make_shared<Sort>(sortField2));
}

void TestSort::testString() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'foo'
  assertEquals(L"bar", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"foo", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testStringReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'foo' comes after 'bar' in reverse order
  assertEquals(L"foo", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"bar", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testStringVal() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING_VAL));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'foo'
  assertEquals(L"bar", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"foo", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testStringValReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING_VAL, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'foo' comes after 'bar' in reverse order
  assertEquals(L"foo", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"bar", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testStringValSorted() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING_VAL));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'foo'
  assertEquals(L"bar", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"foo", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testStringValReverseSorted() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"value", L"bar", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(newStringField(L"value", L"foo", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::STRING_VAL, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'foo' comes after 'bar' in reverse order
  assertEquals(L"foo", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"bar", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testInt() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 300000));
  doc->push_back(newStringField(L"value", L"300000", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort =
      make_shared<Sort>(make_shared<SortField>(L"value", SortField::Type::INT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // numeric order
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"300000", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testIntReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 300000));
  doc->push_back(newStringField(L"value", L"300000", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::INT, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // reverse numeric order
  assertEquals(L"300000", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"-1", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testIntMissing() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort =
      make_shared<Sort>(make_shared<SortField>(L"value", SortField::Type::INT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as a 0
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testIntMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"value", SortField::Type::INT);
  sortField->setMissingValue(numeric_limits<int>::max());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as a Integer.MAX_VALUE
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testLong() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 3000000000LL));
  doc->push_back(newStringField(L"value", L"3000000000", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::LONG));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // numeric order
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"3000000000", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testLongReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 3000000000LL));
  doc->push_back(newStringField(L"value", L"3000000000", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::LONG, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // reverse numeric order
  assertEquals(L"3000000000", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"-1", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testLongMissing() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::LONG));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as 0
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testLongMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", -1));
  doc->push_back(newStringField(L"value", L"-1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"value", 4));
  doc->push_back(newStringField(L"value", L"4", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"value", SortField::Type::LONG);
  sortField->setMissingValue(numeric_limits<int64_t>::max());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as Long.MAX_VALUE
  assertEquals(L"-1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testFloat() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 30.1F));
  doc->push_back(newStringField(L"value", L"30.1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", -1.3F));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 4.2F));
  doc->push_back(newStringField(L"value", L"4.2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::FLOAT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // numeric order
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"30.1", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testFloatReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 30.1F));
  doc->push_back(newStringField(L"value", L"30.1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", -1.3F));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 4.2F));
  doc->push_back(newStringField(L"value", L"4.2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::FLOAT, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // reverse numeric order
  assertEquals(L"30.1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testFloatMissing() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", -1.3F));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 4.2F));
  doc->push_back(newStringField(L"value", L"4.2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::FLOAT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as 0
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4.2", searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testFloatMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", -1.3F));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatDocValuesField>(L"value", 4.2F));
  doc->push_back(newStringField(L"value", L"4.2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"value", SortField::Type::FLOAT);
  sortField->setMissingValue(numeric_limits<float>::max());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null is treated as Float.MAX_VALUE
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[2]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testDouble() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 30.1));
  doc->push_back(newStringField(L"value", L"30.1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", -1.3));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333333));
  doc->push_back(
      newStringField(L"value", L"4.2333333333333", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333332));
  doc->push_back(
      newStringField(L"value", L"4.2333333333332", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::DOUBLE));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(4, td->totalHits);
  // numeric order
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2333333333332",
               searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4.2333333333333",
               searcher->doc(td->scoreDocs[2]->doc)[L"value"]);
  assertEquals(L"30.1", searcher->doc(td->scoreDocs[3]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testDoubleSignedZero() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", +0));
  doc->push_back(newStringField(L"value", L"+0", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", -0));
  doc->push_back(newStringField(L"value", L"-0", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::DOUBLE));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // numeric order
  assertEquals(L"-0", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"+0", searcher->doc(td->scoreDocs[1]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testDoubleReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 30.1));
  doc->push_back(newStringField(L"value", L"30.1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", -1.3));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333333));
  doc->push_back(
      newStringField(L"value", L"4.2333333333333", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333332));
  doc->push_back(
      newStringField(L"value", L"4.2333333333332", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::DOUBLE, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(4, td->totalHits);
  // numeric order
  assertEquals(L"30.1", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2333333333333",
               searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4.2333333333332",
               searcher->doc(td->scoreDocs[2]->doc)[L"value"]);
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[3]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testDoubleMissing() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", -1.3));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333333));
  doc->push_back(
      newStringField(L"value", L"4.2333333333333", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333332));
  doc->push_back(
      newStringField(L"value", L"4.2333333333332", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value", SortField::Type::DOUBLE));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(4, td->totalHits);
  // null treated as a 0
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4.2333333333332",
               searcher->doc(td->scoreDocs[2]->doc)[L"value"]);
  assertEquals(L"4.2333333333333",
               searcher->doc(td->scoreDocs[3]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testDoubleMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", -1.3));
  doc->push_back(newStringField(L"value", L"-1.3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333333));
  doc->push_back(
      newStringField(L"value", L"4.2333333333333", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<DoubleDocValuesField>(L"value", 4.2333333333332));
  doc->push_back(
      newStringField(L"value", L"4.2333333333332", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"value", SortField::Type::DOUBLE);
  sortField->setMissingValue(numeric_limits<double>::max());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(4, td->totalHits);
  // null treated as Double.MAX_VALUE
  assertEquals(L"-1.3", searcher->doc(td->scoreDocs[0]->doc)[L"value"]);
  assertEquals(L"4.2333333333332",
               searcher->doc(td->scoreDocs[1]->doc)[L"value"]);
  assertEquals(L"4.2333333333333",
               searcher->doc(td->scoreDocs[2]->doc)[L"value"]);
  assertNull(searcher->doc(td->scoreDocs[3]->doc)[L"value"]);

  delete ir;
  delete dir;
}

void TestSort::testMultiSort() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value1", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<NumericDocValuesField>(L"value2", 0));
  doc->push_back(newStringField(L"value1", L"foo", Field::Store::YES));
  doc->push_back(newStringField(L"value2", L"0", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value1", make_shared<BytesRef>(L"bar")));
  doc->push_back(make_shared<NumericDocValuesField>(L"value2", 1));
  doc->push_back(newStringField(L"value1", L"bar", Field::Store::YES));
  doc->push_back(newStringField(L"value2", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value1", make_shared<BytesRef>(L"bar")));
  doc->push_back(make_shared<NumericDocValuesField>(L"value2", 0));
  doc->push_back(newStringField(L"value1", L"bar", Field::Store::YES));
  doc->push_back(newStringField(L"value2", L"0", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedDocValuesField>(
      L"value1", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<NumericDocValuesField>(L"value2", 1));
  doc->push_back(newStringField(L"value1", L"foo", Field::Store::YES));
  doc->push_back(newStringField(L"value2", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"value1", SortField::Type::STRING),
      make_shared<SortField>(L"value2", SortField::Type::LONG));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(4, td->totalHits);
  // 'bar' comes before 'foo'
  assertEquals(L"bar", searcher->doc(td->scoreDocs[0]->doc)[L"value1"]);
  assertEquals(L"bar", searcher->doc(td->scoreDocs[1]->doc)[L"value1"]);
  assertEquals(L"foo", searcher->doc(td->scoreDocs[2]->doc)[L"value1"]);
  assertEquals(L"foo", searcher->doc(td->scoreDocs[3]->doc)[L"value1"]);
  // 0 comes before 1
  assertEquals(L"0", searcher->doc(td->scoreDocs[0]->doc)[L"value2"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"value2"]);
  assertEquals(L"0", searcher->doc(td->scoreDocs[2]->doc)[L"value2"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[3]->doc)[L"value2"]);

  // Now with overflow
  td = searcher->search(make_shared<MatchAllDocsQuery>(), 1, sort);
  assertEquals(4, td->totalHits);
  assertEquals(L"bar", searcher->doc(td->scoreDocs[0]->doc)[L"value1"]);
  assertEquals(L"0", searcher->doc(td->scoreDocs[0]->doc)[L"value2"]);

  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search