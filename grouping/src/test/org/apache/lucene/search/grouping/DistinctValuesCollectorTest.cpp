using namespace std;

#include "DistinctValuesCollectorTest.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;
const shared_ptr<NullComparator> DistinctValuesCollectorTest::nullComparator =
    make_shared<NullComparator>();
const wstring DistinctValuesCollectorTest::GROUP_FIELD = L"author";
const wstring DistinctValuesCollectorTest::COUNT_FIELD = L"publisher";

void DistinctValuesCollectorTest::testSimple() 
{
  shared_ptr<Random> random = DistinctValuesCollectorTest::random();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random, dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random))
          ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Document> doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"1");
  addField(doc, COUNT_FIELD, L"1");
  doc->push_back(
      make_shared<TextField>(L"content", L"random text", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"1");
  addField(doc, COUNT_FIELD, L"1");
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random text blob", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::NO));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"1");
  addField(doc, COUNT_FIELD, L"2");
  doc->push_back(make_shared<TextField>(
      L"content", L"some more random textual data", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"3", Field::Store::NO));
  w->addDocument(doc);
  w->commit(); // To ensure a second segment

  // 3 -- no count field
  doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"2");
  doc->push_back(make_shared<TextField>(L"content", L"some random text",
                                        Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"4", Field::Store::NO));
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"3");
  addField(doc, COUNT_FIELD, L"1");
  doc->push_back(make_shared<TextField>(L"content", L"some more random text",
                                        Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"5", Field::Store::NO));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  addField(doc, GROUP_FIELD, L"3");
  addField(doc, COUNT_FIELD, L"1");
  doc->push_back(
      make_shared<TextField>(L"content", L"random blob", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"id", L"6", Field::Store::NO));
  w->addDocument(doc);

  // 6 -- no author field
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content", L"random word stuck in alot of other text",
      Field::Store::YES));
  addField(doc, COUNT_FIELD, L"1");
  doc->push_back(make_shared<StringField>(L"id", L"6", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(w->getReader());
  delete w;

  shared_ptr<Comparator<
      DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>>
      cmp = [&](groupCount1, groupCount2) {
        if (groupCount1->groupValue == nullptr) {
          if (groupCount2->groupValue == nullptr) {
            return 0;
          }
          return -1;
        } else if (groupCount2->groupValue == nullptr) {
          return 1;
        } else {
          return groupCount1::groupValue->compareTo(groupCount2::groupValue);
        }
      };

  // === Search for content:random
  shared_ptr<FirstPassGroupingCollector<Comparable<any>>> firstCollector =
      createRandomFirstPassCollector(make_shared<Sort>(), GROUP_FIELD, 10);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      firstCollector);
  shared_ptr<DistinctValuesCollector<Comparable<any>, Comparable<any>>>
      distinctValuesCollector =
          createDistinctCountCollector(firstCollector, COUNT_FIELD);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      distinctValuesCollector);

  deque<DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>
      gcs = distinctValuesCollector->getGroups();
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(gcs, cmp);
  sort(gcs.begin(), gcs.end(), cmp);
  TestUtil::assertEquals(4, gcs.size());

  compareNull(gcs[0]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.List<Comparable<?>> countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(0).uniqueValues);
  deque < Comparable < ? >> countValues =
                              deque < Comparable < ? >> (gcs[0]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compare(L"1", countValues[0]);

  compare(L"1", gcs[1]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(1).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[1]->uniqueValues);
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(countValues, nullComparator);
  sort(countValues.begin(), countValues.end(), nullComparator);
  TestUtil::assertEquals(2, countValues.size());
  compare(L"1", countValues[0]);
  compare(L"2", countValues[1]);

  compare(L"2", gcs[2]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(2).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[2]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compareNull(countValues[0]);

  compare(L"3", gcs[3]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(3).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[3]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compare(L"1", countValues[0]);

  // === Search for content:some
  firstCollector =
      createRandomFirstPassCollector(make_shared<Sort>(), GROUP_FIELD, 10);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"some")),
      firstCollector);
  distinctValuesCollector =
      createDistinctCountCollector(firstCollector, COUNT_FIELD);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"some")),
      distinctValuesCollector);

  gcs = distinctValuesCollector->getGroups();
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(gcs, cmp);
  sort(gcs.begin(), gcs.end(), cmp);
  TestUtil::assertEquals(3, gcs.size());

  compare(L"1", gcs[0]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(0).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[0]->uniqueValues);
  TestUtil::assertEquals(2, countValues.size());
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(countValues, nullComparator);
  sort(countValues.begin(), countValues.end(), nullComparator);
  compare(L"1", countValues[0]);
  compare(L"2", countValues[1]);

  compare(L"2", gcs[1]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(1).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[1]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compareNull(countValues[0]);

  compare(L"3", gcs[2]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(2).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[2]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compare(L"1", countValues[0]);

  // === Search for content:blob
  firstCollector =
      createRandomFirstPassCollector(make_shared<Sort>(), GROUP_FIELD, 10);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"blob")),
      firstCollector);
  distinctValuesCollector =
      createDistinctCountCollector(firstCollector, COUNT_FIELD);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"blob")),
      distinctValuesCollector);

  gcs = distinctValuesCollector->getGroups();
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(gcs, cmp);
  sort(gcs.begin(), gcs.end(), cmp);
  TestUtil::assertEquals(2, gcs.size());

  compare(L"1", gcs[0]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(0).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[0]->uniqueValues);
  // B/c the only one document matched with blob inside the author 1 group
  TestUtil::assertEquals(1, countValues.size());
  compare(L"1", countValues[0]);

  compare(L"3", gcs[1]->groupValue);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: countValues = new
  // java.util.ArrayList<Comparable<?>>(gcs.get(1).uniqueValues);
  countValues = deque < Comparable < ? >> (gcs[1]->uniqueValues);
  TestUtil::assertEquals(1, countValues.size());
  compare(L"1", countValues[0]);

  delete indexSearcher->getIndexReader();
  delete dir;
}

