using namespace std;

#include "AllGroupHeadsCollectorTest.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void AllGroupHeadsCollectorTest::testBasic() 
{
  const wstring groupField = L"author";
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  DocValuesType valueType = DocValuesType::SORTED;

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", valueType);
  doc->push_back(newTextField(L"content", L"random text", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 1));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"1")));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", valueType);
  doc->push_back(newTextField(L"content", L"some more random text blob",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 2));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"2")));
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author1", valueType);
  doc->push_back(newTextField(L"content", L"some more random textual data",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 3));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"3")));
  w->addDocument(doc);
  w->commit(); // To ensure a second segment

  // 3
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author2", valueType);
  doc->push_back(
      newTextField(L"content", L"some random text", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 4));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"4")));
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3", valueType);
  doc->push_back(
      newTextField(L"content", L"some more random text", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 5));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"5")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  addGroupField(doc, groupField, L"author3", valueType);
  doc->push_back(newTextField(L"content", L"random blob", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 6));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"6")));
  w->addDocument(doc);

  // 6 -- no author field
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"content",
                              L"random word stuck in alot of other text",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 6));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"6")));
  w->addDocument(doc);

  // 7 -- no author field
  doc = make_shared<Document>();
  doc->push_back(newTextField(L"content",
                              L"random word stuck in alot of other text",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"id_1", 7));
  doc->push_back(
      make_shared<SortedDocValuesField>(L"id_2", make_shared<BytesRef>(L"7")));
  w->addDocument(doc);

  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> indexSearcher = newSearcher(reader);

  delete w;
  int maxDoc = reader->maxDoc();

  shared_ptr<Sort> sortWithinGroup = make_shared<Sort>(
      make_shared<SortField>(L"id_1", SortField::Type::INT, true));
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: AllGroupHeadsCollector<?> allGroupHeadsCollector =
  // createRandomCollector(groupField, sortWithinGroup);
  shared_ptr < AllGroupHeadsCollector <
      ? >> allGroupHeadsCollector =
            createRandomCollector(groupField, sortWithinGroup);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      allGroupHeadsCollector);
  assertTrue(arrayContains(std::deque<int>{2, 3, 5, 7},
                           allGroupHeadsCollector->retrieveGroupHeads()));
  assertTrue(openBitSetContains(
      std::deque<int>{2, 3, 5, 7},
      allGroupHeadsCollector->retrieveGroupHeads(maxDoc), maxDoc));

  allGroupHeadsCollector = createRandomCollector(groupField, sortWithinGroup);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"some")),
      allGroupHeadsCollector);
  assertTrue(arrayContains(std::deque<int>{2, 3, 4},
                           allGroupHeadsCollector->retrieveGroupHeads()));
  assertTrue(openBitSetContains(
      std::deque<int>{2, 3, 4},
      allGroupHeadsCollector->retrieveGroupHeads(maxDoc), maxDoc));

  allGroupHeadsCollector = createRandomCollector(groupField, sortWithinGroup);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"blob")),
      allGroupHeadsCollector);
  assertTrue(arrayContains(std::deque<int>{1, 5},
                           allGroupHeadsCollector->retrieveGroupHeads()));
  assertTrue(openBitSetContains(
      std::deque<int>{1, 5},
      allGroupHeadsCollector->retrieveGroupHeads(maxDoc), maxDoc));

  // STRING sort type triggers different implementation
  shared_ptr<Sort> sortWithinGroup2 = make_shared<Sort>(
      make_shared<SortField>(L"id_2", SortField::Type::STRING, true));
  allGroupHeadsCollector = createRandomCollector(groupField, sortWithinGroup2);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      allGroupHeadsCollector);
  assertTrue(arrayContains(std::deque<int>{2, 3, 5, 7},
                           allGroupHeadsCollector->retrieveGroupHeads()));
  assertTrue(openBitSetContains(
      std::deque<int>{2, 3, 5, 7},
      allGroupHeadsCollector->retrieveGroupHeads(maxDoc), maxDoc));

  shared_ptr<Sort> sortWithinGroup3 = make_shared<Sort>(
      make_shared<SortField>(L"id_2", SortField::Type::STRING, false));
  allGroupHeadsCollector = createRandomCollector(groupField, sortWithinGroup3);
  indexSearcher->search(
      make_shared<TermQuery>(make_shared<Term>(L"content", L"random")),
      allGroupHeadsCollector);
  // 7 b/c higher doc id wins, even if order of field is in not in reverse.
  assertTrue(arrayContains(std::deque<int>{0, 3, 4, 6},
                           allGroupHeadsCollector->retrieveGroupHeads()));
  assertTrue(openBitSetContains(
      std::deque<int>{0, 3, 4, 6},
      allGroupHeadsCollector->retrieveGroupHeads(maxDoc), maxDoc));

  delete indexSearcher->getIndexReader();
  delete dir;
}

