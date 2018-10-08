using namespace std;

#include "TestGrouping.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using CachingCollector = org::apache::lucene::search::CachingCollector;
using Collector = org::apache::lucene::search::Collector;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiCollector = org::apache::lucene::search::MultiCollector;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using Weight = org::apache::lucene::search::Weight;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

void TestGrouping::testBasic() 
{

  wstring groupField = L"author";

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
  doc->push_back(make_shared<TextField>(L"content", L"some more random text",
                                        Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"2", customType));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1");
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random textual data", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"id", L"3", customType));
  w->addDocument(doc);

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
      make_shared<TextField>(L"content", L"random", Field::Store::YES));
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

  shared_ptr<Sort> *const groupSort = Sort::RELEVANCE;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final FirstPassGroupingCollector<?> c1 =
  // createRandomFirstPassCollector(groupField, groupSort, 10);
  shared_ptr < FirstPassGroupingCollector <
      ? >> *const c1 =
            createRandomFirstPassCollector(groupField, groupSort, 10);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")), c1);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final TopGroupsCollector<?> c2 =
  // createSecondPassCollector(c1, groupSort,
  // org.apache.lucene.search.Sort.RELEVANCE, 0, 5, true, true, true);
  shared_ptr < TopGroupsCollector <
      ? >> *const c2 = createSecondPassCollector(c1, groupSort, Sort::RELEVANCE,
                                                 0, 5, true, true, true);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")), c2);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final TopGroups<?> groups = c2.getTopGroups(0);
  shared_ptr < TopGroups < ? >> *const groups = c2->getTopGroups(0);
  assertFalse(isnan(groups->maxScore));

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
  assertTrue(group->scoreDocs[0]->score > group->scoreDocs[1]->score);

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

  delete indexSearcher->getIndexReader();
  delete dir;
}