void DistinctValuesCollectorTest::testRandom() 
{
  shared_ptr<Random> random = DistinctValuesCollectorTest::random();
  int numberOfRuns = TestUtil::nextInt(random, 3, 6);
  for (int indexIter = 0; indexIter < numberOfRuns; indexIter++) {
    shared_ptr<IndexContext> context = createIndexContext();
    for (int searchIter = 0; searchIter < 100; searchIter++) {
      shared_ptr<IndexSearcher> *const searcher =
          newSearcher(context->indexReader);
      wstring term =
          context
              ->contentStrings[random->nextInt(context->contentStrings.size())];
      shared_ptr<Sort> groupSort = make_shared<Sort>(
          make_shared<SortField>(L"id", SortField::Type::STRING));
      int topN = 1 + random->nextInt(10);

      deque<
          DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>
          expectedResult = createExpectedResult(context, term, groupSort, topN);

      shared_ptr<FirstPassGroupingCollector<Comparable<any>>> firstCollector =
          createRandomFirstPassCollector(groupSort, GROUP_FIELD, topN);
      searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"content", term)),
          firstCollector);
      shared_ptr<DistinctValuesCollector<Comparable<any>, Comparable<any>>>
          distinctValuesCollector =
              createDistinctCountCollector(firstCollector, COUNT_FIELD);
      searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"content", term)),
          distinctValuesCollector);
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked")
      // java.util.List<DistinctValuesCollector.GroupCount<Comparable<Object>,
      // Comparable<Object>>> actualResult = distinctValuesCollector.getGroups();
      deque<
          DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>
          actualResult = distinctValuesCollector->getGroups();

      if (VERBOSE) {
        wcout << L"Index iter=" << indexIter << endl;
        wcout << L"Search iter=" << searchIter << endl;
        wcout << L"1st pass collector class name="
              << firstCollector->getClassName() << endl;
        wcout << L"2nd pass collector class name="
              << distinctValuesCollector->getClassName() << endl;
        wcout << L"Search term=" << term << endl;
        wcout << L"1st pass groups=" << firstCollector->getTopGroups(0, false)
              << endl;
        wcout << L"Expected:" << endl;
        printGroups(expectedResult);
        wcout << L"Actual:" << endl;
        printGroups(actualResult);
      }

      TestUtil::assertEquals(expectedResult.size(), actualResult.size());
      for (int i = 0; i < expectedResult.size(); i++) {
        shared_ptr<DistinctValuesCollector::GroupCount<Comparable<any>,
                                                       Comparable<any>>>
            expected = expectedResult[i];
        shared_ptr<DistinctValuesCollector::GroupCount<Comparable<any>,
                                                       Comparable<any>>>
            actual = actualResult[i];
        assertValues(expected->groupValue, actual->groupValue);
        TestUtil::assertEquals(expected->uniqueValues->size(),
                               actual->uniqueValues->size());
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: java.util.List<Comparable<?>> expectedUniqueValues =
        // new java.util.ArrayList<>(expected.uniqueValues);
        deque < Comparable < ? >> expectedUniqueValues = deque < Comparable <
                                ? >> (expected->uniqueValues);
        // C++ TODO: The 'Compare' parameter of std::sort produces a bool
        // value, while the Java Comparator parameter produces a tri-state
        // result: ORIGINAL LINE:
        // java.util.Collections.sort(expectedUniqueValues, nullComparator);
        sort(expectedUniqueValues.begin(), expectedUniqueValues.end(),
             nullComparator);
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: java.util.List<Comparable<?>> actualUniqueValues = new
        // java.util.ArrayList<>(actual.uniqueValues);
        deque < Comparable < ? >> actualUniqueValues = deque < Comparable <
                                ? >> (actual->uniqueValues);
        // C++ TODO: The 'Compare' parameter of std::sort produces a bool
        // value, while the Java Comparator parameter produces a tri-state
        // result: ORIGINAL LINE: java.util.Collections.sort(actualUniqueValues,
        // nullComparator);
        sort(actualUniqueValues.begin(), actualUniqueValues.end(),
             nullComparator);
        for (int j = 0; j < expectedUniqueValues.size(); j++) {
          assertValues(expectedUniqueValues[j], actualUniqueValues[j]);
        }
      }
    }
    context->indexReader->close();
    delete context->directory;
  }
}

