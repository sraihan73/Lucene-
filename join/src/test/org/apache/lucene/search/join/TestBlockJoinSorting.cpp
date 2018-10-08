using namespace std;

#include "TestBlockJoinSorting.h"

namespace org::apache::lucene::search::join
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNestedSorting() throws Exception
void TestBlockJoinSorting::testNestedSorting() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  deque<std::shared_ptr<Document>> docs = deque<std::shared_ptr<Document>>();
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"a", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"a")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"b", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"b")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"c", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"c")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"a", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);
  w->commit();

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"c", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"c")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"d", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"d")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"e", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"e")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"b", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"e", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"e")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"f", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"f")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"g", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"g")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"c", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"g", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"g")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"h", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"h")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"i", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"i")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"d", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);
  w->commit();

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"i", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"i")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"j", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"j")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"k", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"k")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"f", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"k", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"k")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"l", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"l")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"m", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"m")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"g", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);

  docs.clear();
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"m", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"m")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"T", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"n", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"n")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"field2", L"o", Field::Store::NO));
  document->push_back(make_shared<SortedDocValuesField>(
      L"field2", make_shared<BytesRef>(L"o")));
  document->push_back(
      make_shared<StringField>(L"filter_1", L"F", Field::Store::NO));
  docs.push_back(document);
  document = make_shared<Document>();
  document->push_back(
      make_shared<StringField>(L"__type", L"parent", Field::Store::NO));
  document->push_back(
      make_shared<StringField>(L"field1", L"i", Field::Store::NO));
  docs.push_back(document);
  w->addDocuments(docs);
  w->commit();

  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(DirectoryReader::open(w->w));
  delete w;
  shared_ptr<BitSetProducer> parentFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"__type", L"parent")));
  CheckJoinIndex::check(searcher->getIndexReader(), parentFilter);
  shared_ptr<BitSetProducer> childFilter = make_shared<QueryBitSetProducer>(
      make_shared<PrefixQuery>(make_shared<Term>(L"field2")));
  shared_ptr<ToParentBlockJoinQuery> query =
      make_shared<ToParentBlockJoinQuery>(
          make_shared<PrefixQuery>(make_shared<Term>(L"field2")), parentFilter,
          ScoreMode::None);

  // Sort by field ascending, order first
  shared_ptr<ToParentBlockJoinSortField> sortField =
      make_shared<ToParentBlockJoinSortField>(
          L"field2", SortField::Type::STRING, false, parentFilter, childFilter);
  shared_ptr<Sort> sort = make_shared<Sort>(sortField);
  shared_ptr<TopFieldDocs> topDocs = searcher->search(query, 5, sort);
  assertEquals(7, topDocs->totalHits);
  assertEquals(5, topDocs->scoreDocs.size());
  assertEquals(3, topDocs->scoreDocs[0]->doc);
  assertEquals(L"a",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[0]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(7, topDocs->scoreDocs[1]->doc);
  assertEquals(L"c",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[1]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(11, topDocs->scoreDocs[2]->doc);
  assertEquals(L"e",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[2]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(15, topDocs->scoreDocs[3]->doc);
  assertEquals(L"g",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[3]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(19, topDocs->scoreDocs[4]->doc);
  assertEquals(L"i",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[4]))
                        ->fields[0]))
                   .utf8ToString());

  // Sort by field ascending, order last
  sortField = notEqual(sortField, [&]() {
    make_shared<ToParentBlockJoinSortField>(
        L"field2", org::apache::lucene::search::SortField::Type::STRING, false,
        true, parentFilter, childFilter);
  });

  sort = make_shared<Sort>(sortField);
  topDocs = searcher->search(query, 5, sort);
  assertEquals(7, topDocs->totalHits);
  assertEquals(5, topDocs->scoreDocs.size());
  assertEquals(3, topDocs->scoreDocs[0]->doc);
  assertEquals(L"c",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[0]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(7, topDocs->scoreDocs[1]->doc);
  assertEquals(L"e",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[1]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(11, topDocs->scoreDocs[2]->doc);
  assertEquals(L"g",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[2]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(15, topDocs->scoreDocs[3]->doc);
  assertEquals(L"i",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[3]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(19, topDocs->scoreDocs[4]->doc);
  assertEquals(L"k",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[4]))
                        ->fields[0]))
                   .utf8ToString());

  // Sort by field descending, order last
  sortField = notEqual(sortField, [&]() {
    make_shared<ToParentBlockJoinSortField>(
        L"field2", org::apache::lucene::search::SortField::Type::STRING, true,
        parentFilter, childFilter);
  });
  sort = make_shared<Sort>(sortField);
  topDocs = searcher->search(query, 5, sort);
  assertEquals(topDocs->totalHits, 7);
  assertEquals(5, topDocs->scoreDocs.size());
  assertEquals(27, topDocs->scoreDocs[0]->doc);
  assertEquals(L"o",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[0]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(23, topDocs->scoreDocs[1]->doc);
  assertEquals(L"m",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[1]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(19, topDocs->scoreDocs[2]->doc);
  assertEquals(L"k",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[2]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(15, topDocs->scoreDocs[3]->doc);
  assertEquals(L"i",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[3]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(11, topDocs->scoreDocs[4]->doc);
  assertEquals(L"g",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[4]))
                        ->fields[0]))
                   .utf8ToString());

  // Sort by field descending, order last, sort filter (filter_1:T)
  shared_ptr<BitSetProducer> childFilter1T = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>((make_shared<Term>(L"filter_1", L"T"))));
  query = make_shared<ToParentBlockJoinQuery>(
      make_shared<TermQuery>((make_shared<Term>(L"filter_1", L"T"))),
      parentFilter, ScoreMode::None);

  sortField = notEqual(sortField, [&]() {
    make_shared<ToParentBlockJoinSortField>(
        L"field2", org::apache::lucene::search::SortField::Type::STRING, true,
        parentFilter, childFilter1T);
  });

  sort = make_shared<Sort>(sortField);
  topDocs = searcher->search(query, 5, sort);
  assertEquals(6, topDocs->totalHits);
  assertEquals(5, topDocs->scoreDocs.size());
  assertEquals(23, topDocs->scoreDocs[0]->doc);
  assertEquals(L"m",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[0]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(27, topDocs->scoreDocs[1]->doc);
  assertEquals(L"m",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[1]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(11, topDocs->scoreDocs[2]->doc);
  assertEquals(L"g",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[2]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(15, topDocs->scoreDocs[3]->doc);
  assertEquals(L"g",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[3]))
                        ->fields[0]))
                   .utf8ToString());
  assertEquals(7, topDocs->scoreDocs[4]->doc);
  assertEquals(L"e",
               (any_cast<std::shared_ptr<BytesRef>>(
                    (std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[4]))
                        ->fields[0]))
                   .utf8ToString());

      sortField = notEqual(sortField, [&] ()
      {
      make_shared<ToParentBlockJoinSortField>(L"field2", org::apache::lucene::search::SortField::Type::STRING, true, make_shared<QueryBitSetProducer>(make_shared<org::apache::lucene::search::TermQuery>(make_shared<org::apache::lucene::index::Term>(L"__type", L"another")));
      };
         , childFilter1T));

      delete searcher->getIndexReader();
      delete dir;
}

shared_ptr<ToParentBlockJoinSortField>
TestBlockJoinSorting::notEqual(shared_ptr<ToParentBlockJoinSortField> old,
                               function<ToParentBlockJoinSortField *()> &create)
{
  shared_ptr<ToParentBlockJoinSortField> *const newObj = create();
  assertFalse(old->equals(newObj));
  assertNotSame(old, newObj);

  shared_ptr<ToParentBlockJoinSortField> *const bro = create();
  assertEquals(newObj, bro);
  assertEquals(newObj->hashCode(), bro->hashCode());
  assertNotSame(bro, newObj);

  assertFalse(old->equals(bro));
  return newObj;
}
} // namespace org::apache::lucene::search::join