void TestGrouping::addGroupField(shared_ptr<Document> doc,
                                 const wstring &groupField,
                                 const wstring &value)
{
  doc->push_back(make_shared<SortedDocValuesField>(
      groupField, make_shared<BytesRef>(value)));
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private FirstPassGroupingCollector<?>
// createRandomFirstPassCollector(std::wstring groupField,
// org.apache.lucene.search.Sort groupSort, int topDocs) throws
// java.io.IOException
shared_ptr < FirstPassGroupingCollector <
    ? >> TestGrouping::createRandomFirstPassCollector(
             const wstring &groupField, shared_ptr<Sort> groupSort,
             int topDocs) 
{
  if (random()->nextBoolean()) {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new FirstPassGroupingCollector<>(new
    // ValueSourceGroupSelector(vs, new java.util.HashMap<>()), groupSort,
    // topDocs);
    return make_shared < FirstPassGroupingCollector <
        ? >> (make_shared<ValueSourceGroupSelector>(vs, unordered_map<>()),
              groupSort, topDocs);
  } else {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new FirstPassGroupingCollector<>(new
    // TermGroupSelector(groupField), groupSort, topDocs);
    return make_shared < FirstPassGroupingCollector <
        ? >> (make_shared<TermGroupSelector>(groupField), groupSort, topDocs);
  }
}

template <typename T1>
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private FirstPassGroupingCollector<?>
    // createFirstPassCollector(std::wstring groupField, org.apache.lucene.search.Sort
    // groupSort, int topDocs, FirstPassGroupingCollector<?>
    // firstPassGroupingCollector) throws java.io.IOException
    shared_ptr < FirstPassGroupingCollector <
    ? >> TestGrouping::createFirstPassCollector(
             const wstring &groupField, shared_ptr<Sort> groupSort, int topDocs,
             shared_ptr<FirstPassGroupingCollector<T1>>
                 firstPassGroupingCollector) 
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: GroupSelector<?> selector =
  // firstPassGroupingCollector.getGroupSelector();
  shared_ptr < GroupSelector <
      ? >> selector = firstPassGroupingCollector->getGroupSelector();
  if (TermGroupSelector::typeid->isAssignableFrom(selector->getClass())) {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new FirstPassGroupingCollector<>(new
    // ValueSourceGroupSelector(vs, new java.util.HashMap<>()), groupSort,
    // topDocs);
    return make_shared < FirstPassGroupingCollector <
        ? >> (make_shared<ValueSourceGroupSelector>(vs, unordered_map<>()),
              groupSort, topDocs);
  } else {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new FirstPassGroupingCollector<>(new
    // TermGroupSelector(groupField), groupSort, topDocs);
    return make_shared < FirstPassGroupingCollector <
        ? >> (make_shared<TermGroupSelector>(groupField), groupSort, topDocs);
  }
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T>
// TopGroupsCollector<T> createSecondPassCollector(FirstPassGroupingCollector
// firstPassGroupingCollector, org.apache.lucene.search.Sort groupSort,
// org.apache.lucene.search.Sort sortWithinGroup, int groupOffset, int
// maxDocsPerGroup, bool getScores, bool getMaxScores, bool
// fillSortFields) throws java.io.IOException
shared_ptr<TopGroupsCollector<T>> TestGrouping::createSecondPassCollector(
    shared_ptr<FirstPassGroupingCollector> firstPassGroupingCollector,
    shared_ptr<Sort> groupSort, shared_ptr<Sort> sortWithinGroup,
    int groupOffset, int maxDocsPerGroup, bool getScores, bool getMaxScores,
    bool fillSortFields) 
{

  shared_ptr<deque<SearchGroup<T>>> searchGroups =
      firstPassGroupingCollector->getTopGroups(groupOffset, fillSortFields);
  return make_shared<TopGroupsCollector<T>>(
      firstPassGroupingCollector->getGroupSelector(), searchGroups, groupSort,
      sortWithinGroup, maxDocsPerGroup, getScores, getMaxScores,
      fillSortFields);
}

template <typename T1>
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") private
    // TopGroupsCollector<?>
    // createSecondPassCollector(FirstPassGroupingCollector<?>
    // firstPassGroupingCollector, std::wstring groupField,
    // java.util.deque<SearchGroup<org.apache.lucene.util.BytesRef>>
    // searchGroups, org.apache.lucene.search.Sort groupSort,
    // org.apache.lucene.search.Sort sortWithinGroup, int maxDocsPerGroup,
    // bool getScores, bool getMaxScores, bool fillSortFields) throws
    // java.io.IOException C++ TODO: Java wildcard generics are not converted to
    // C++:
    shared_ptr < TopGroupsCollector <
    ? >> TestGrouping::createSecondPassCollector(
             shared_ptr<FirstPassGroupingCollector<T1>>
                 firstPassGroupingCollector,
             const wstring &groupField,
             shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>>
                 searchGroups,
             shared_ptr<Sort> groupSort, shared_ptr<Sort> sortWithinGroup,
             int maxDocsPerGroup, bool getScores, bool getMaxScores,
             bool fillSortFields) 
{
  if (firstPassGroupingCollector->getGroupSelector()
          ->getClass()
          .isAssignableFrom(TermGroupSelector::typeid)) {
    shared_ptr<GroupSelector<std::shared_ptr<BytesRef>>> selector =
        std::static_pointer_cast<GroupSelector<std::shared_ptr<BytesRef>>>(
            firstPassGroupingCollector->getGroupSelector());
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new TopGroupsCollector<>(selector, searchGroups,
    // groupSort, sortWithinGroup, maxDocsPerGroup, getScores, getMaxScores,
    // fillSortFields);
    return make_shared < TopGroupsCollector <
        ? >> (selector, searchGroups, groupSort, sortWithinGroup,
              maxDocsPerGroup, getScores, getMaxScores, fillSortFields);
  } else {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    deque<SearchGroup<std::shared_ptr<MutableValue>>> mvalSearchGroups =
        deque<SearchGroup<std::shared_ptr<MutableValue>>>(
            searchGroups->size());
    for (auto mergedTopGroup : searchGroups) {
      shared_ptr<SearchGroup<std::shared_ptr<MutableValue>>> sg =
          make_shared<SearchGroup<std::shared_ptr<MutableValue>>>();
      shared_ptr<MutableValueStr> groupValue = make_shared<MutableValueStr>();
      if (mergedTopGroup->groupValue != nullptr) {
        groupValue->value->copyBytes(mergedTopGroup->groupValue);
      } else {
        groupValue->exists_ = false;
      }
      sg->groupValue = groupValue;
      sg->sortValues = mergedTopGroup->sortValues;
      mvalSearchGroups.push_back(sg);
    }
    shared_ptr<ValueSourceGroupSelector> selector =
        make_shared<ValueSourceGroupSelector>(vs, unordered_map<>());
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return new TopGroupsCollector<>(selector,
    // mvalSearchGroups, groupSort, sortWithinGroup, maxDocsPerGroup, getScores,
    // getMaxScores, fillSortFields);
    return make_shared < TopGroupsCollector <
        ? >> (selector, mvalSearchGroups, groupSort, sortWithinGroup,
              maxDocsPerGroup, getScores, getMaxScores, fillSortFields);
  }
}

template <typename T1>
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private AllGroupsCollector<?>
    // createAllGroupsCollector(FirstPassGroupingCollector<?>
    // firstPassGroupingCollector, std::wstring groupField)
    shared_ptr < AllGroupsCollector <
    ? >> TestGrouping::createAllGroupsCollector(
             shared_ptr<FirstPassGroupingCollector<T1>>
                 firstPassGroupingCollector,
             const wstring &groupField)
{
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: return new
  // AllGroupsCollector<>(firstPassGroupingCollector.getGroupSelector());
  return make_shared < AllGroupsCollector <
      ? >> (firstPassGroupingCollector->getGroupSelector());
}

template <typename T1>
void TestGrouping::compareGroupValue(const wstring &expected,
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

template <typename T1>
shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>>
TestGrouping::getSearchGroups(shared_ptr<FirstPassGroupingCollector<T1>> c,
                              int groupOffset,
                              bool fillFields) 
{
  if (TermGroupSelector::typeid->isAssignableFrom(
          c->getGroupSelector()->getClass())) {
    shared_ptr<FirstPassGroupingCollector<std::shared_ptr<BytesRef>>>
        collector = std::static_pointer_cast<
            FirstPassGroupingCollector<std::shared_ptr<BytesRef>>>(c);
    return collector->getTopGroups(groupOffset, fillFields);
  } else if (ValueSourceGroupSelector::typeid->isAssignableFrom(
                 c->getGroupSelector()->getClass())) {
    shared_ptr<FirstPassGroupingCollector<std::shared_ptr<MutableValue>>>
        collector = std::static_pointer_cast<
            FirstPassGroupingCollector<std::shared_ptr<MutableValue>>>(c);
    shared_ptr<deque<SearchGroup<std::shared_ptr<MutableValue>>>>
        mutableValueGroups = collector->getTopGroups(groupOffset, fillFields);
    if (mutableValueGroups == nullptr) {
      return nullptr;
    }

    deque<SearchGroup<std::shared_ptr<BytesRef>>> groups =
        deque<SearchGroup<std::shared_ptr<BytesRef>>>(
            mutableValueGroups->size());
    for (auto mutableValueGroup : mutableValueGroups) {
      shared_ptr<SearchGroup<std::shared_ptr<BytesRef>>> sg =
          make_shared<SearchGroup<std::shared_ptr<BytesRef>>>();
      sg->groupValue = mutableValueGroup->groupValue->exists()
                           ? (std::static_pointer_cast<MutableValueStr>(
                                  mutableValueGroup->groupValue))
                                 ->value->get()
                           : nullptr;
      sg->sortValues = mutableValueGroup->sortValues;
      groups.push_back(sg);
    }
    return groups;
  }
  fail();
  return nullptr;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) private
// TopGroups<org.apache.lucene.util.BytesRef> getTopGroups(TopGroupsCollector c,
// int withinGroupOffset)
shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
TestGrouping::getTopGroups(shared_ptr<TopGroupsCollector> c,
                           int withinGroupOffset)
{
  if (c->getGroupSelector()->getClass().isAssignableFrom(
          TermGroupSelector::typeid)) {
    shared_ptr<TopGroupsCollector<std::shared_ptr<BytesRef>>> collector =
        std::static_pointer_cast<TopGroupsCollector<std::shared_ptr<BytesRef>>>(
            c);
    return collector->getTopGroups(withinGroupOffset);
  } else if (c->getGroupSelector()->getClass().isAssignableFrom(
                 ValueSourceGroupSelector::typeid)) {
    shared_ptr<TopGroupsCollector<std::shared_ptr<MutableValue>>> collector =
        std::static_pointer_cast<
            TopGroupsCollector<std::shared_ptr<MutableValue>>>(c);
    shared_ptr<TopGroups<std::shared_ptr<MutableValue>>> mvalTopGroups =
        collector->getTopGroups(withinGroupOffset);
    deque<GroupDocs<std::shared_ptr<BytesRef>>> groups =
        deque<GroupDocs<std::shared_ptr<BytesRef>>>(
            mvalTopGroups->groups.size());
    for (auto mvalGd : mvalTopGroups->groups) {
      shared_ptr<BytesRef> groupValue =
          mvalGd->groupValue->exists()
              ? (std::static_pointer_cast<MutableValueStr>(mvalGd->groupValue))
                    ->value->get()
              : nullptr;
      groups.push_back(make_shared<GroupDocs<>>(
          NAN, mvalGd->maxScore, mvalGd->totalHits, mvalGd->scoreDocs,
          groupValue, mvalGd->groupSortValues));
    }
    // NOTE: currenlty using diamond operator on MergedIterator (without
    // explicit Term class) causes errors on Eclipse Compiler (ecj) used for
    // javadoc lint
    return make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
        mvalTopGroups->groupSort, mvalTopGroups->withinGroupSort,
        mvalTopGroups->totalHitCount, mvalTopGroups->totalGroupedHitCount,
        groups.toArray(std::deque<std::shared_ptr<GroupDocs>>(groups.size())),
        NAN);
  }
  fail();
  return nullptr;
}

TestGrouping::GroupDoc::GroupDoc(int id, shared_ptr<BytesRef> group,
                                 shared_ptr<BytesRef> sort1,
                                 shared_ptr<BytesRef> sort2,
                                 const wstring &content)
    : id(id), group(group), sort1(sort1), sort2(sort2), content(content)
{
}

shared_ptr<Sort> TestGrouping::getRandomSort()
{
  const deque<std::shared_ptr<SortField>> sortFields =
      deque<std::shared_ptr<SortField>>();
  if (random()->nextInt(7) == 2) {
    sortFields.push_back(SortField::FIELD_SCORE);
  } else {
    if (random()->nextBoolean()) {
      if (random()->nextBoolean()) {
        sortFields.push_back(make_shared<SortField>(
            L"sort1", SortField::Type::STRING, random()->nextBoolean()));
      } else {
        sortFields.push_back(make_shared<SortField>(
            L"sort2", SortField::Type::STRING, random()->nextBoolean()));
      }
    } else if (random()->nextBoolean()) {
      sortFields.push_back(make_shared<SortField>(
          L"sort1", SortField::Type::STRING, random()->nextBoolean()));
      sortFields.push_back(make_shared<SortField>(
          L"sort2", SortField::Type::STRING, random()->nextBoolean()));
    }
  }
  // Break ties:
  sortFields.push_back(make_shared<SortField>(L"id", SortField::Type::INT));
  return make_shared<Sort>(sortFields.toArray(
      std::deque<std::shared_ptr<SortField>>(sortFields.size())));
}

shared_ptr<Comparator<std::shared_ptr<GroupDoc>>>
TestGrouping::getComparator(shared_ptr<Sort> sort)
{
  std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
  return make_shared<ComparatorAnonymousInnerClass>(shared_from_this(),
                                                    sortFields);
}

TestGrouping::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestGrouping> outerInstance,
    deque<std::shared_ptr<SortField>> &sortFields)
{
  this->outerInstance = outerInstance;
  this->sortFields = sortFields;
}

int TestGrouping::ComparatorAnonymousInnerClass::compare(
    shared_ptr<GroupDoc> d1, shared_ptr<GroupDoc> d2)
{
  for (auto sf : sortFields) {
    constexpr int cmp;
    if (sf->getType() == SortField::Type::SCORE) {
      if (d1->score > d2->score) {
        cmp = -1;
      } else if (d1->score < d2->score) {
        cmp = 1;
      } else {
        cmp = 0;
      }
    } else if (sf->getField() == L"sort1") {
      cmp = d1->sort1->compareTo(d2->sort1);
    } else if (sf->getField() == L"sort2") {
      cmp = d1->sort2->compareTo(d2->sort2);
    } else {
      TestUtil::assertEquals(sf->getField(), L"id");
      cmp = d1->id - d2->id;
    }
    if (cmp != 0) {
      return sf->getReverse() ? -cmp : cmp;
    }
  }
  // Our sort always fully tie breaks:
  fail();
  return 0;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private
// Comparable<?>[] fillFields(GroupDoc d, org.apache.lucene.search.Sort sort) C++
// TODO: Java wildcard generics are not converted to C++:
std::deque < Comparable <
    ? >> TestGrouping::fillFields(shared_ptr<GroupDoc> d, shared_ptr<Sort> sort)
{
  std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final Comparable<?>[] fields = new
  // Comparable[sortFields.length];
  const std::deque < Comparable <
      ? >> fields = std::deque<Comparable>(sortFields.size());
  for (int fieldIDX = 0; fieldIDX < sortFields.size(); fieldIDX++) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final Comparable<?> c;
    const Comparable < ? > c;
    shared_ptr<SortField> *const sf = sortFields[fieldIDX];
    if (sf->getType() == SortField::Type::SCORE) {
      c = d->score;
    } else if (sf->getField() == L"sort1") {
      c = d->sort1;
    } else if (sf->getField() == L"sort2") {
      c = d->sort2;
    } else {
      assertEquals(L"id", sf->getField());
      c = d->id;
    }
    fields[fieldIDX] = c;
  }
  return fields;
}

wstring TestGrouping::groupToString(shared_ptr<BytesRef> b)
{
  if (b == nullptr) {
    return L"null";
  } else {
    return b->utf8ToString();
  }
}

shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
TestGrouping::slowGrouping(std::deque<std::shared_ptr<GroupDoc>> &groupDocs,
                           const wstring &searchTerm, bool fillFields,
                           bool getScores, bool getMaxScores, bool doAllGroups,
                           shared_ptr<Sort> groupSort, shared_ptr<Sort> docSort,
                           int topNGroups, int docsPerGroup, int groupOffset,
                           int docOffset)
{

  shared_ptr<Comparator<std::shared_ptr<GroupDoc>>> *const groupSortComp =
      getComparator(groupSort);

  Arrays::sort(groupDocs, groupSortComp);
  const unordered_map<std::shared_ptr<BytesRef>,
                      deque<std::shared_ptr<GroupDoc>>>
      groups = unordered_map<std::shared_ptr<BytesRef>,
                             deque<std::shared_ptr<GroupDoc>>>();
  const deque<std::shared_ptr<BytesRef>> sortedGroups =
      deque<std::shared_ptr<BytesRef>>();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final java.util.List<Comparable<?>[]> sortedGroupFields =
  // new java.util.ArrayList<>();
  const deque < Comparable < ? > [] > sortedGroupFields =
                                    std::deque < deque < Comparable <
                                ? > [] >> () > ();

  int totalHitCount = 0;
  shared_ptr<Set<std::shared_ptr<BytesRef>>> knownGroups =
      unordered_set<std::shared_ptr<BytesRef>>();

  // System.out.println("TEST: slowGrouping");
  for (auto d : groupDocs) {
    // TODO: would be better to filter by searchTerm before sorting!
    if (!StringHelper::startsWith(d->content, searchTerm)) {
      continue;
    }
    totalHitCount++;
    // System.out.println("  match id=" + d.id + " score=" + d.score);

    if (doAllGroups) {
      if (!knownGroups->contains(d->group)) {
        knownGroups->add(d->group);
        // System.out.println("    add group=" + groupToString(d.group));
      }
    }

    deque<std::shared_ptr<GroupDoc>> l = groups[d->group];
    if (l.empty()) {
      // System.out.println("    add sortedGroup=" + groupToString(d.group));
      sortedGroups.push_back(d->group);
      if (fillFields) {
        sortedGroupFields.push_back(this->fillFields(d, groupSort));
      }
      l = deque<>();
      groups.emplace(d->group, l);
    }
    l.push_back(d);
  }

  if (groupOffset >= sortedGroups.size()) {
    // slice is out of bounds
    return nullptr;
  }

  constexpr int limit = min(groupOffset + topNGroups, groups.size());

  shared_ptr<Comparator<std::shared_ptr<GroupDoc>>> *const docSortComp =
      getComparator(docSort);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
  // GroupDocs<org.apache.lucene.util.BytesRef>[] result = new
  // GroupDocs[limit-groupOffset];
  std::deque<GroupDocs<std::shared_ptr<BytesRef>>> result =
      std::deque<std::shared_ptr<GroupDocs>>(limit - groupOffset);
  int totalGroupedHitCount = 0;
  for (int idx = groupOffset; idx < limit; idx++) {
    shared_ptr<BytesRef> *const group = sortedGroups[idx];
    const deque<std::shared_ptr<GroupDoc>> docs = groups[group];
    totalGroupedHitCount += docs.size();
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(docs, docSortComp);
    sort(docs.begin(), docs.end(), docSortComp);
    std::deque<std::shared_ptr<ScoreDoc>> hits;
    if (docs.size() > docOffset) {
      constexpr int docIDXLimit = min(docOffset + docsPerGroup, docs.size());
      hits = std::deque<std::shared_ptr<ScoreDoc>>(docIDXLimit - docOffset);
      for (int docIDX = docOffset; docIDX < docIDXLimit; docIDX++) {
        shared_ptr<GroupDoc> *const d = docs[docIDX];
        shared_ptr<FieldDoc> *const fd;
        if (fillFields) {
          fd = make_shared<FieldDoc>(d->id, getScores ? d->score : NAN,
                                     this->fillFields(d, docSort));
        } else {
          fd = make_shared<FieldDoc>(d->id, getScores ? d->score : NAN);
        }
        hits[docIDX - docOffset] = fd;
      }
    } else {
      hits = std::deque<std::shared_ptr<ScoreDoc>>(0);
    }

    result[idx - groupOffset] =
        make_shared<GroupDocs<std::shared_ptr<BytesRef>>>(
            NAN, 0.0f, docs.size(), hits, group,
            fillFields ? sortedGroupFields[idx] : nullptr);
  }

  if (doAllGroups) {
    return make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
        make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
            groupSort->getSort(), docSort->getSort(), totalHitCount,
            totalGroupedHitCount, result, NAN),
        knownGroups->size());
  } else {
    return make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
        groupSort->getSort(), docSort->getSort(), totalHitCount,
        totalGroupedHitCount, result, NAN);
  }
}

shared_ptr<DirectoryReader> TestGrouping::getDocBlockReader(
    shared_ptr<Directory> dir,
    std::deque<std::shared_ptr<GroupDoc>> &groupDocs) 
{
  // Coalesce by group, but in random order:
  Collections::shuffle(Arrays::asList(groupDocs), random());
  const unordered_map<std::shared_ptr<BytesRef>,
                      deque<std::shared_ptr<GroupDoc>>>
      groupMap = unordered_map<std::shared_ptr<BytesRef>,
                               deque<std::shared_ptr<GroupDoc>>>();
  const deque<std::shared_ptr<BytesRef>> groupValues =
      deque<std::shared_ptr<BytesRef>>();

  for (auto groupDoc : groupDocs) {
    if (groupMap.find(groupDoc->group) == groupMap.end()) {
      groupValues.push_back(groupDoc->group);
      groupMap.emplace(groupDoc->group, deque<std::shared_ptr<GroupDoc>>());
    }
    groupMap[groupDoc->group].push_back(groupDoc);
  }

  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  const deque<deque<std::shared_ptr<Document>>> updateDocs =
      deque<deque<std::shared_ptr<Document>>>();

  shared_ptr<FieldType> groupEndType =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  groupEndType->setIndexOptions(IndexOptions::DOCS);
  groupEndType->setOmitNorms(true);

  // System.out.println("TEST: index groups");
  for (auto group : groupValues) {
    const deque<std::shared_ptr<Document>> docs =
        deque<std::shared_ptr<Document>>();
    // System.out.println("TEST:   group=" + (group == null ? "null" :
    // group.utf8ToString()));
    for (auto groupValue : groupMap[group]) {
      shared_ptr<Document> doc = make_shared<Document>();
      docs.push_back(doc);
      if (groupValue->group != nullptr) {
        doc->push_back(newStringField(
            L"group", groupValue->group->utf8ToString(), Field::Store::YES));
        doc->push_back(make_shared<SortedDocValuesField>(
            L"group", BytesRef::deepCopyOf(groupValue->group)));
      }
      doc->push_back(newStringField(L"sort1", groupValue->sort1->utf8ToString(),
                                    Field::Store::NO));
      doc->push_back(make_shared<SortedDocValuesField>(
          L"sort1", BytesRef::deepCopyOf(groupValue->sort1)));
      doc->push_back(newStringField(L"sort2", groupValue->sort2->utf8ToString(),
                                    Field::Store::NO));
      doc->push_back(make_shared<SortedDocValuesField>(
          L"sort2", BytesRef::deepCopyOf(groupValue->sort2)));
      doc->push_back(make_shared<NumericDocValuesField>(L"id", groupValue->id));
      doc->push_back(
          newTextField(L"content", groupValue->content, Field::Store::NO));
      // System.out.println("TEST:     doc content=" + groupValue.content + "
      // group=" + (groupValue.group == null ? "null" :
      // groupValue.group.utf8ToString()) + " sort1=" +
      // groupValue.sort1.utf8ToString() + " id=" + groupValue.id);
    }
    // So we can pull filter marking last doc in block:
    shared_ptr<Field> *const groupEnd =
        newField(L"groupend", L"x", groupEndType);
    docs[docs.size() - 1]->push_back(groupEnd);
    // Add as a doc block:
    w->addDocuments(docs);
    if (group != nullptr && random()->nextInt(7) == 4) {
      updateDocs.push_back(docs);
    }
  }

  for (auto docs : updateDocs) {
    // Just replaces docs w/ same docs:
    w->updateDocuments(make_shared<Term>(L"group", docs[0][L"group"]), docs);
  }

  shared_ptr<DirectoryReader> *const r = w->getReader();
  delete w;

  return r;
}

TestGrouping::ShardState::ShardState(shared_ptr<IndexSearcher> s)
    : subSearchers(std::deque<std::shared_ptr<ShardSearcher>>(leaves->size())),
      docStarts(std::deque<int>(subSearchers.size()))
{
  shared_ptr<IndexReaderContext> *const ctx = s->getTopReaderContext();
  const deque<std::shared_ptr<LeafReaderContext>> leaves = ctx->leaves();
  for (int searcherIDX = 0; searcherIDX < subSearchers.size(); searcherIDX++) {
    subSearchers[searcherIDX] =
        make_shared<ShardSearcher>(leaves[searcherIDX], ctx);
  }

  for (int subIDX = 0; subIDX < docStarts.size(); subIDX++) {
    docStarts[subIDX] = leaves[subIDX]->docBase;
    // System.out.println("docStarts[" + subIDX + "]=" + docStarts[subIDX]);
  }
}

void TestGrouping::testRandom() 
{
  int numberOfRuns = TestUtil::nextInt(random(), 3, 6);
  for (int iter = 0; iter < numberOfRuns; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }

    constexpr int numDocs =
        TestUtil::nextInt(random(), 100, 1000) * RANDOM_MULTIPLIER;
    // final int numDocs = _TestUtil.nextInt(random, 5, 20);

    constexpr int numGroups = TestUtil::nextInt(random(), 1, numDocs);

    if (VERBOSE) {
      wcout << L"TEST: numDocs=" << numDocs << L" numGroups=" << numGroups
            << endl;
    }

    const deque<std::shared_ptr<BytesRef>> groups =
        deque<std::shared_ptr<BytesRef>>();
    for (int i = 0; i < numGroups; i++) {
      wstring randomValue;
      do {
        // B/c of DV based impl we can't see the difference between an empty
        // string and a null value. For that reason we don't generate empty
        // string groups.
        randomValue = TestUtil::randomRealisticUnicodeString(random());
        // randomValue = TestUtil.randomSimpleString(random());
      } while (L"" == randomValue);

      groups.push_back(make_shared<BytesRef>(randomValue));
    }
    const std::deque<wstring> contentStrings =
        std::deque<wstring>(TestUtil::nextInt(random(), 2, 20));
    if (VERBOSE) {
      wcout << L"TEST: create fake content" << endl;
    }
    for (int contentIDX = 0; contentIDX < contentStrings.size(); contentIDX++) {
      shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
      sb->append(L"real")->append(random()->nextInt(3))->append(L' ');
      constexpr int fakeCount = random()->nextInt(10);
      for (int fakeIDX = 0; fakeIDX < fakeCount; fakeIDX++) {
        sb->append(L"fake ");
      }
      contentStrings[contentIDX] = sb->toString();
      if (VERBOSE) {
        wcout << L"  content=" << sb->toString() << endl;
      }
    }

    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
        random(), dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Document> docNoGroup = make_shared<Document>();
    shared_ptr<Field> idvGroupField =
        make_shared<SortedDocValuesField>(L"group", make_shared<BytesRef>());
    doc->push_back(idvGroupField);
    docNoGroup->push_back(idvGroupField);

    shared_ptr<Field> group = newStringField(L"group", L"", Field::Store::NO);
    doc->push_back(group);
    shared_ptr<Field> sort1 =
        make_shared<SortedDocValuesField>(L"sort1", make_shared<BytesRef>());
    doc->push_back(sort1);
    docNoGroup->push_back(sort1);
    shared_ptr<Field> sort2 =
        make_shared<SortedDocValuesField>(L"sort2", make_shared<BytesRef>());
    doc->push_back(sort2);
    docNoGroup->push_back(sort2);
    shared_ptr<Field> content = newTextField(L"content", L"", Field::Store::NO);
    doc->push_back(content);
    docNoGroup->push_back(content);
    shared_ptr<NumericDocValuesField> idDV =
        make_shared<NumericDocValuesField>(L"id", 0);
    doc->push_back(idDV);
    docNoGroup->push_back(idDV);
    std::deque<std::shared_ptr<GroupDoc>> groupDocs(numDocs);
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<BytesRef> *const groupValue;
      if (random()->nextInt(24) == 17) {
        // So we test the "doc doesn't have the group'd
        // field" case:
        groupValue.reset();
      } else {
        groupValue = groups[random()->nextInt(groups.size())];
      }
      shared_ptr<GroupDoc> *const groupDoc = make_shared<GroupDoc>(
          i, groupValue, groups[random()->nextInt(groups.size())],
          groups[random()->nextInt(groups.size())],
          contentStrings[random()->nextInt(contentStrings.size())]);
      if (VERBOSE) {
        wcout << L"  doc content=" << groupDoc->content << L" id=" << i
              << L" group="
              << (groupDoc->group == nullptr ? L"null"
                                             : groupDoc->group->utf8ToString())
              << L" sort1=" << groupDoc->sort1->utf8ToString() << L" sort2="
              << groupDoc->sort2->utf8ToString() << endl;
      }

      groupDocs[i] = groupDoc;
      if (groupDoc->group != nullptr) {
        group->setStringValue(groupDoc->group->utf8ToString());
        idvGroupField->setBytesValue(BytesRef::deepCopyOf(groupDoc->group));
      } else {
        // TODO: not true
        // Must explicitly set empty string, else eg if
        // the segment has all docs missing the field then
        // we get null back instead of empty BytesRef:
        idvGroupField->setBytesValue(make_shared<BytesRef>());
      }
      sort1->setBytesValue(BytesRef::deepCopyOf(groupDoc->sort1));
      sort2->setBytesValue(BytesRef::deepCopyOf(groupDoc->sort2));
      content->setStringValue(groupDoc->content);
      idDV->setLongValue(groupDoc->id);
      if (groupDoc->group == nullptr) {
        w->addDocument(docNoGroup);
      } else {
        w->addDocument(doc);
      }
    }

    std::deque<std::shared_ptr<GroupDoc>> groupDocsByID(groupDocs.size());
    System::arraycopy(groupDocs, 0, groupDocsByID, 0, groupDocs.size());

    shared_ptr<DirectoryReader> *const r = w->getReader();
    delete w;

    shared_ptr<NumericDocValues> values =
        MultiDocValues::getNumericValues(r, L"id");
    std::deque<int> docIDToID(r->maxDoc());
    for (int i = 0; i < r->maxDoc(); i++) {
      assertEquals(i, values->nextDoc());
      docIDToID[i] = static_cast<int>(values->longValue());
    }
    shared_ptr<DirectoryReader> rBlocks = nullptr;
    shared_ptr<Directory> dirBlocks = nullptr;

    shared_ptr<IndexSearcher> *const s = newSearcher(r);
    // This test relies on the fact that longer fields produce lower scores
    s->setSimilarity(make_shared<BM25Similarity>());

    if (VERBOSE) {
      wcout << L"\nTEST: searcher=" << s << endl;
    }

    shared_ptr<ShardState> *const shards = make_shared<ShardState>(s);

    shared_ptr<Set<int>> seenIDs = unordered_set<int>();
    for (int contentID = 0; contentID < 3; contentID++) {
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          s->search(make_shared<TermQuery>(make_shared<Term>(
                        L"content", L"real" + to_wstring(contentID))),
                    numDocs)
              ->scoreDocs;
      for (auto hit : hits) {
        int idValue = docIDToID[hit->doc];

        shared_ptr<GroupDoc> *const gd = groupDocs[idValue];
        seenIDs->add(idValue);
        assertTrue(gd->score == 0.0);
        gd->score = hit->score;
        assertEquals(gd->id, idValue);
      }
    }

    // make sure all groups were seen across the hits
    assertEquals(groupDocs.size(), seenIDs->size());

    for (auto gd : groupDocs) {
      assertTrue(Float::isFinite(gd->score));
      assertTrue(gd->score >= 0.0);
    }

    // Build 2nd index, where docs are added in blocks by
    // group, so we can use single pass collector
    dirBlocks = newDirectory();
    rBlocks = getDocBlockReader(dirBlocks, groupDocs);
    shared_ptr<Query> *const lastDocInBlock =
        make_shared<TermQuery>(make_shared<Term>(L"groupend", L"x"));

    shared_ptr<IndexSearcher> *const sBlocks = newSearcher(rBlocks);
    // This test relies on the fact that longer fields produce lower scores
    sBlocks->setSimilarity(make_shared<BM25Similarity>());

    shared_ptr<ShardState> *const shardsBlocks =
        make_shared<ShardState>(sBlocks);

    // ReaderBlocks only increases maxDoc() vs reader, which
    // means a monotonic shift in scores, so we can
    // reliably remap them w/ Map:
    const unordered_map<wstring, unordered_map<float, float>> scoreMap =
        unordered_map<wstring, unordered_map<float, float>>();

    values = MultiDocValues::getNumericValues(rBlocks, L"id");
    assertNotNull(values);
    std::deque<int> docIDToIDBlocks(rBlocks->maxDoc());
    for (int i = 0; i < rBlocks->maxDoc(); i++) {
      assertEquals(i, values->nextDoc());
      docIDToIDBlocks[i] = static_cast<int>(values->longValue());
    }

    // Tricky: must separately set .score2, because the doc
    // block index was created with possible deletions!
    // System.out.println("fixup score2");
    for (int contentID = 0; contentID < 3; contentID++) {
      // System.out.println("  term=real" + contentID);
      const unordered_map<float, float> termScoreMap =
          unordered_map<float, float>();
      scoreMap.emplace(L"real" + to_wstring(contentID), termScoreMap);
      // System.out.println("term=real" + contentID + " dfold=" + s.docFreq(new
      // Term("content", "real"+contentID)) + " dfnew=" + sBlocks.docFreq(new
      //Term("content", "real"+contentID)));
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          sBlocks
              ->search(make_shared<TermQuery>(make_shared<Term>(
                           L"content", L"real" + to_wstring(contentID))),
                       numDocs)
              ->scoreDocs;
      for (auto hit : hits) {
        shared_ptr<GroupDoc> *const gd =
            groupDocsByID[docIDToIDBlocks[hit->doc]];
        assertTrue(gd->score2 == 0.0);
        gd->score2 = hit->score;
        assertEquals(gd->id, docIDToIDBlocks[hit->doc]);
        // System.out.println("    score=" + gd.score + " score2=" + hit.score +
        // " id=" + docIDToIDBlocks[hit.doc]);
        termScoreMap.emplace(gd->score, gd->score2);
      }
    }

    for (int searchIter = 0; searchIter < 100; searchIter++) {

      if (VERBOSE) {
        wcout << L"\nTEST: searchIter=" << searchIter << endl;
      }

      const wstring searchTerm = L"real" + random()->nextInt(3);
      constexpr bool fillFields = random()->nextBoolean();
      bool getScores = random()->nextBoolean();
      constexpr bool getMaxScores = random()->nextBoolean();
      shared_ptr<Sort> *const groupSort = getRandomSort();
      // final Sort groupSort = new Sort(new SortField[] {new SortField("sort1",
      // SortField.STRING), new SortField("id", SortField.INT)});
      shared_ptr<Sort> *const docSort = getRandomSort();

      getScores |= (groupSort->needsScores() || docSort->needsScores());

      constexpr int topNGroups = TestUtil::nextInt(random(), 1, 30);
      // final int topNGroups = 10;
      constexpr int docsPerGroup = TestUtil::nextInt(random(), 1, 50);

      constexpr int groupOffset =
          TestUtil::nextInt(random(), 0, (topNGroups - 1) / 2);
      // final int groupOffset = 0;

      constexpr int docOffset =
          TestUtil::nextInt(random(), 0, docsPerGroup - 1);
      // final int docOffset = 0;

      constexpr bool doCache = random()->nextBoolean();
      constexpr bool doAllGroups = random()->nextBoolean();
      if (VERBOSE) {
        wcout << L"TEST: groupSort=" << groupSort << L" docSort=" << docSort
              << L" searchTerm=" << searchTerm << L" dF="
              << r->docFreq(make_shared<Term>(L"content", searchTerm))
              << L" dFBlock="
              << rBlocks->docFreq(make_shared<Term>(L"content", searchTerm))
              << L" topNGroups=" << topNGroups << L" groupOffset="
              << groupOffset << L" docOffset=" << docOffset << L" doCache="
              << doCache << L" docsPerGroup=" << docsPerGroup
              << L" doAllGroups=" << doAllGroups << L" getScores=" << getScores
              << L" getMaxScores=" << getMaxScores << endl;
      }

      wstring groupField = L"group";
      if (VERBOSE) {
        wcout << L"  groupField=" << groupField << endl;
      }
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: final FirstPassGroupingCollector<?> c1 =
      // createRandomFirstPassCollector(groupField, groupSort,
      // groupOffset+topNGroups);
      shared_ptr < FirstPassGroupingCollector <
          ? >> *const c1 = createRandomFirstPassCollector(
                groupField, groupSort, groupOffset + topNGroups);
      shared_ptr<CachingCollector> *const cCache;
      shared_ptr<Collector> *const c;

      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: final AllGroupsCollector<?> allGroupsCollector;
      shared_ptr < AllGroupsCollector < ? >> *const allGroupsCollector;
      if (doAllGroups) {
        allGroupsCollector = createAllGroupsCollector(c1, groupField);
      } else {
        allGroupsCollector.reset();
      }

      constexpr bool useWrappingCollector = random()->nextBoolean();

      if (doCache) {
        constexpr double maxCacheMB = random()->nextDouble();
        if (VERBOSE) {
          wcout << L"TEST: maxCacheMB=" << maxCacheMB << endl;
        }

        if (useWrappingCollector) {
          if (doAllGroups) {
            cCache = CachingCollector::create(c1, true, maxCacheMB);
            c = MultiCollector::wrap({cCache, allGroupsCollector});
          } else {
            c = cCache = CachingCollector::create(c1, true, maxCacheMB);
          }
        } else {
          // Collect only into cache, then replay multiple times:
          c = cCache = CachingCollector::create(true, maxCacheMB);
        }
      } else {
        cCache.reset();
        if (doAllGroups) {
          c = MultiCollector::wrap({c1, allGroupsCollector});
        } else {
          c = c1;
        }
      }

      // Search top reader:
      shared_ptr<Query> *const query =
          make_shared<TermQuery>(make_shared<Term>(L"content", searchTerm));

      s->search(query, c);

      if (doCache && !useWrappingCollector) {
        if (cCache->isCached()) {
          // Replay for first-pass grouping
          cCache->replay(c1);
          if (doAllGroups) {
            // Replay for all groups:
            cCache->replay(allGroupsCollector);
          }
        } else {
          // Replay by re-running search:
          s->search(query, c1);
          if (doAllGroups) {
            s->search(query, allGroupsCollector);
          }
        }
      }

      // Get 1st pass top groups
      shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>>
          *const topGroups = getSearchGroups(c1, groupOffset, fillFields);
      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> *const groupsResult;
      if (VERBOSE) {
        wcout << L"TEST: first pass topGroups" << endl;
        if (topGroups == nullptr) {
          wcout << L"  null" << endl;
        } else {
          for (auto searchGroup : topGroups) {
            wcout << L"  "
                  << (searchGroup->groupValue == nullptr
                          ? L"null"
                          : searchGroup->groupValue)
                  << L": " << Arrays::deepToString(searchGroup->sortValues)
                  << endl;
          }
        }
      }

      // Get 1st pass top groups using shards

      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> *const topGroupsShards =
          searchShards(s, shards->subSearchers, query, groupSort, docSort,
                       groupOffset, topNGroups, docOffset, docsPerGroup,
                       getScores, getMaxScores, true, true);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: final TopGroupsCollector<?> c2;
      shared_ptr < TopGroupsCollector < ? >> *const c2;
      if (topGroups != nullptr) {

        if (VERBOSE) {
          wcout << L"TEST: topGroups" << endl;
          for (auto searchGroup : topGroups) {
            wcout << L"  "
                  << (searchGroup->groupValue == nullptr
                          ? L"null"
                          : searchGroup->groupValue->utf8ToString())
                  << L": " << Arrays::deepToString(searchGroup->sortValues)
                  << endl;
          }
        }

        c2 = createSecondPassCollector(c1, groupSort, docSort, groupOffset,
                                       docOffset + docsPerGroup, getScores,
                                       getMaxScores, fillFields);
        if (doCache) {
          if (cCache->isCached()) {
            if (VERBOSE) {
              wcout << L"TEST: cache is intact" << endl;
            }
            cCache->replay(c2);
          } else {
            if (VERBOSE) {
              wcout << L"TEST: cache was too large" << endl;
            }
            s->search(query, c2);
          }
        } else {
          s->search(query, c2);
        }

        if (doAllGroups) {
          shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> tempTopGroups =
              getTopGroups(c2, docOffset);
          groupsResult = make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
              tempTopGroups, allGroupsCollector->getGroupCount());
        } else {
          groupsResult = getTopGroups(c2, docOffset);
        }
      } else {
        c2.reset();
        groupsResult.reset();
        if (VERBOSE) {
          wcout << L"TEST:   no results" << endl;
        }
      }

      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> *const expectedGroups =
          slowGrouping(groupDocs, searchTerm, fillFields, getScores,
                       getMaxScores, doAllGroups, groupSort, docSort,
                       topNGroups, docsPerGroup, groupOffset, docOffset);

      if (VERBOSE) {
        if (expectedGroups == nullptr) {
          wcout << L"TEST: no expected groups" << endl;
        } else {
          wcout << L"TEST: expected groups totalGroupedHitCount="
                << expectedGroups->totalGroupedHitCount << endl;
          for (auto gd : expectedGroups->groups) {
            wcout << L"  group="
                  << (gd->groupValue == nullptr ? L"null" : gd->groupValue)
                  << L" totalHits=" << gd->totalHits << L" scoreDocs.len="
                  << gd->scoreDocs.size() << endl;
            for (auto sd : gd->scoreDocs) {
              wcout << L"    id=" << sd->doc << L" score=" << sd->score << endl;
            }
          }
        }

        if (groupsResult == nullptr) {
          wcout << L"TEST: no matched groups" << endl;
        } else {
          wcout << L"TEST: matched groups totalGroupedHitCount="
                << groupsResult->totalGroupedHitCount << endl;
          for (auto gd : groupsResult->groups) {
            wcout << L"  group="
                  << (gd->groupValue == nullptr ? L"null" : gd->groupValue)
                  << L" totalHits=" << gd->totalHits << endl;
            for (auto sd : gd->scoreDocs) {
              wcout << L"    id=" << docIDToID[sd->doc] << L" score="
                    << sd->score << endl;
            }
          }

          if (searchIter == 14) {
            for (int docIDX = 0; docIDX < s->getIndexReader()->maxDoc();
                 docIDX++) {
              wcout << L"ID=" << docIDToID[docIDX] << L" explain="
                    << s->explain(query, docIDX) << endl;
            }
          }
        }

        if (topGroupsShards == nullptr) {
          wcout << L"TEST: no matched-merged groups" << endl;
        } else {
          wcout << L"TEST: matched-merged groups totalGroupedHitCount="
                << topGroupsShards->totalGroupedHitCount << endl;
          for (auto gd : topGroupsShards->groups) {
            wcout << L"  group="
                  << (gd->groupValue == nullptr ? L"null" : gd->groupValue)
                  << L" totalHits=" << gd->totalHits << endl;
            for (auto sd : gd->scoreDocs) {
              wcout << L"    id=" << docIDToID[sd->doc] << L" score="
                    << sd->score << endl;
            }
          }
        }
      }

      assertEquals(docIDToID, expectedGroups, groupsResult, true, true, true,
                   getScores, true);

      // Confirm merged shards match:
      assertEquals(docIDToID, expectedGroups, topGroupsShards, true, false,
                   fillFields, getScores, true);
      if (topGroupsShards != nullptr) {
        verifyShards(shards->docStarts, topGroupsShards);
      }

      constexpr bool needsScores =
          getScores || getMaxScores || docSort == nullptr;
      shared_ptr<BlockGroupingCollector> *const c3 =
          make_shared<BlockGroupingCollector>(
              groupSort, groupOffset + topNGroups, needsScores,
              sBlocks->createWeight(sBlocks->rewrite(lastDocInBlock), false,
                                    1));
      shared_ptr<AllGroupsCollector<std::shared_ptr<BytesRef>>>
          *const allGroupsCollector2;
      shared_ptr<Collector> *const c4;
      if (doAllGroups) {
        // NOTE: must be "group" and not "group_dv"
        // (groupField) because we didn't index doc
        // values in the block index:
        allGroupsCollector2 =
            make_shared<AllGroupsCollector<std::shared_ptr<BytesRef>>>(
                make_shared<TermGroupSelector>(L"group"));
        c4 = MultiCollector::wrap({c3, allGroupsCollector2});
      } else {
        allGroupsCollector2.reset();
        c4 = c3;
      }
      // Get block grouping result:
      sBlocks->search(query, c4);
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
      // TopGroups<org.apache.lucene.util.BytesRef> tempTopGroupsBlocks =
      // (TopGroups<org.apache.lucene.util.BytesRef>) c3.getTopGroups(docSort,
      // groupOffset, docOffset, docOffset+docsPerGroup, fillFields);
      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
          *const tempTopGroupsBlocks =
              std::static_pointer_cast<TopGroups<std::shared_ptr<BytesRef>>>(
                  c3->getTopGroups(docSort, groupOffset, docOffset,
                                   docOffset + docsPerGroup, fillFields));
      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
          *const groupsResultBlocks;
      if (doAllGroups && tempTopGroupsBlocks != nullptr) {
        assertEquals(static_cast<int>(tempTopGroupsBlocks->totalGroupCount),
                     allGroupsCollector2->getGroupCount());
        groupsResultBlocks = make_shared<TopGroups<std::shared_ptr<BytesRef>>>(
            tempTopGroupsBlocks, allGroupsCollector2->getGroupCount());
      } else {
        groupsResultBlocks = tempTopGroupsBlocks;
      }

      if (VERBOSE) {
        if (groupsResultBlocks == nullptr) {
          wcout << L"TEST: no block groups" << endl;
        } else {
          wcout << L"TEST: block groups totalGroupedHitCount="
                << groupsResultBlocks->totalGroupedHitCount << endl;
          bool first = true;
          for (auto gd : groupsResultBlocks->groups) {
            wcout << L"  group="
                  << (gd->groupValue == nullptr
                          ? L"null"
                          : gd->groupValue->utf8ToString())
                  << L" totalHits=" << gd->totalHits << endl;
            for (auto sd : gd->scoreDocs) {
              wcout << L"    id=" << docIDToIDBlocks[sd->doc] << L" score="
                    << sd->score << endl;
              if (first) {
                wcout << L"explain: " << sBlocks->explain(query, sd->doc)
                      << endl;
                first = false;
              }
            }
          }
        }
      }

      // Get shard'd block grouping result:
      shared_ptr<TopGroups<std::shared_ptr<BytesRef>>>
          *const topGroupsBlockShards =
              searchShards(sBlocks, shardsBlocks->subSearchers, query,
                           groupSort, docSort, groupOffset, topNGroups,
                           docOffset, docsPerGroup, getScores, getMaxScores,
                           false, false);

      if (expectedGroups != nullptr) {
        // Fixup scores for reader2
        for (auto groupDocsHits : expectedGroups->groups) {
          for (auto hit : groupDocsHits->scoreDocs) {
            shared_ptr<GroupDoc> *const gd = groupDocsByID[hit->doc];
            assertEquals(gd->id, hit->doc);
            // System.out.println("fixup score " + hit.score + " to " +
            // gd.score2 + " vs " + gd.score);
            hit->score = gd->score2;
          }
        }

        std::deque<std::shared_ptr<SortField>> sortFields =
            groupSort->getSort();
        const unordered_map<float, float> termScoreMap = scoreMap[searchTerm];
        for (int groupSortIDX = 0; groupSortIDX < sortFields.size();
             groupSortIDX++) {
          if (sortFields[groupSortIDX]->getType() == SortField::Type::SCORE) {
            for (auto groupDocsHits : expectedGroups->groups) {
              if (groupDocsHits->groupSortValues.size() > 0) {
                // System.out.println("remap " +
                // groupDocsHits.groupSortValues[groupSortIDX] + " to " +
                // termScoreMap.get(groupDocsHits.groupSortValues[groupSortIDX]));
                groupDocsHits->groupSortValues[groupSortIDX] =
                    termScoreMap[groupDocsHits->groupSortValues[groupSortIDX]];
                assertNotNull(groupDocsHits->groupSortValues[groupSortIDX]);
              }
            }
          }
        }

        std::deque<std::shared_ptr<SortField>> docSortFields =
            docSort->getSort();
        for (int docSortIDX = 0; docSortIDX < docSortFields.size();
             docSortIDX++) {
          if (docSortFields[docSortIDX]->getType() == SortField::Type::SCORE) {
            for (auto groupDocsHits : expectedGroups->groups) {
              for (auto _hit : groupDocsHits->scoreDocs) {
                shared_ptr<FieldDoc> hit =
                    std::static_pointer_cast<FieldDoc>(_hit);
                if (hit->fields.size() > 0) {
                  hit->fields[docSortIDX] =
                      termScoreMap[hit->fields[docSortIDX]];
                  assertNotNull(hit->fields[docSortIDX]);
                }
              }
            }
          }
        }
      }

      assertEquals(docIDToIDBlocks, expectedGroups, groupsResultBlocks, false,
                   true, true, getScores, false);
      assertEquals(docIDToIDBlocks, expectedGroups, topGroupsBlockShards, false,
                   false, fillFields, getScores, false);
    }

    r->close();
    delete dir;

    rBlocks->close();
    delete dirBlocks;
  }
}