void DistinctValuesCollectorTest::printGroups(
    deque<DistinctValuesCollector::GroupCount<Comparable<any>,
                                               Comparable<any>>> &results)
{
  for (int i = 0; i < results.size(); i++) {
    shared_ptr<
        DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>
        group = results[i];
    any gv = group->groupValue;
    if (std::dynamic_pointer_cast<BytesRef>(gv) != nullptr) {
      wcout << i << L": groupValue="
            << (any_cast<std::shared_ptr<BytesRef>>(gv)).utf8ToString() << endl;
    } else {
      wcout << i << L": groupValue=" << gv << endl;
    }
    for (auto o : group->uniqueValues) {
      if (std::dynamic_pointer_cast<BytesRef>(o) != nullptr) {
        wcout << L"  "
              << (std::static_pointer_cast<BytesRef>(o))->utf8ToString()
              << endl;
      } else {
        wcout << L"  " << o << endl;
      }
    }
  }
}

void DistinctValuesCollectorTest::assertValues(any expected, any actual)
{
  if (expected == nullptr) {
    compareNull(actual);
  } else {
    compare((any_cast<std::shared_ptr<BytesRef>>(expected)).utf8ToString(),
            actual);
  }
}

void DistinctValuesCollectorTest::compare(const wstring &expected,
                                          any groupValue)
{
  if (BytesRef::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(
        expected,
        (any_cast<std::shared_ptr<BytesRef>>(groupValue)).utf8ToString());
  } else if (Double::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(stod(expected), groupValue);
  } else if (Long::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(StringHelper::fromString<int64_t>(expected),
                           groupValue);
  } else if (MutableValue::typeid->isAssignableFrom(groupValue.type())) {
    shared_ptr<MutableValueStr> mutableValue = make_shared<MutableValueStr>();
    mutableValue->value->copyChars(expected);
    TestUtil::assertEquals(mutableValue, groupValue);
  } else {
    fail();
  }
}

void DistinctValuesCollectorTest::compareNull(any groupValue)
{
  if (groupValue == nullptr) {
    return; // term based impl...
  }
  // DV based impls..
  if (BytesRef::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(
        L"", (any_cast<std::shared_ptr<BytesRef>>(groupValue)).utf8ToString());
  } else if (Double::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(0.0, groupValue);
  } else if (Long::typeid->isAssignableFrom(groupValue.type())) {
    TestUtil::assertEquals(0LL, groupValue);
    // Function based impl
  } else if (MutableValue::typeid->isAssignableFrom(groupValue.type())) {
    assertFalse((any_cast<std::shared_ptr<MutableValue>>(groupValue)).exists());
  } else {
    fail();
  }
}

