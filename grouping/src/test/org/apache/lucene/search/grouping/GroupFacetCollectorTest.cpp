using namespace std;

#include "GroupFacetCollectorTest.h"

namespace org::apache::lucene::search::grouping
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

void GroupFacetCollectorTest::testSimple() 
{
  const wstring groupField = L"hotel";
  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));
  bool useDv = true;

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  addField(doc, L"airport", L"ams", useDv);
  addField(doc, L"duration", L"5", useDv);
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  addField(doc, L"airport", L"dus", useDv);
  addField(doc, L"duration", L"10", useDv);
  w->addDocument(doc);

  // 2
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  addField(doc, L"airport", L"ams", useDv);
  addField(doc, L"duration", L"10", useDv);
  w->addDocument(doc);
  w->commit(); // To ensure a second segment

  // 3
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  addField(doc, L"airport", L"ams", useDv);
  addField(doc, L"duration", L"5", useDv);
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  addField(doc, L"airport", L"ams", useDv);
  addField(doc, L"duration", L"5", useDv);
  w->addDocument(doc);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(w->getReader());

  deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>> entries;
  shared_ptr<GroupFacetCollector> groupedAirportFacetCollector;
  shared_ptr<TermGroupFacetCollector::GroupedFacetResult> airportResult;

  for (auto limit : std::deque<int>{2, 10, 100, numeric_limits<int>::max()}) {
    // any of these limits is plenty for the data we have

    groupedAirportFacetCollector =
        createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                              useDv ? L"airport_dv" : L"airport", L"", false);
    indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                          groupedAirportFacetCollector);
    int maxOffset = 5;
    airportResult = groupedAirportFacetCollector->mergeSegmentResults(
        numeric_limits<int>::max() == limit ? limit : maxOffset + limit, 0,
        false);

    TestUtil::assertEquals(3, airportResult->getTotalCount());
    TestUtil::assertEquals(0, airportResult->getTotalMissingCount());

    entries = airportResult->getFacetEntries(maxOffset, limit);
    TestUtil::assertEquals(0, entries.size());

    entries = airportResult->getFacetEntries(0, limit);
    TestUtil::assertEquals(2, entries.size());
    TestUtil::assertEquals(L"ams", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[0]->getCount());
    TestUtil::assertEquals(L"dus", entries[1]->getValue().utf8ToString());
    TestUtil::assertEquals(1, entries[1]->getCount());

    entries = airportResult->getFacetEntries(1, limit);
    TestUtil::assertEquals(1, entries.size());
    TestUtil::assertEquals(L"dus", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(1, entries[0]->getCount());
  }

  shared_ptr<GroupFacetCollector> groupedDurationFacetCollector =
      createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                            useDv ? L"duration_dv" : L"duration", L"", false);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedDurationFacetCollector);
  shared_ptr<TermGroupFacetCollector::GroupedFacetResult> durationResult =
      groupedDurationFacetCollector->mergeSegmentResults(10, 0, false);
  TestUtil::assertEquals(4, durationResult->getTotalCount());
  TestUtil::assertEquals(0, durationResult->getTotalMissingCount());

  entries = durationResult->getFacetEntries(0, 10);
  TestUtil::assertEquals(2, entries.size());
  TestUtil::assertEquals(L"10", entries[0]->getValue().utf8ToString());
  TestUtil::assertEquals(2, entries[0]->getCount());
  TestUtil::assertEquals(L"5", entries[1]->getValue().utf8ToString());
  TestUtil::assertEquals(2, entries[1]->getCount());

  // 5
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  // missing airport
  if (useDv) {
    addField(doc, L"airport", L"", useDv);
  }
  addField(doc, L"duration", L"5", useDv);
  w->addDocument(doc);

  // 6
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  addField(doc, L"airport", L"bru", useDv);
  addField(doc, L"duration", L"10", useDv);
  w->addDocument(doc);

  // 7
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  addField(doc, L"airport", L"bru", useDv);
  addField(doc, L"duration", L"15", useDv);
  w->addDocument(doc);

  // 8
  doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  addField(doc, L"airport", L"bru", useDv);
  addField(doc, L"duration", L"10", useDv);
  w->addDocument(doc);

  delete indexSearcher->getIndexReader();
  indexSearcher = newSearcher(w->getReader());
  groupedAirportFacetCollector =
      createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                            useDv ? L"airport_dv" : L"airport", L"", !useDv);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedAirportFacetCollector);
  airportResult = groupedAirportFacetCollector->mergeSegmentResults(3, 0, true);
  entries = airportResult->getFacetEntries(1, 2);
  TestUtil::assertEquals(2, entries.size());
  if (useDv) {
    TestUtil::assertEquals(6, airportResult->getTotalCount());
    TestUtil::assertEquals(0, airportResult->getTotalMissingCount());
    TestUtil::assertEquals(L"bru", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[0]->getCount());
    TestUtil::assertEquals(L"", entries[1]->getValue().utf8ToString());
    TestUtil::assertEquals(1, entries[1]->getCount());
  } else {
    TestUtil::assertEquals(5, airportResult->getTotalCount());
    TestUtil::assertEquals(1, airportResult->getTotalMissingCount());
    TestUtil::assertEquals(L"bru", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[0]->getCount());
    TestUtil::assertEquals(L"dus", entries[1]->getValue().utf8ToString());
    TestUtil::assertEquals(1, entries[1]->getCount());
  }

  groupedDurationFacetCollector =
      createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                            useDv ? L"duration_dv" : L"duration", L"", false);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedDurationFacetCollector);
  durationResult =
      groupedDurationFacetCollector->mergeSegmentResults(10, 2, true);
  TestUtil::assertEquals(5, durationResult->getTotalCount());
  TestUtil::assertEquals(0, durationResult->getTotalMissingCount());

  entries = durationResult->getFacetEntries(1, 1);
  TestUtil::assertEquals(1, entries.size());
  TestUtil::assertEquals(L"5", entries[0]->getValue().utf8ToString());
  TestUtil::assertEquals(2, entries[0]->getCount());

  // 9
  doc = make_shared<Document>();
  addField(doc, groupField, L"c", useDv);
  addField(doc, L"airport", L"bru", useDv);
  addField(doc, L"duration", L"15", useDv);
  w->addDocument(doc);

  // 10
  doc = make_shared<Document>();
  addField(doc, groupField, L"c", useDv);
  addField(doc, L"airport", L"dus", useDv);
  addField(doc, L"duration", L"10", useDv);
  w->addDocument(doc);

  delete indexSearcher->getIndexReader();
  indexSearcher = newSearcher(w->getReader());
  groupedAirportFacetCollector =
      createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                            useDv ? L"airport_dv" : L"airport", L"", false);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedAirportFacetCollector);
  airportResult =
      groupedAirportFacetCollector->mergeSegmentResults(10, 0, false);
  entries = airportResult->getFacetEntries(0, 10);
  if (useDv) {
    TestUtil::assertEquals(8, airportResult->getTotalCount());
    TestUtil::assertEquals(0, airportResult->getTotalMissingCount());
    TestUtil::assertEquals(4, entries.size());
    TestUtil::assertEquals(L"", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(1, entries[0]->getCount());
    TestUtil::assertEquals(L"ams", entries[1]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[1]->getCount());
    TestUtil::assertEquals(L"bru", entries[2]->getValue().utf8ToString());
    TestUtil::assertEquals(3, entries[2]->getCount());
    TestUtil::assertEquals(L"dus", entries[3]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[3]->getCount());
  } else {
    TestUtil::assertEquals(7, airportResult->getTotalCount());
    TestUtil::assertEquals(1, airportResult->getTotalMissingCount());
    TestUtil::assertEquals(3, entries.size());
    TestUtil::assertEquals(L"ams", entries[0]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[0]->getCount());
    TestUtil::assertEquals(L"bru", entries[1]->getValue().utf8ToString());
    TestUtil::assertEquals(3, entries[1]->getCount());
    TestUtil::assertEquals(L"dus", entries[2]->getValue().utf8ToString());
    TestUtil::assertEquals(2, entries[2]->getCount());
  }

  groupedDurationFacetCollector =
      createRandomCollector(useDv ? L"hotel_dv" : L"hotel",
                            useDv ? L"duration_dv" : L"duration", L"1", false);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedDurationFacetCollector);
  durationResult =
      groupedDurationFacetCollector->mergeSegmentResults(10, 0, true);
  TestUtil::assertEquals(5, durationResult->getTotalCount());
  TestUtil::assertEquals(0, durationResult->getTotalMissingCount());

  entries = durationResult->getFacetEntries(0, 10);
  TestUtil::assertEquals(2, entries.size());
  TestUtil::assertEquals(L"10", entries[0]->getValue().utf8ToString());
  TestUtil::assertEquals(3, entries[0]->getCount());
  TestUtil::assertEquals(L"15", entries[1]->getValue().utf8ToString());
  TestUtil::assertEquals(2, entries[1]->getCount());

  delete w;
  delete indexSearcher->getIndexReader();
  delete dir;
}

