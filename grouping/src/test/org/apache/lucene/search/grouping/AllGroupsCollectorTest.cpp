using namespace std;

#include "AllGroupsCollectorTest.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void AllGroupsCollectorTest::testTotalGroupCount() 
{

  const wstring groupField = L"author";
  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1");
  doc->push_back(
      make_shared<TextField>(L"content", L"random text", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"1", customType));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1");
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random text blob", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"2", customType));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1");
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random textual data", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"3", customType));
  w->addDocument(doc);
  w->commit(); // To ensure a second segment

  // 3
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author2");
  doc->push_back(make_shared<TextField>(L"content", L"some random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"4", customType));
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3");
  doc->push_back(make_shared<TextField>(L"content", L"some more random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"5", customType));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3");
  doc->push_back(
      make_shared<TextField>(L"content", L"random blob", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"6", customType));
  w->addDocument(doc);

  // 6 -- no author field
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"random word stuck in alot of other text",
      Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"6", customType));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(w->getReader());
  delete w;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: AllGroupsCollector<?> allGroupsCollector =
  // createRandomCollector(groupField);
  shared_ptr < AllGroupsCollector < ? >> allGroupsCollector =
                                          createRandomCollector(groupField);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      allGroupsCollector);
  assertEquals(4, allGroupsCollector->getGroupCount());

  allGroupsCollector = createRandomCollector(groupField);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"some")),
      allGroupsCollector);
  assertEquals(3, allGroupsCollector->getGroupCount());

  allGroupsCollector = createRandomCollector(groupField);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"blob")),
      allGroupsCollector);
  assertEquals(2, allGroupsCollector->getGroupCount());

  delete indexSearcher->getIndexReader();
  delete dir;
}

void AllGroupsCollectorTest::addGroupField(shared_ptr<Document> doc,
                                           const wstring &groupField,
                                           const wstring &value)
{
  doc->push_back(make_shared<TextField>(groupField, value, Field::Store::YES));
  doc->push_back(make_shared<SortedDocValuesField>(
      groupField, make_shared<BytesRef>(value)));
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private AllGroupsCollector<?> createRandomCollector(std::wstring
// groupField)
shared_ptr < AllGroupsCollector <
    ? >> AllGroupsCollectorTest::createRandomCollector(
             const wstring &groupField)
{
  if (random()->nextBoolean()) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new AllGroupsCollector<>(new
    // TermGroupSelector(groupField));
    return make_shared < AllGroupsCollector <
        ? >> (make_shared<TermGroupSelector>(groupField));
  } else {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new AllGroupsCollector<>(new
    // ValueSourceGroupSelector(vs, new java.util.HashMap<>()));
    return make_shared < AllGroupsCollector <
        ? >> (make_shared<ValueSourceGroupSelector>(vs, unordered_map<>()));
  }
}
} // namespace org::apache::lucene::search::grouping