void DistinctValuesCollectorTest::addField(shared_ptr<Document> doc,
                                           const wstring &field,
                                           const wstring &value)
{
  doc->push_back(
      make_shared<SortedDocValuesField>(field, make_shared<BytesRef>(value)));
}

template <typename T, typename R>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T extends
// Comparable<Object>, R extends Comparable<Object>> DistinctValuesCollector<T,
// R> createDistinctCountCollector(FirstPassGroupingCollector<T>
// firstPassGroupingCollector, std::wstring countField) throws java.io.IOException
shared_ptr<DistinctValuesCollector<T, R>>
DistinctValuesCollectorTest::createDistinctCountCollector(
    shared_ptr<FirstPassGroupingCollector<T>> firstPassGroupingCollector,
    const wstring &countField) 
{
  static_assert(is_base_of<Comparable<any>, R>::value,
                L"R must inherit from Comparable<std::any>");
  static_assert(is_base_of<Comparable<any>, T>::value,
                L"T must inherit from Comparable<std::any>");

  shared_ptr<deque<SearchGroup<T>>> searchGroups =
      firstPassGroupingCollector->getTopGroups(0, false);
  shared_ptr<GroupSelector<T>> selector =
      firstPassGroupingCollector->getGroupSelector();
  if (ValueSourceGroupSelector::typeid->isAssignableFrom(
          selector->getClass())) {
    shared_ptr<GroupSelector> gs = make_shared<ValueSourceGroupSelector>(
        make_shared<BytesRefFieldSource>(countField), unordered_map<>());
    return make_shared<DistinctValuesCollector<T, R>>(selector, searchGroups,
                                                      gs);
  } else {
    shared_ptr<GroupSelector> ts = make_shared<TermGroupSelector>(countField);
    return make_shared<DistinctValuesCollector<T, R>>(selector, searchGroups,
                                                      ts);
  }
}