void GroupFacetCollectorTest::testMVGroupedFacetingWithDeletes() throw(
    runtime_error)
{
  const wstring groupField = L"hotel";
  shared_ptr<FieldType> customType = make_shared<FieldType>();
  customType->setStored(true);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(NoMergePolicy::INSTANCE));
  bool useDv = true;

  // Cannot assert this since we use NoMergePolicy:
  w->setDoRandomForceMergeAssert(false);

  // 0
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"x", L"x", Field::Store::NO));
  w->addDocument(doc);

  // 1
  doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"ams")));
  w->addDocument(doc);

  w->commit();
  w->deleteDocuments(
      make_shared<TermQuery>(make_shared<Term>(L"airport", L"ams")));

  // 2
  doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"ams")));
  w->addDocument(doc);

  // 3
  doc = make_shared<Document>();
  addField(doc, groupField, L"a", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"dus")));
  w->addDocument(doc);

  // 4
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"ams")));
  w->addDocument(doc);

  // 5
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"ams")));
  w->addDocument(doc);

  // 6
  doc = make_shared<Document>();
  addField(doc, groupField, L"b", useDv);
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"airport", make_shared<BytesRef>(L"ams")));
  w->addDocument(doc);
  w->commit();

  // 7
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"x", L"x", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  delete w;
  shared_ptr<IndexSearcher> indexSearcher =
      newSearcher(DirectoryReader::open(dir));
  shared_ptr<GroupFacetCollector> groupedAirportFacetCollector =
      createRandomCollector(groupField + L"_dv", L"airport", L"", true);
  indexSearcher->search(make_shared<MatchAllDocsQuery>(),
                        groupedAirportFacetCollector);
  shared_ptr<TermGroupFacetCollector::GroupedFacetResult> airportResult =
      groupedAirportFacetCollector->mergeSegmentResults(10, 0, false);
  TestUtil::assertEquals(3, airportResult->getTotalCount());
  TestUtil::assertEquals(1, airportResult->getTotalMissingCount());

  deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>> entries =
      airportResult->getFacetEntries(0, 10);
  TestUtil::assertEquals(2, entries.size());
  TestUtil::assertEquals(L"ams", entries[0]->getValue().utf8ToString());
  TestUtil::assertEquals(2, entries[0]->getCount());
  TestUtil::assertEquals(L"dus", entries[1]->getValue().utf8ToString());
  TestUtil::assertEquals(1, entries[1]->getCount());

  delete indexSearcher->getIndexReader();
  delete dir;
}

