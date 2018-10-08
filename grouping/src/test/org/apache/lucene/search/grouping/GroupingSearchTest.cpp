using namespace std;

#include "GroupingSearchTest.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

void GroupingSearchTest::testBasic() 
{

  const wstring groupField = L"author";

  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  bool canUseIDV = true;
  deque<std::shared_ptr<Document>> documents =
      deque<std::shared_ptr<Document>>();
  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", canUseIDV);
  doc->push_back(
      make_shared<TextField>(L"content", L"random text", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"1", customType));
  documents.push_back(doc);

  // 1
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", canUseIDV);
  doc->push_back(make_shared<TextField>(L"content", L"some more random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"2", customType));
  documents.push_back(doc);

  // 2
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", canUseIDV);
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random textual data", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"3", customType));
  doc->push_back(make_shared<StringField>(L"groupend", L"x", Field::Store::NO));
  documents.push_back(doc);
  w->addDocuments(documents);
  documents.clear();

  // 3
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author2", canUseIDV);
  doc->push_back(make_shared<TextField>(L"content", L"some random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"4", customType));
  doc->push_back(make_shared<StringField>(L"groupend", L"x", Field::Store::NO));
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3", canUseIDV);
  doc->push_back(make_shared<TextField>(L"content", L"some more random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"5", customType));
  documents.push_back(doc);

  // 5
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3", canUseIDV);
  doc->push_back(
      make_shared<TextField>(L"content", L"random", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"6", customType));
  doc->push_back(make_shared<StringField>(L"groupend", L"x", Field::Store::NO));
  documents.push_back(doc);
  w->addDocuments(documents);
  documents.clear();

  // 6 -- no author field
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"random word stuck in alot of other text",
      Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"6", customType));
  doc->push_back(make_shared<StringField>(L"groupend", L"x", Field::Store::NO));

  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(w->getReader());
  delete w;

  shared_ptr<Sort> groupSort = Sort::RELEVANCE;
  shared_ptr<GroupingSearch> groupingSearch =
      createRandomGroupingSearch(groupField, groupSort, 5, canUseIDV);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: TopGroups<?> groups = groupingSearch.search(indexSearcher,
  // new org.apache.lucene.search.TermQuery(new
  // org.apache.lucene.index.Term("content", "random")), 0, 10);
  shared_ptr < TopGroups <
      ? >> groups = groupingSearch->search(
            indexSearcher,
            make_shared<TermQuery>(make_shared<Term>(L"content", L"random")), 0,
            10);

  assertEquals(7, groups->totalHitCount);
  assertEquals(7, groups->totalGroupedHitCount);
  assertEquals(4, groups->groups.size());

  // relevance order: 5, 0, 3, 4, 1, 2, 6

  // the later a document is added the higher this docId
  // value
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: GroupDocs<?> group = groups.groups[0];
  shared_ptr < GroupDocs < ? >> group = groups->groups[0];
  compareGroupValue(L"author3", group);
  assertEquals(2, group->scoreDocs.size());
  assertEquals(5, group->scoreDocs[0]->doc);
  assertEquals(4, group->scoreDocs[1]->doc);
  assertTrue(group->scoreDocs[0]->score >= group->scoreDocs[1]->score);

  group = groups->groups[1];
  compareGroupValue(L"author1", group);
  assertEquals(3, group->scoreDocs.size());
  assertEquals(0, group->scoreDocs[0]->doc);
  assertEquals(1, group->scoreDocs[1]->doc);
  assertEquals(2, group->scoreDocs[2]->doc);
  assertTrue(group->scoreDocs[0]->score >= group->scoreDocs[1]->score);
  assertTrue(group->scoreDocs[1]->score >= group->scoreDocs[2]->score);

  group = groups->groups[2];
  compareGroupValue(L"author2", group);
  assertEquals(1, group->scoreDocs.size());
  assertEquals(3, group->scoreDocs[0]->doc);

  group = groups->groups[3];
  compareGroupValue(L"", group);
  assertEquals(1, group->scoreDocs.size());
  assertEquals(6, group->scoreDocs[0]->doc);

  shared_ptr<Query> lastDocInBlock =
      make_shared<TermQuery>(make_shared<Term>(L"groupend", L"x"));
  groupingSearch = make_shared<GroupingSearch>(lastDocInBlock);
  groups = groupingSearch->search(
      indexSearcher,
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")), 0, 10);

  assertEquals(7, groups->totalHitCount);
  assertEquals(7, groups->totalGroupedHitCount);
  assertEquals(4, groups->totalGroupCount.value());
  assertEquals(4, groups->groups.size());

  delete indexSearcher->getIndexReader();
  delete dir;
}

void GroupingSearchTest::addGroupField(shared_ptr<Document> doc,
                                       const wstring &groupField,
                                       const wstring &value, bool canUseIDV)
{
  doc->push_back(make_shared<TextField>(groupField, value, Field::Store::YES));
  if (canUseIDV) {
    doc->push_back(make_shared<SortedDocValuesField>(
        groupField, make_shared<BytesRef>(value)));
  }
}

template <typename T1>
void GroupingSearchTest::compareGroupValue(const wstring &expected,
                                           shared_ptr<GroupDocs<T1>> group)
{
  if (expected == L"") {
    if (group->groupValue == nullptr) {
      return;
    } else if (group->groupValue->getClass().isAssignableFrom(
                   MutableValueStr::typeid)) {
      return;
    } else if ((std::static_pointer_cast<BytesRef>(group->groupValue))
                   ->length == 0) {
      return;
    }
    fail();
  }

  if (group->groupValue->getClass().isAssignableFrom(BytesRef::typeid)) {
    assertEquals(make_shared<BytesRef>(expected), group->groupValue);
  } else if (group->groupValue->getClass().isAssignableFrom(
                 MutableValueStr::typeid)) {
    shared_ptr<MutableValueStr> v = make_shared<MutableValueStr>();
    v->value->copyChars(expected);
    assertEquals(v, group->groupValue);
  } else {
    fail();
  }
}

shared_ptr<GroupingSearch>
GroupingSearchTest::createRandomGroupingSearch(const wstring &groupField,
                                               shared_ptr<Sort> groupSort,
                                               int docsInGroup, bool canUseIDV)
{
  shared_ptr<GroupingSearch> groupingSearch;
  if (random()->nextBoolean()) {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    groupingSearch = make_shared<GroupingSearch>(vs, unordered_map<>());
  } else {
    groupingSearch = make_shared<GroupingSearch>(groupField);
  }

  groupingSearch->setGroupSort(groupSort);
  groupingSearch->setGroupDocsLimit(docsInGroup);

  if (random()->nextBoolean()) {
    groupingSearch->setCachingInMB(4.0, true);
  }

  return groupingSearch;
}

void GroupingSearchTest::testSetAllGroups() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"group", L"foo", StringField::TYPE_NOT_STORED));
  doc->push_back(make_shared<SortedDocValuesField>(
      L"group", make_shared<BytesRef>(L"foo")));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(w->getReader());
  delete w;

  shared_ptr<GroupingSearch> gs = make_shared<GroupingSearch>(L"group");
  gs->setAllGroups(true);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: TopGroups<?> groups = gs.search(indexSearcher, new
  // org.apache.lucene.search.TermQuery(new org.apache.lucene.index.Term("group",
  // "foo")), 0, 10);
  shared_ptr < TopGroups <
      ? >> groups = gs->search(
            indexSearcher,
            make_shared<TermQuery>(make_shared<Term>(L"group", L"foo")), 0, 10);
  assertEquals(1, groups->totalHitCount);
  // assertEquals(1, groups.totalGroupCount.intValue());
  assertEquals(1, groups->totalGroupedHitCount);
  assertEquals(1, gs->getAllMatchingGroups()->size());
  delete indexSearcher->getIndexReader();
  delete dir;
}
} // namespace org::apache::lucene::search::grouping