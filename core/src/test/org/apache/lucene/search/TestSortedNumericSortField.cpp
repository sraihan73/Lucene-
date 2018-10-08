using namespace std;

#include "TestSortedNumericSortField.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;

void TestSortedNumericSortField::testEmptyIndex() 
{
  shared_ptr<IndexSearcher> empty = newSearcher(make_shared<MultiReader>());
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"contents", L"foo"));

  shared_ptr<Sort> sort = make_shared<Sort>();
  sort->setSort(make_shared<SortedNumericSortField>(L"sortednumeric",
                                                    SortField::Type::LONG));
  shared_ptr<TopDocs> td = empty->search(query, 10, sort, true, true);
  assertEquals(0, td->totalHits);

  // for an empty index, any selector should work
  for (SortedNumericSelector::Type v : SortedNumericSelector::Type::values()) {
    sort->setSort(make_shared<SortedNumericSortField>(
        L"sortednumeric", SortField::Type::LONG, false, v));
    td = empty->search(query, 10, sort, true, true);
    assertEquals(0, td->totalHits);
  }
}

void TestSortedNumericSortField::testEquals() 
{
  shared_ptr<SortField> sf =
      make_shared<SortedNumericSortField>(L"a", SortField::Type::LONG);
  assertFalse(sf->equals(nullptr));

  assertEquals(sf, sf);

  shared_ptr<SortField> sf2 =
      make_shared<SortedNumericSortField>(L"a", SortField::Type::LONG);
  assertEquals(sf, sf2);
  assertEquals(sf->hashCode(), sf2->hashCode());

  assertFalse(sf->equals(
      make_shared<SortedNumericSortField>(L"a", SortField::Type::LONG, true)));
  assertFalse(sf->equals(
      make_shared<SortedNumericSortField>(L"a", SortField::Type::FLOAT)));
  assertFalse(sf->equals(
      make_shared<SortedNumericSortField>(L"b", SortField::Type::LONG)));
  assertFalse(sf->equals(make_shared<SortedNumericSortField>(
      L"a", SortField::Type::LONG, false, SortedNumericSelector::Type::MAX)));
  assertFalse(sf->equals(L"foo"));
}

void TestSortedNumericSortField::testForward() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 5));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 7));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"value", SortField::Type::INT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 3 comes before 5
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 7));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 5));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedNumericSortField>(
      L"value", SortField::Type::INT, true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testMissingFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 5));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 7));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"value", SortField::Type::INT);
  sortField->setMissingValue(numeric_limits<int>::min());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 3 comes before 5
  // null comes first
  assertEquals(L"3", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 5));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 3));
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 7));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortedNumericSortField>(L"value", SortField::Type::INT);
  sortField->setMissingValue(numeric_limits<int>::max());
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 3 comes before 5
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  // null comes last
  assertEquals(L"3", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testSingleton() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 5));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(L"value", 3));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"value", SortField::Type::INT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 3 comes before 5
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testFloat() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::floatToSortableInt(-3.0f)));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::floatToSortableInt(-5.0f)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::floatToSortableInt(7.0f)));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"value", SortField::Type::FLOAT));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // -5 comes before -3
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedNumericSortField::testDouble() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::doubleToSortableLong(-3)));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::doubleToSortableLong(-5)));
  doc->push_back(make_shared<SortedNumericDocValuesField>(
      L"value", NumericUtils::doubleToSortableLong(7)));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortedNumericSortField>(L"value", SortField::Type::DOUBLE));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // -5 comes before -3
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search