void GroupFacetCollectorTest::addField(shared_ptr<Document> doc,
                                       const wstring &field,
                                       const wstring &value, bool canUseIDV)
{
  assert(canUseIDV);
  doc->push_back(make_shared<SortedDocValuesField>(
      field + L"_dv", make_shared<BytesRef>(value)));
}

void GroupFacetCollectorTest::testRandom() 
{
  shared_ptr<Random> random = GroupFacetCollectorTest::random();
  int numberOfRuns = TestUtil::nextInt(random, 3, 6);
  for (int indexIter = 0; indexIter < numberOfRuns; indexIter++) {
    bool multipleFacetsPerDocument = random->nextBoolean();
    shared_ptr<IndexContext> context =
        createIndexContext(multipleFacetsPerDocument);
    shared_ptr<IndexSearcher> *const searcher =
        newSearcher(context->indexReader);

    if (VERBOSE) {
      wcout << L"TEST: searcher=" << searcher << endl;
    }

    for (int searchIter = 0; searchIter < 100; searchIter++) {
      if (VERBOSE) {
        wcout << L"TEST: searchIter=" << searchIter << endl;
      }
      wstring searchTerm =
          context
              ->contentStrings[random->nextInt(context->contentStrings.size())];
      int limit = random->nextInt(context->facetValues->size());
      int offset = random->nextInt(context->facetValues->size() - limit);
      int size = offset + limit;
      int minCount =
          random->nextBoolean()
              ? 0
              : random->nextInt(1 + context->facetWithMostGroups / 10);
      bool orderByCount = random->nextBoolean();
      wstring randomStr = getFromSet(
          context->facetValues, random->nextInt(context->facetValues->size()));
      const wstring facetPrefix;
      if (randomStr == L"") {
        facetPrefix = L"";
      } else {
        int codePointLen = randomStr.codePointCount(0, randomStr.length());
        int randomLen = random->nextInt(codePointLen);
        if (codePointLen == randomLen - 1) {
          facetPrefix = L"";
        } else {
          int end = randomStr.offsetByCodePoints(0, randomLen);
          facetPrefix = random->nextBoolean() ? L"" : randomStr.substr(end);
        }
      }

      shared_ptr<GroupedFacetResult> expectedFacetResult =
          createExpectedFacetResult(searchTerm, context, offset, limit,
                                    minCount, orderByCount, facetPrefix);
      shared_ptr<GroupFacetCollector> groupFacetCollector =
          createRandomCollector(L"group", L"facet", facetPrefix,
                                multipleFacetsPerDocument);
      searcher->search(
          make_shared<TermQuery>(make_shared<Term>(L"content", searchTerm)),
          groupFacetCollector);
      shared_ptr<TermGroupFacetCollector::GroupedFacetResult>
          actualFacetResult = groupFacetCollector->mergeSegmentResults(
              size, minCount, orderByCount);

      deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
          expectedFacetEntries = expectedFacetResult->getFacetEntries();
      deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
          actualFacetEntries =
              actualFacetResult->getFacetEntries(offset, limit);

      if (VERBOSE) {
        wcout << L"Collector: "
              << groupFacetCollector->getClass().getSimpleName() << endl;
        wcout << L"Num group: " << context->numGroups << endl;
        wcout << L"Num doc: " << context->numDocs << endl;
        wcout << L"Index iter: " << indexIter << endl;
        wcout << L"multipleFacetsPerDocument: " << multipleFacetsPerDocument
              << endl;
        wcout << L"Search iter: " << searchIter << endl;

        wcout << L"Search term: " << searchTerm << endl;
        wcout << L"Min count: " << minCount << endl;
        wcout << L"Facet offset: " << offset << endl;
        wcout << L"Facet limit: " << limit << endl;
        wcout << L"Facet prefix: " << facetPrefix << endl;
        wcout << L"Order by count: " << orderByCount << endl;

        wcout << L"\n=== Expected: \n" << endl;
        wcout << L"Total count " << expectedFacetResult->getTotalCount()
              << endl;
        wcout << L"Total missing count "
              << expectedFacetResult->getTotalMissingCount() << endl;
        int counter = 0;
        for (auto expectedFacetEntry : expectedFacetEntries) {
          wcout << wstring::format(
                       Locale::ROOT,
                       L"%d. Expected facet value %s with count %d", counter++,
                       expectedFacetEntry->getValue().utf8ToString(),
                       expectedFacetEntry->getCount())
                << endl;
        }

        wcout << L"\n=== Actual: \n" << endl;
        wcout << L"Total count " << actualFacetResult->getTotalCount() << endl;
        wcout << L"Total missing count "
              << actualFacetResult->getTotalMissingCount() << endl;
        counter = 0;
        for (auto actualFacetEntry : actualFacetEntries) {
          wcout << wstring::format(
                       Locale::ROOT, L"%d. Actual facet value %s with count %d",
                       counter++, actualFacetEntry->getValue().utf8ToString(),
                       actualFacetEntry->getCount())
                << endl;
        }
        wcout << L"\n=========================================================="
                 L"========================="
              << endl;
      }

      TestUtil::assertEquals(expectedFacetResult->getTotalCount(),
                             actualFacetResult->getTotalCount());
      TestUtil::assertEquals(expectedFacetResult->getTotalMissingCount(),
                             actualFacetResult->getTotalMissingCount());
      TestUtil::assertEquals(expectedFacetEntries.size(),
                             actualFacetEntries.size());
      for (int i = 0; i < expectedFacetEntries.size(); i++) {
        shared_ptr<TermGroupFacetCollector::FacetEntry> expectedFacetEntry =
            expectedFacetEntries[i];
        shared_ptr<TermGroupFacetCollector::FacetEntry> actualFacetEntry =
            actualFacetEntries[i];
        assertEquals(L"i=" + to_wstring(i) + L": " +
                         expectedFacetEntry->getValue().utf8ToString() +
                         L" != " + actualFacetEntry->getValue().utf8ToString(),
                     expectedFacetEntry->getValue(),
                     actualFacetEntry->getValue());
        assertEquals(
            L"i=" + to_wstring(i) + L": " + expectedFacetEntry->getCount() +
                L" != " + actualFacetEntry->getCount(),
            expectedFacetEntry->getCount(), actualFacetEntry->getCount());
      }
    }

    context->indexReader->close();
    delete context->dir;
  }
}