template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private <T>
// FirstPassGroupingCollector<T>
// createRandomFirstPassCollector(org.apache.lucene.search.Sort groupSort, std::wstring
// groupField, int topNGroups) throws java.io.IOException
shared_ptr<FirstPassGroupingCollector<T>>
DistinctValuesCollectorTest::createRandomFirstPassCollector(
    shared_ptr<Sort> groupSort, const wstring &groupField,
    int topNGroups) 
{
  shared_ptr<Random> random = DistinctValuesCollectorTest::random();
  if (random->nextBoolean()) {
    return std::static_pointer_cast<FirstPassGroupingCollector<T>>(
        make_shared<FirstPassGroupingCollector<T>>(
            make_shared<ValueSourceGroupSelector>(
                make_shared<BytesRefFieldSource>(groupField),
                unordered_map<>()),
            groupSort, topNGroups));
  } else {
    return std::static_pointer_cast<FirstPassGroupingCollector<T>>(
        make_shared<FirstPassGroupingCollector<T>>(
            make_shared<TermGroupSelector>(groupField), groupSort, topNGroups));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private
// java.util.List<DistinctValuesCollector.GroupCount<Comparable<Object>,
// Comparable<Object>>> createExpectedResult(IndexContext context, std::wstring term,
// org.apache.lucene.search.Sort groupSort, int topN)
deque<DistinctValuesCollector::GroupCount<Comparable<any>, Comparable<any>>>
DistinctValuesCollectorTest::createExpectedResult(
    shared_ptr<IndexContext> context, const wstring &term,
    shared_ptr<Sort> groupSort, int topN)
{
  deque result = deque();
  unordered_map<wstring, Set<wstring>> groupCounts =
      context->searchTermToGroupCounts[term];
  int i = 0;
  for (auto group : groupCounts) {
    if (topN <= i++) {
      break;
    }
    shared_ptr<Set<std::shared_ptr<BytesRef>>> uniqueValues =
        unordered_set<std::shared_ptr<BytesRef>>();
    for (auto val : groupCounts[group.first]) {
      uniqueValues->add(val != L"" ? make_shared<BytesRef>(val) : nullptr);
    }
    result.push_back(make_shared<DistinctValuesCollector::GroupCount>(
        group.first != nullptr ? make_shared<BytesRef>(group.first) : nullptr,
        uniqueValues));
  }
  return result;
}

shared_ptr<IndexContext>
DistinctValuesCollectorTest::createIndexContext() 
{
  shared_ptr<Random> random = DistinctValuesCollectorTest::random();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random, dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random))
          ->setMergePolicy(newLogMergePolicy()));

  int numDocs = 86 + random->nextInt(1087) * RANDOM_MULTIPLIER;
  std::deque<wstring> groupValues(numDocs / 5);
  std::deque<wstring> countValues(numDocs / 10);
  for (int i = 0; i < groupValues.size(); i++) {
    groupValues[i] = generateRandomNonEmptyString();
  }
  for (int i = 0; i < countValues.size(); i++) {
    countValues[i] = generateRandomNonEmptyString();
  }

  deque<wstring> contentStrings = deque<wstring>();
  unordered_map<wstring, unordered_map<wstring, Set<wstring>>>
      searchTermToGroupCounts =
          unordered_map<wstring, unordered_map<wstring, Set<wstring>>>();
  for (int i = 1; i <= numDocs; i++) {
    wstring groupValue = random->nextInt(23) == 14
                             ? L""
                             : groupValues[random->nextInt(groupValues.size())];
    wstring countValue = random->nextInt(21) == 13
                             ? L""
                             : countValues[random->nextInt(countValues.size())];
    wstring content = L"random" + random->nextInt(numDocs / 20);
    unordered_map<wstring, Set<wstring>> groupToCounts =
        searchTermToGroupCounts[content];
    if (groupToCounts.empty()) {
      // Groups sort always DOCID asc...
      searchTermToGroupCounts.emplace(
          content, groupToCounts = make_shared<LinkedHashMap<>>());
      contentStrings.push_back(content);
    }

    shared_ptr<Set<wstring>> countsVals = groupToCounts[groupValue];
    if (countsVals == nullptr) {
      groupToCounts.emplace(groupValue, countsVals = unordered_set<>());
    }
    countsVals->add(countValue);

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(
        L"id", wstring::format(Locale::ROOT, L"%09d", i), Field::Store::YES));
    doc->push_back(make_shared<SortedDocValuesField>(
        L"id",
        make_shared<BytesRef>(wstring::format(Locale::ROOT, L"%09d", i))));
    if (groupValue != L"") {
      addField(doc, GROUP_FIELD, groupValue);
    }
    if (countValue != L"") {
      addField(doc, COUNT_FIELD, countValue);
    }
    doc->push_back(
        make_shared<TextField>(L"content", content, Field::Store::YES));
    w->addDocument(doc);
  }

  shared_ptr<DirectoryReader> reader = w->getReader();
  if (VERBOSE) {
    for (int docID = 0; docID < reader->maxDoc(); docID++) {
      shared_ptr<Document> doc = reader->document(docID);
      wcout << L"docID=" << docID << L" id=" << doc[L"id"] << L" content="
            << doc[L"content"] << L" author=" << doc[L"author"]
            << L" publisher=" << doc[L"publisher"] << endl;
    }
  }

  delete w;
  return make_shared<IndexContext>(
      dir, reader, searchTermToGroupCounts,
      contentStrings.toArray(std::deque<wstring>(contentStrings.size())));
}

DistinctValuesCollectorTest::IndexContext::IndexContext(
    shared_ptr<Directory> directory, shared_ptr<DirectoryReader> indexReader,
    unordered_map<wstring, unordered_map<wstring, Set<wstring>>>
        &searchTermToGroupCounts,
    std::deque<wstring> &contentStrings)
    : directory(directory), indexReader(indexReader),
      searchTermToGroupCounts(searchTermToGroupCounts),
      contentStrings(contentStrings)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings({"unchecked","rawtypes"}) public
// int compare(Comparable a, Comparable b)
int DistinctValuesCollectorTest::NullComparator::compare(Comparable a,
                                                         Comparable b)
{
  if (a == b) {
    return 0;
  } else if (a == nullptr) {
    return -1;
  } else if (b == nullptr) {
    return 1;
  } else {
    return a(b);
  }
}
} // namespace org::apache::lucene::search::grouping