void AllGroupHeadsCollectorTest::testRandom() 
{
  int numberOfRuns = TestUtil::nextInt(random(), 3, 6);
  for (int iter = 0; iter < numberOfRuns; iter++) {
    if (VERBOSE) {
      wcout << wstring::format(Locale::ROOT, L"TEST: iter=%d total=%d", iter,
                               numberOfRuns)
            << endl;
    }

    constexpr int numDocs =
        TestUtil::nextInt(random(), 100, 1000) * RANDOM_MULTIPLIER;
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
    DocValuesType valueType = DocValuesType::SORTED;

    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Document> docNoGroup = make_shared<Document>();
    shared_ptr<Field> valuesField = nullptr;
    valuesField =
        make_shared<SortedDocValuesField>(L"group", make_shared<BytesRef>());
    doc->push_back(valuesField);
    shared_ptr<Field> sort1 =
        make_shared<SortedDocValuesField>(L"sort1", make_shared<BytesRef>());
    doc->push_back(sort1);
    docNoGroup->push_back(sort1);
    shared_ptr<Field> sort2 =
        make_shared<SortedDocValuesField>(L"sort2", make_shared<BytesRef>());
    doc->push_back(sort2);
    docNoGroup->push_back(sort2);
    shared_ptr<Field> sort3 =
        make_shared<SortedDocValuesField>(L"sort3", make_shared<BytesRef>());
    doc->push_back(sort3);
    docNoGroup->push_back(sort3);
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
          make_shared<BytesRef>(wstring::format(Locale::ROOT, L"%05d", i)),
          contentStrings[random()->nextInt(contentStrings.size())]);

      if (VERBOSE) {
        wcout << L"  doc content=" << groupDoc->content << L" id=" << i
              << L" group="
              << (groupDoc->group == nullptr ? L"null"
                                             : groupDoc->group->utf8ToString())
              << L" sort1=" << groupDoc->sort1->utf8ToString() << L" sort2="
              << groupDoc->sort2->utf8ToString() << L" sort3="
              << groupDoc->sort3->utf8ToString() << endl;
      }

      groupDocs[i] = groupDoc;
      if (groupDoc->group != nullptr) {
        valuesField->setBytesValue(
            make_shared<BytesRef>(groupDoc->group->utf8ToString()));
      }
      sort1->setBytesValue(groupDoc->sort1);
      sort2->setBytesValue(groupDoc->sort2);
      sort3->setBytesValue(groupDoc->sort3);
      content->setStringValue(groupDoc->content);
      idDV->setLongValue(groupDoc->id);
      if (groupDoc->group == nullptr) {
        w->addDocument(docNoGroup);
      } else {
        w->addDocument(doc);
      }
    }

    shared_ptr<DirectoryReader> *const r = w->getReader();
    delete w;

    shared_ptr<NumericDocValues> values =
        MultiDocValues::getNumericValues(r, L"id");
    const std::deque<int> docIDToFieldId = std::deque<int>(numDocs);
    const std::deque<int> fieldIdToDocID = std::deque<int>(numDocs);
    for (int i = 0; i < numDocs; i++) {
      TestUtil::assertEquals(i, values->nextDoc());
      int fieldId = static_cast<int>(values->longValue());
      docIDToFieldId[i] = fieldId;
      fieldIdToDocID[fieldId] = i;
    }

    shared_ptr<IndexSearcher> *const s = newSearcher(r);

    shared_ptr<Set<int>> seenIDs = unordered_set<int>();
    for (int contentID = 0; contentID < 3; contentID++) {
      std::deque<std::shared_ptr<ScoreDoc>> hits =
          s->search(make_shared<TermQuery>(make_shared<Term>(
                        L"content", L"real" + to_wstring(contentID))),
                    numDocs)
              ->scoreDocs;
      for (auto hit : hits) {
        int idValue = docIDToFieldId[hit->doc];
        shared_ptr<GroupDoc> *const gd = groupDocs[idValue];
        TestUtil::assertEquals(gd->id, idValue);
        seenIDs->add(idValue);
        assertTrue(gd->score == 0.0);
        gd->score = hit->score;
      }
    }

    // make sure all groups were seen across the hits
    TestUtil::assertEquals(groupDocs.size(), seenIDs->size());

    // make sure scores are sane
    for (auto gd : groupDocs) {
      assertTrue(Float::isFinite(gd->score));
      assertTrue(gd->score >= 0.0);
    }

    for (int searchIter = 0; searchIter < 100; searchIter++) {

      if (VERBOSE) {
        wcout << L"TEST: searchIter=" << searchIter << endl;
      }

      const wstring searchTerm = L"real" + random()->nextInt(3);
      bool sortByScoreOnly = random()->nextBoolean();
      shared_ptr<Sort> sortWithinGroup = getRandomSort(sortByScoreOnly);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: AllGroupHeadsCollector<?> allGroupHeadsCollector =
      // createRandomCollector("group", sortWithinGroup);
      shared_ptr < AllGroupHeadsCollector <
          ? >> allGroupHeadsCollector =
                createRandomCollector(L"group", sortWithinGroup);
      s->search(
          make_shared<TermQuery>(make_shared<Term>(L"content", searchTerm)),
          allGroupHeadsCollector);
      std::deque<int> expectedGroupHeads =
          createExpectedGroupHeads(searchTerm, groupDocs, sortWithinGroup,
                                   sortByScoreOnly, fieldIdToDocID);
      std::deque<int> actualGroupHeads =
          allGroupHeadsCollector->retrieveGroupHeads();
      // The actual group heads contains Lucene ids. Need to change them into
      // our id value.
      for (int i = 0; i < actualGroupHeads.size(); i++) {
        actualGroupHeads[i] = docIDToFieldId[actualGroupHeads[i]];
      }
      // Allows us the easily iterate and assert the actual and expected
      // results.
      Arrays::sort(expectedGroupHeads);
      Arrays::sort(actualGroupHeads);

      if (VERBOSE) {
        wcout << L"Collector: "
              << allGroupHeadsCollector->getClass().getSimpleName() << endl;
        wcout << L"Sort within group: " << sortWithinGroup << endl;
        wcout << L"Num group: " << numGroups << endl;
        wcout << L"Num doc: " << numDocs << endl;
        wcout << L"\n=== Expected: \n" << endl;
        for (auto expectedDocId : expectedGroupHeads) {
          shared_ptr<GroupDoc> expectedGroupDoc = groupDocs[expectedDocId];
          wstring expectedGroup = expectedGroupDoc->group == nullptr
                                      ? L""
                                      : expectedGroupDoc->group->utf8ToString();
          wcout << wstring::format(Locale::ROOT,
                                   L"Group:%10s score%5f Sort1:%10s Sort2:%10s "
                                   L"Sort3:%10s doc:%5d",
                                   expectedGroup, expectedGroupDoc->score,
                                   expectedGroupDoc->sort1->utf8ToString(),
                                   expectedGroupDoc->sort2->utf8ToString(),
                                   expectedGroupDoc->sort3->utf8ToString(),
                                   expectedDocId)
                << endl;
        }
        wcout << L"\n=== Actual: \n" << endl;
        for (auto actualDocId : actualGroupHeads) {
          shared_ptr<GroupDoc> actualGroupDoc = groupDocs[actualDocId];
          wstring actualGroup = actualGroupDoc->group == nullptr
                                    ? L""
                                    : actualGroupDoc->group->utf8ToString();
          wcout << wstring::format(Locale::ROOT,
                                   L"Group:%10s score%5f Sort1:%10s Sort2:%10s "
                                   L"Sort3:%10s doc:%5d",
                                   actualGroup, actualGroupDoc->score,
                                   actualGroupDoc->sort1->utf8ToString(),
                                   actualGroupDoc->sort2->utf8ToString(),
                                   actualGroupDoc->sort3->utf8ToString(),
                                   actualDocId)
                << endl;
        }
        wcout << L"\n=========================================================="
                 L"========================="
              << endl;
      }

      assertArrayEquals(expectedGroupHeads, actualGroupHeads);
    }

    r->close();
    delete dir;
  }
}