shared_ptr<IndexContext> GroupFacetCollectorTest::createIndexContext(
    bool multipleFacetValuesPerDocument) 
{
  shared_ptr<Random> *const random = GroupFacetCollectorTest::random();
  constexpr int numDocs =
      TestUtil::nextInt(random, 138, 1145) * RANDOM_MULTIPLIER;
  constexpr int numGroups = TestUtil::nextInt(random, 1, numDocs / 4);
  constexpr int numFacets = TestUtil::nextInt(random, 1, numDocs / 6);

  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << L" numGroups=" << numGroups
          << endl;
  }

  const deque<wstring> groups = deque<wstring>();
  for (int i = 0; i < numGroups; i++) {
    groups.push_back(generateRandomNonEmptyString());
  }
  const deque<wstring> facetValues = deque<wstring>();
  for (int i = 0; i < numFacets; i++) {
    facetValues.push_back(generateRandomNonEmptyString());
  }
  const std::deque<wstring> contentBrs =
      std::deque<wstring>(TestUtil::nextInt(random, 2, 20));
  if (VERBOSE) {
    wcout << L"TEST: create fake content" << endl;
  }
  for (int contentIDX = 0; contentIDX < contentBrs.size(); contentIDX++) {
    contentBrs[contentIDX] = generateRandomNonEmptyString();
    if (VERBOSE) {
      wcout << L"  content=" << contentBrs[contentIDX] << endl;
    }
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random, dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random)));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Document> docNoGroup = make_shared<Document>();
  shared_ptr<Document> docNoFacet = make_shared<Document>();
  shared_ptr<Document> docNoGroupNoFacet = make_shared<Document>();
  shared_ptr<Field> group = newStringField(L"group", L"", Field::Store::NO);
  shared_ptr<Field> groupDc =
      make_shared<SortedDocValuesField>(L"group", make_shared<BytesRef>());
  doc->push_back(groupDc);
  docNoFacet->push_back(groupDc);
  doc->push_back(group);
  docNoFacet->push_back(group);
  std::deque<std::shared_ptr<Field>> facetFields;
  if (multipleFacetValuesPerDocument == false) {
    facetFields = std::deque<std::shared_ptr<Field>>(2);
    facetFields[0] = newStringField(L"facet", L"", Field::Store::NO);
    doc->push_back(facetFields[0]);
    docNoGroup->push_back(facetFields[0]);
    facetFields[1] =
        make_shared<SortedDocValuesField>(L"facet", make_shared<BytesRef>());
    doc->push_back(facetFields[1]);
    docNoGroup->push_back(facetFields[1]);
  } else {
    facetFields =
        multipleFacetValuesPerDocument
            ? std::deque<std::shared_ptr<Field>>(2 + random->nextInt(6))
            : std::deque<std::shared_ptr<Field>>(1);
    for (int i = 0; i < facetFields.size(); i++) {
      facetFields[i] = make_shared<SortedSetDocValuesField>(
          L"facet", make_shared<BytesRef>());
      doc->push_back(facetFields[i]);
      docNoGroup->push_back(facetFields[i]);
    }
  }
  shared_ptr<Field> content = newStringField(L"content", L"", Field::Store::NO);
  doc->push_back(content);
  docNoGroup->push_back(content);
  docNoFacet->push_back(content);
  docNoGroupNoFacet->push_back(content);

  shared_ptr<NavigableSet<wstring>> uniqueFacetValues = set<wstring>(
      make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));
  unordered_map<wstring, unordered_map<wstring, Set<wstring>>>
      searchTermToFacetToGroups =
          unordered_map<wstring, unordered_map<wstring, Set<wstring>>>();
  int facetWithMostGroups = 0;
  for (int i = 0; i < numDocs; i++) {
    const wstring groupValue;
    if (random->nextInt(24) == 17) {
      // So we test the "doc doesn't have the group'd
      // field" case:
      groupValue = L"";
    } else {
      groupValue = groups[random->nextInt(groups.size())];
    }

    wstring contentStr = contentBrs[random->nextInt(contentBrs.size())];
    if (searchTermToFacetToGroups.find(contentStr) ==
        searchTermToFacetToGroups.end()) {
      searchTermToFacetToGroups.emplace(contentStr,
                                        unordered_map<wstring, Set<wstring>>());
    }
    unordered_map<wstring, Set<wstring>> facetToGroups =
        searchTermToFacetToGroups[contentStr];

    deque<wstring> facetVals = deque<wstring>();
    if (multipleFacetValuesPerDocument == false) {
      wstring facetValue = facetValues[random->nextInt(facetValues.size())];
      uniqueFacetValues->add(facetValue);
      if (facetToGroups.find(facetValue) == facetToGroups.end()) {
        facetToGroups.emplace(facetValue, unordered_set<wstring>());
      }
      shared_ptr<Set<wstring>> groupsInFacet = facetToGroups[facetValue];
      groupsInFacet->add(groupValue);
      if (groupsInFacet->size() > facetWithMostGroups) {
        facetWithMostGroups = groupsInFacet->size();
      }
      facetFields[0]->setStringValue(facetValue);
      facetFields[1]->setBytesValue(make_shared<BytesRef>(facetValue));
      facetVals.push_back(facetValue);
    } else {
      for (auto facetField : facetFields) {
        wstring facetValue = facetValues[random->nextInt(facetValues.size())];
        uniqueFacetValues->add(facetValue);
        if (facetToGroups.find(facetValue) == facetToGroups.end()) {
          facetToGroups.emplace(facetValue, unordered_set<wstring>());
        }
        shared_ptr<Set<wstring>> groupsInFacet = facetToGroups[facetValue];
        groupsInFacet->add(groupValue);
        if (groupsInFacet->size() > facetWithMostGroups) {
          facetWithMostGroups = groupsInFacet->size();
        }
        facetField->setBytesValue(make_shared<BytesRef>(facetValue));
        facetVals.push_back(facetValue);
      }
    }

    if (VERBOSE) {
      wcout << L"  doc content=" << contentStr << L" group="
            << (groupValue == L"" ? L"null" : groupValue) << L" facetVals="
            << facetVals << endl;
    }

    if (groupValue != L"") {
      groupDc->setBytesValue(make_shared<BytesRef>(groupValue));
      group->setStringValue(groupValue);
    } else {
      // TODO: not true
      // DV cannot have missing values:
      groupDc->setBytesValue(make_shared<BytesRef>());
    }
    content->setStringValue(contentStr);
    if (groupValue == L"" && facetVals.empty()) {
      writer->addDocument(docNoGroupNoFacet);
    } else if (facetVals.empty()) {
      writer->addDocument(docNoFacet);
    } else if (groupValue == L"") {
      writer->addDocument(docNoGroup);
    } else {
      writer->addDocument(doc);
    }
  }

  shared_ptr<DirectoryReader> reader = writer->getReader();
  delete writer;

  return make_shared<IndexContext>(searchTermToFacetToGroups, reader, numDocs,
                                   dir, facetWithMostGroups, numGroups,
                                   contentBrs, uniqueFacetValues);
}