void TestGrouping::verifyShards(
    std::deque<int> &docStarts,
    shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> topGroups)
{
  for (auto group : topGroups->groups) {
    for (int hitIDX = 0; hitIDX < group->scoreDocs.size(); hitIDX++) {
      shared_ptr<ScoreDoc> *const sd = group->scoreDocs[hitIDX];
      assertEquals(L"doc=" + to_wstring(sd->doc) + L" wrong shard",
                   ReaderUtil::subIndex(sd->doc, docStarts), sd->shardIndex);
    }
  }
}

shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> TestGrouping::searchShards(
    shared_ptr<IndexSearcher> topSearcher,
    std::deque<std::shared_ptr<ShardSearcher>> &subSearchers,
    shared_ptr<Query> query, shared_ptr<Sort> groupSort,
    shared_ptr<Sort> docSort, int groupOffset, int topNGroups, int docOffset,
    int topNDocs, bool getScores, bool getMaxScores, bool canUseIDV,
    bool preFlex) 
{

  // TODO: swap in caching, all groups collector
  // hereassertEquals(expected.totalHitCount, actual.totalHitCount); too...
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"TEST: " << subSearchers.size() << L" shards: "
          << Arrays->toString(subSearchers) << L" canUseIDV=" << canUseIDV
          << endl;
  }
  // Run 1st pass collector to get top groups per shard
  shared_ptr<Weight> *const w =
      topSearcher->createWeight(topSearcher->rewrite(query), getScores, 1);
  const deque<deque<SearchGroup<std::shared_ptr<BytesRef>>>> shardGroups =
      deque<deque<SearchGroup<std::shared_ptr<BytesRef>>>>();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.List<FirstPassGroupingCollector<?>>
  // firstPassGroupingCollectors = new java.util.ArrayList<>();
  deque < FirstPassGroupingCollector <
      ? >> firstPassGroupingCollectors =
            deque < FirstPassGroupingCollector < ? >> ();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: FirstPassGroupingCollector<?> firstPassCollector = null;
  shared_ptr < FirstPassGroupingCollector < ? >> firstPassCollector = nullptr;
  bool shardsCanUseIDV = canUseIDV;

  wstring groupField = L"group";

  for (int shardIDX = 0; shardIDX < subSearchers.size(); shardIDX++) {

    // First shard determines whether we use IDV or not;
    // all other shards match that:
    if (firstPassCollector == nullptr) {
      firstPassCollector = createRandomFirstPassCollector(
          groupField, groupSort, groupOffset + topNGroups);
    } else {
      firstPassCollector = createFirstPassCollector(
          groupField, groupSort, groupOffset + topNGroups, firstPassCollector);
    }
    if (VERBOSE) {
      wcout << L"  shard=" << shardIDX << L" groupField=" << groupField << endl;
      wcout << L"    1st pass collector=" << firstPassCollector << endl;
    }
    firstPassGroupingCollectors.push_back(firstPassCollector);
    subSearchers[shardIDX]->search(w, firstPassCollector);
    shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>>
        *const topGroups = getSearchGroups(firstPassCollector, 0, true);
    if (topGroups != nullptr) {
      if (VERBOSE) {
        wcout << L"  shard " << shardIDX << L" s=" << subSearchers[shardIDX]
              << L" totalGroupedHitCount=?" << L" " << topGroups->size()
              << L" groups:" << endl;
        for (auto group : topGroups) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"    " << groupToString(group->groupValue) << L" groupSort="
                << Arrays->toString(group->sortValues) << endl;
        }
      }
      shardGroups.push_back(topGroups);
    }
  }

  shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>>
      *const mergedTopGroups =
          SearchGroup::merge(shardGroups, groupOffset, topNGroups, groupSort);
  if (VERBOSE) {
    wcout << L" top groups merged:" << endl;
    if (mergedTopGroups == nullptr) {
      wcout << L"    null" << endl;
    } else {
      wcout << L"    " << mergedTopGroups->size() << L" top groups:" << endl;
      for (auto group : mergedTopGroups) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"    [" << groupToString(group->groupValue) << L"] groupSort="
              << Arrays->toString(group->sortValues) << endl;
      }
    }
  }

  if (mergedTopGroups != nullptr) {
    // Now 2nd pass:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
    // TopGroups<org.apache.lucene.util.BytesRef>[] shardTopGroups = new
    // TopGroups[subSearchers.length];
    std::deque<TopGroups<std::shared_ptr<BytesRef>>> shardTopGroups =
        std::deque<std::shared_ptr<TopGroups>>(subSearchers.size());
    for (int shardIDX = 0; shardIDX < subSearchers.size(); shardIDX++) {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: final TopGroupsCollector<?> secondPassCollector =
      // createSecondPassCollector(firstPassGroupingCollectors.get(shardIDX),
      // groupField, mergedTopGroups, groupSort, docSort, docOffset + topNDocs,
      // getScores, getMaxScores, true);
      shared_ptr < TopGroupsCollector <
          ? >> *const secondPassCollector = createSecondPassCollector(
                firstPassGroupingCollectors[shardIDX], groupField,
                mergedTopGroups, groupSort, docSort, docOffset + topNDocs,
                getScores, getMaxScores, true);
      subSearchers[shardIDX]->search(w, secondPassCollector);
      shardTopGroups[shardIDX] = getTopGroups(secondPassCollector, 0);
      if (VERBOSE) {
        wcout << L" " << shardTopGroups[shardIDX]->groups->size() << L" shard["
              << shardIDX << L"] groups:" << endl;
        for (auto group : shardTopGroups[shardIDX]->groups) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"    [" << groupToString(group->groupValue)
                << L"] groupSort=" << Arrays->toString(group->groupSortValues)
                << L" numDocs=" << group->scoreDocs.size() << endl;
        }
      }
    }

    shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> mergedGroups =
        TopGroups::merge(shardTopGroups, groupSort, docSort, docOffset,
                         topNDocs, TopGroups::ScoreMergeMode::None);
    if (VERBOSE) {
      wcout << L" " << mergedGroups->groups.size() << L" merged groups:"
            << endl;
      for (auto group : mergedGroups->groups) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"    [" << groupToString(group->groupValue) << L"] groupSort="
              << Arrays->toString(group->groupSortValues) << L" numDocs="
              << group->scoreDocs.size() << endl;
      }
    }
    return mergedGroups;
  } else {
    return nullptr;
  }
}