bool AllGroupHeadsCollectorTest::arrayContains(std::deque<int> &expected,
                                               std::deque<int> &actual)
{
  Arrays::sort(actual); // in some cases the actual docs aren't sorted by docid.
                        // This method expects that.
  if (expected.size() != actual.size()) {
    return false;
  }

  for (auto e : expected) {
    bool found = false;
    for (auto a : actual) {
      if (e == a) {
        found = true;
        break;
      }
    }

    if (!found) {
      return false;
    }
  }

  return true;
}

bool AllGroupHeadsCollectorTest::openBitSetContains(
    std::deque<int> &expectedDocs, shared_ptr<Bits> actual,
    int maxDoc) 
{
  assert(std::dynamic_pointer_cast<FixedBitSet>(actual) != nullptr);
  if (expectedDocs.size() !=
      (std::static_pointer_cast<FixedBitSet>(actual))->cardinality()) {
    return false;
  }

  shared_ptr<FixedBitSet> expected = make_shared<FixedBitSet>(maxDoc);
  for (auto expectedDoc : expectedDocs) {
    expected->set(expectedDoc);
  }

  for (int docId = expected->nextSetBit(0);
       docId != DocIdSetIterator::NO_MORE_DOCS;
       docId = docId + 1 >= expected->length()
                   ? DocIdSetIterator::NO_MORE_DOCS
                   : expected->nextSetBit(docId + 1)) {
    if (!actual->get(docId)) {
      return false;
    }
  }

  return true;
}