GroupFacetCollectorTest::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<GroupFacetCollectorTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int GroupFacetCollectorTest::ComparatorAnonymousInnerClass::compare(
    const wstring &a, const wstring &b)
{
  if (a == b) {
    return 0;
  } else if (a == L"") {
    return -1;
  } else if (b == L"") {
    return 1;
  } else {
    return a.compare(b);
  }
}

shared_ptr<GroupedFacetResult>
GroupFacetCollectorTest::createExpectedFacetResult(
    const wstring &searchTerm, shared_ptr<IndexContext> context, int offset,
    int limit, int minCount, bool const orderByCount,
    const wstring &facetPrefix)
{
  unordered_map<wstring, Set<wstring>> facetGroups =
      context->searchTermToFacetGroups[searchTerm];
  if (facetGroups.empty()) {
    facetGroups = unordered_map<>();
  }

  int totalCount = 0;
  int totalMissCount = 0;
  shared_ptr<Set<wstring>> facetValues;
  if (facetPrefix != L"") {
    facetValues = unordered_set<>();
    for (auto facetValue : context->facetValues) {
      if (facetValue != L"" && facetValue.startsWith(facetPrefix)) {
        facetValues->add(facetValue);
      }
    }
  } else {
    facetValues = context->facetValues;
  }

  deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>> entries =
      deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>(
          facetGroups.size());
  // also includes facets with count 0
  for (auto facetValue : facetValues) {
    if (facetValue == L"") {
      continue;
    }

    shared_ptr<Set<wstring>> groups = facetGroups[facetValue];
    int count = groups != nullptr ? groups->size() : 0;
    if (count >= minCount) {
      entries.push_back(make_shared<TermGroupFacetCollector::FacetEntry>(
          make_shared<BytesRef>(facetValue), count));
    }
    totalCount += count;
  }

  // Only include null count when no facet prefix is specified
  if (facetPrefix == L"") {
    shared_ptr<Set<wstring>> groups = facetGroups[nullptr];
    if (groups != nullptr) {
      totalMissCount = groups->size();
    }
  }

  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(entries, new
  // java.util.Comparator<TermGroupFacetCollector.FacetEntry>()
  sort(entries.begin(), entries.end(),
       make_shared<ComparatorAnonymousInnerClass2>(shared_from_this(),
                                                   orderByCount));

  int endOffset = offset + limit;
  deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>> entriesResult;
  if (offset >= entries.size()) {
    entriesResult = Collections::emptyList();
  } else if (endOffset >= entries.size()) {
    entriesResult = entries.subList(offset, entries.size());
  } else {
    entriesResult = entries.subList(offset, endOffset);
  }
  return make_shared<GroupedFacetResult>(totalCount, totalMissCount,
                                         entriesResult);
}

GroupFacetCollectorTest::ComparatorAnonymousInnerClass2::
    ComparatorAnonymousInnerClass2(
        shared_ptr<GroupFacetCollectorTest> outerInstance, bool orderByCount)
{
  this->outerInstance = outerInstance;
  this->orderByCount = orderByCount;
}

int GroupFacetCollectorTest::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<TermGroupFacetCollector::FacetEntry> a,
    shared_ptr<TermGroupFacetCollector::FacetEntry> b)
{
  if (orderByCount) {
    int cmp = b->getCount() - a->getCount();
    if (cmp != 0) {
      return cmp;
    }
  }
  return a->getValue()->compareTo(b->getValue());
}