void TestGrouping::assertEquals(
    std::deque<int> &docIDtoID,
    shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> expected,
    shared_ptr<TopGroups<std::shared_ptr<BytesRef>>> actual,
    bool verifyGroupValues, bool verifyTotalGroupCount, bool verifySortValues,
    bool testScores, bool idvBasedImplsUsed)
{
  if (expected == nullptr) {
    assertNull(actual);
    return;
  }
  assertNotNull(actual);

  assertEquals(L"expected.groups.length != actual.groups.length",
               expected->groups.size(), actual->groups.size());
  assertEquals(L"expected.totalHitCount != actual.totalHitCount",
               expected->totalHitCount, actual->totalHitCount);
  assertEquals(L"expected.totalGroupedHitCount != actual.totalGroupedHitCount",
               expected->totalGroupedHitCount, actual->totalGroupedHitCount);
  if (expected->totalGroupCount && verifyTotalGroupCount) {
    assertEquals(L"expected.totalGroupCount != actual.totalGroupCount",
                 expected->totalGroupCount, actual->totalGroupCount);
  }

  for (int groupIDX = 0; groupIDX < expected->groups.size(); groupIDX++) {
    if (VERBOSE) {
      wcout << L"  check groupIDX=" << groupIDX << endl;
    }
    shared_ptr<GroupDocs<std::shared_ptr<BytesRef>>> *const expectedGroup =
        expected->groups[groupIDX];
    shared_ptr<GroupDocs<std::shared_ptr<BytesRef>>> *const actualGroup =
        actual->groups[groupIDX];
    if (verifyGroupValues) {
      if (idvBasedImplsUsed) {
        if (actualGroup->groupValue->length == 0) {
          assertNull(expectedGroup->groupValue);
        } else {
          assertEquals(expectedGroup->groupValue, actualGroup->groupValue);
        }
      } else {
        assertEquals(expectedGroup->groupValue, actualGroup->groupValue);
      }
    }
    if (verifySortValues) {
      assertArrayEquals(expectedGroup->groupSortValues,
                        actualGroup->groupSortValues);
    }

    // TODO
    // assertEquals(expectedGroup.maxScore, actualGroup.maxScore);
    assertEquals(expectedGroup->totalHits, actualGroup->totalHits);

    std::deque<std::shared_ptr<ScoreDoc>> expectedFDs =
        expectedGroup->scoreDocs;
    std::deque<std::shared_ptr<ScoreDoc>> actualFDs = actualGroup->scoreDocs;

    assertEquals(expectedFDs.size(), actualFDs.size());
    for (int docIDX = 0; docIDX < expectedFDs.size(); docIDX++) {
      shared_ptr<FieldDoc> *const expectedFD =
          std::static_pointer_cast<FieldDoc>(expectedFDs[docIDX]);
      shared_ptr<FieldDoc> *const actualFD =
          std::static_pointer_cast<FieldDoc>(actualFDs[docIDX]);
      // System.out.println("  actual doc=" + docIDtoID[actualFD.doc] + "
      // score=" + actualFD.score);
      assertEquals(expectedFD->doc, docIDtoID[actualFD->doc]);
      if (testScores) {
        assertEquals(expectedFD->score, actualFD->score, 0.1);
      } else {
        // TODO: too anal for now
        // assertEquals(Float.NaN, actualFD.score);
      }
      if (verifySortValues) {
        assertArrayEquals(expectedFD->fields, actualFD->fields);
      }
    }
  }
}

TestGrouping::ShardSearcher::ShardSearcher(
    shared_ptr<LeafReaderContext> ctx, shared_ptr<IndexReaderContext> parent)
    : org::apache::lucene::search::IndexSearcher(parent),
      ctx(Collections::singletonList(ctx))
{
}

void TestGrouping::ShardSearcher::search(
    shared_ptr<Weight> weight,
    shared_ptr<Collector> collector) 
{
  search(ctx, weight, collector);
}

wstring TestGrouping::ShardSearcher::toString()
{
  return L"ShardSearcher(" + ctx[0]->reader() + L")";
}
} // namespace org::apache::lucene::search::grouping