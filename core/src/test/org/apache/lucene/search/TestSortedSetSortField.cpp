using namespace std;

#include "TestSortedSetSortField.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSortedSetSortField::testEmptyIndex() 
{
  shared_ptr<IndexSearcher> empty = newSearcher(make_shared<MultiReader>());
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"contents", L"foo"));

  shared_ptr<Sort> sort = make_shared<Sort>();
  sort->setSort(make_shared<SortedSetSortField>(L"sortedset", false));
  shared_ptr<TopDocs> td = empty->search(query, 10, sort, true, true);
  assertEquals(0, td->totalHits);

  // for an empty index, any selector should work
  for (SortedSetSelector::Type v : SortedSetSelector::Type::values()) {
    sort->setSort(make_shared<SortedSetSortField>(L"sortedset", false, v));
    td = empty->search(query, 10, sort, true, true);
    assertEquals(0, td->totalHits);
  }
}

void TestSortedSetSortField::testEquals() 
{
  shared_ptr<SortField> sf = make_shared<SortedSetSortField>(L"a", false);
  assertFalse(sf->equals(nullptr));

  assertEquals(sf, sf);

  shared_ptr<SortField> sf2 = make_shared<SortedSetSortField>(L"a", false);
  assertEquals(sf, sf2);
  assertEquals(sf->hashCode(), sf2->hashCode());

  assertFalse(sf->equals(make_shared<SortedSetSortField>(L"a", true)));
  assertFalse(sf->equals(make_shared<SortedSetSortField>(L"b", false)));
  assertFalse(sf->equals(make_shared<SortedSetSortField>(
      L"a", false, SortedSetSelector::Type::MAX)));
  assertFalse(sf->equals(L"foo"));
}

void TestSortedSetSortField::testForward() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort =
      make_shared<Sort>(make_shared<SortedSetSortField>(L"value", false));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSortField::testReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort =
      make_shared<Sort>(make_shared<SortedSetSortField>(L"value", true));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSortField::testMissingFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortedSetSortField>(L"value", false);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 'bar' comes before 'baz'
  // null comes first
  assertEquals(L"3", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSortField::testMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<SortField> sortField =
      make_shared<SortedSetSortField>(L"value", false);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  // null comes last
  assertEquals(L"3", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSortField::testSingleton() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<Sort> sort =
      make_shared<Sort>(make_shared<SortedSetSortField>(L"value", false));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search