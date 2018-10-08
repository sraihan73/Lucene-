using namespace std;

#include "TestSortedSetSelector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

void TestSortedSetSelector::testMax() 
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

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);

  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MAX));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'baz' comes before 'foo'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMaxReverse() 
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

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);

  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", true, SortedSetSelector::Type::MAX));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'baz' comes before 'foo'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMaxMissingFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);

  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MAX);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null comes first
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  // 'baz' comes before 'foo'
  assertEquals(L"3", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMaxMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);

  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MAX);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 'baz' comes before 'foo'
  assertEquals(L"3", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  // null comes last
  assertEquals(L"1", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMaxSingleton() 
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

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MAX));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMin() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MIN));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMinReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", true, SortedSetSelector::Type::MIDDLE_MIN));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMinMissingFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MIN);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null comes first
  assertEquals(L"3", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  // 'b' comes before 'c'
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMinMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MIN);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  // null comes last
  assertEquals(L"3", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMinSingleton() 
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

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MIN));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'bar' comes before 'baz'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMax() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MAX));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMaxReverse() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", true, SortedSetSelector::Type::MIDDLE_MAX));

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(2, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"1", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMaxMissingFirst() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MAX);
  sortField->setMissingValue(SortField::STRING_FIRST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // null comes first
  assertEquals(L"3", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  // 'b' comes before 'c'
  assertEquals(L"2", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMaxMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"a")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"c")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"d")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"b")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<SortField> sortField = make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MAX);
  sortField->setMissingValue(SortField::STRING_LAST);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);

  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
  assertEquals(3, td->totalHits);
  // 'b' comes before 'c'
  assertEquals(L"2", searcher->doc(td->scoreDocs[0]->doc)[L"id"]);
  assertEquals(L"1", searcher->doc(td->scoreDocs[1]->doc)[L"id"]);
  // null comes last
  assertEquals(L"3", searcher->doc(td->scoreDocs[2]->doc)[L"id"]);

  delete ir;
  delete dir;
}

void TestSortedSetSelector::testMiddleMaxSingleton() 
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

  // slow wrapper does not support random access ordinals (there is no need for
  // that!)
  shared_ptr<IndexSearcher> searcher = newSearcher(ir, false);
  shared_ptr<Sort> sort = make_shared<Sort>(make_shared<SortedSetSortField>(
      L"value", false, SortedSetSelector::Type::MIDDLE_MAX));

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