std::deque<int> AllGroupHeadsCollectorTest::createExpectedGroupHeads(
    const wstring &searchTerm,
    std::deque<std::shared_ptr<GroupDoc>> &groupDocs, shared_ptr<Sort> docSort,
    bool sortByScoreOnly, std::deque<int> &fieldIdToDocID)
{
  unordered_map<std::shared_ptr<BytesRef>, deque<std::shared_ptr<GroupDoc>>>
      groupHeads = unordered_map<std::shared_ptr<BytesRef>,
                                 deque<std::shared_ptr<GroupDoc>>>();
  for (auto groupDoc : groupDocs) {
    if (!StringHelper::startsWith(groupDoc->content, searchTerm)) {
      continue;
    }

    if (groupHeads.find(groupDoc->group) == groupHeads.end()) {
      deque<std::shared_ptr<GroupDoc>> deque =
          deque<std::shared_ptr<GroupDoc>>();
      deque.push_back(groupDoc);
      groupHeads.emplace(groupDoc->group, deque);
      continue;
    }
    groupHeads[groupDoc->group].push_back(groupDoc);
  }

  std::deque<int> allGroupHeads(groupHeads.size());
  int i = 0;
  for (auto groupValue : groupHeads) {
    deque<std::shared_ptr<GroupDoc>> docs = groupHeads[groupValue->first];
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(docs, getComparator(docSort,
    // sortByScoreOnly, fieldIdToDocID));
    sort(docs.begin(), docs.end(),
         getComparator(docSort, sortByScoreOnly, fieldIdToDocID));
    allGroupHeads[i++] = docs[0]->id;
  }

  return allGroupHeads;
}

shared_ptr<Sort> AllGroupHeadsCollectorTest::getRandomSort(bool scoreOnly)
{
  const deque<std::shared_ptr<SortField>> sortFields =
      deque<std::shared_ptr<SortField>>();
  if (random()->nextInt(7) == 2 || scoreOnly) {
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
  if (random()->nextBoolean() && !scoreOnly) {
    sortFields.push_back(
        make_shared<SortField>(L"sort3", SortField::Type::STRING));
  } else if (!scoreOnly) {
    sortFields.push_back(make_shared<SortField>(L"id", SortField::Type::INT));
  }
  return make_shared<Sort>(sortFields.toArray(
      std::deque<std::shared_ptr<SortField>>(sortFields.size())));
}

shared_ptr<Comparator<std::shared_ptr<GroupDoc>>>
AllGroupHeadsCollectorTest::getComparator(shared_ptr<Sort> sort,
                                          bool const sortByScoreOnly,
                                          std::deque<int> &fieldIdToDocID)
{
  std::deque<std::shared_ptr<SortField>> sortFields = sort->getSort();
  return make_shared<ComparatorAnonymousInnerClass>(
      shared_from_this(), sortByScoreOnly, fieldIdToDocID, sortFields);
}

AllGroupHeadsCollectorTest::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<AllGroupHeadsCollectorTest> outerInstance,
        bool sortByScoreOnly, deque<int> &fieldIdToDocID,
        deque<std::shared_ptr<SortField>> &sortFields)
{
  this->outerInstance = outerInstance;
  this->sortByScoreOnly = sortByScoreOnly;
  this->fieldIdToDocID = fieldIdToDocID;
  this->sortFields = sortFields;
}

int AllGroupHeadsCollectorTest::ComparatorAnonymousInnerClass::compare(
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
        cmp = sortByScoreOnly ? fieldIdToDocID[d1->id] - fieldIdToDocID[d2->id]
                              : 0;
      }
    } else if (sf->getField() == L"sort1") {
      cmp = d1->sort1->compareTo(d2->sort1);
    } else if (sf->getField() == L"sort2") {
      cmp = d1->sort2->compareTo(d2->sort2);
    } else if (sf->getField() == L"sort3") {
      cmp = d1->sort3->compareTo(d2->sort3);
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
// AllGroupHeadsCollector<?> createRandomCollector(std::wstring groupField,
// org.apache.lucene.search.Sort sortWithinGroup) C++ TODO: Java wildcard
// generics are not converted to C++:
shared_ptr < AllGroupHeadsCollector <
    ? >> AllGroupHeadsCollectorTest::createRandomCollector(
             const wstring &groupField, shared_ptr<Sort> sortWithinGroup)
{
  if (random()->nextBoolean()) {
    shared_ptr<ValueSource> vs = make_shared<BytesRefFieldSource>(groupField);
    return AllGroupHeadsCollector::newCollector(
        make_shared<ValueSourceGroupSelector>(vs, unordered_map<>()),
        sortWithinGroup);
  } else {
    return AllGroupHeadsCollector::newCollector(
        make_shared<TermGroupSelector>(groupField), sortWithinGroup);
  }
}

void AllGroupHeadsCollectorTest::addGroupField(shared_ptr<Document> doc,
                                               const wstring &groupField,
                                               const wstring &value,
                                               DocValuesType valueType)
{
  shared_ptr<Field> valuesField = nullptr;
  switch (valueType) {
  case DocValuesType::BINARY:
    valuesField = make_shared<BinaryDocValuesField>(
        groupField, make_shared<BytesRef>(value));
    break;
  case DocValuesType::SORTED:
    valuesField = make_shared<SortedDocValuesField>(
        groupField, make_shared<BytesRef>(value));
    break;
  default:
    fail(L"unhandled type");
  }
  doc->push_back(valuesField);
}

AllGroupHeadsCollectorTest::GroupDoc::GroupDoc(int id,
                                               shared_ptr<BytesRef> group,
                                               shared_ptr<BytesRef> sort1,
                                               shared_ptr<BytesRef> sort2,
                                               shared_ptr<BytesRef> sort3,
                                               const wstring &content)
    : id(id), group(group), sort1(sort1), sort2(sort2), sort3(sort3),
      content(content)
{
}
} // namespace org::apache::lucene::search::grouping