shared_ptr<GroupFacetCollector> GroupFacetCollectorTest::createRandomCollector(
    const wstring &groupField, const wstring &facetField,
    const wstring &facetPrefix, bool multipleFacetsPerDocument)
{
  shared_ptr<BytesRef> facetPrefixBR =
      facetPrefix == L"" ? nullptr : make_shared<BytesRef>(facetPrefix);
  return TermGroupFacetCollector::createTermGroupFacetCollector(
      groupField, facetField, multipleFacetsPerDocument, facetPrefixBR,
      random()->nextInt(1024));
}

wstring GroupFacetCollectorTest::getFromSet(shared_ptr<Set<wstring>> set,
                                            int index)
{
  int currentIndex = 0;
  for (auto bytesRef : set) {
    if (currentIndex++ == index) {
      return bytesRef;
    }
  }

  return L"";
}

GroupFacetCollectorTest::IndexContext::IndexContext(
    unordered_map<wstring, unordered_map<wstring, Set<wstring>>>
        &searchTermToFacetGroups,
    shared_ptr<DirectoryReader> r, int numDocs, shared_ptr<Directory> dir,
    int facetWithMostGroups, int numGroups,
    std::deque<wstring> &contentStrings,
    shared_ptr<NavigableSet<wstring>> facetValues)
    : numDocs(numDocs), indexReader(r),
      searchTermToFacetGroups(searchTermToFacetGroups),
      facetValues(facetValues), dir(dir),
      facetWithMostGroups(facetWithMostGroups), numGroups(numGroups),
      contentStrings(contentStrings)
{
}

GroupFacetCollectorTest::GroupedFacetResult::GroupedFacetResult(
    int totalCount, int totalMissingCount,
    deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>> &facetEntries)
    : totalCount(totalCount), totalMissingCount(totalMissingCount),
      facetEntries(facetEntries)
{
}

int GroupFacetCollectorTest::GroupedFacetResult::getTotalCount()
{
  return totalCount;
}

int GroupFacetCollectorTest::GroupedFacetResult::getTotalMissingCount()
{
  return totalMissingCount;
}

deque<std::shared_ptr<TermGroupFacetCollector::FacetEntry>>
GroupFacetCollectorTest::GroupedFacetResult::getFacetEntries()
{
  return facetEntries;
}
} // namespace org::apache::lucene::search::grouping