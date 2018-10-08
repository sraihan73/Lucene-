using namespace std;

#include "TestLongValueFacetCounts.h"

namespace org::apache::lucene::facet
{
using Document = org::apache::lucene::document::Document;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestLongValueFacetCounts::testBasic() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  for (int64_t l = 0; l < 100; l++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"field", l % 5));
    w->addDocument(doc);
  }

  // Also add Long.MAX_VALUE
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(
      L"field", numeric_limits<int64_t>::max()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<LongValueFacetCounts> facets =
      make_shared<LongValueFacetCounts>(L"field", fc, false);

  shared_ptr<FacetResult> result = facets->getAllChildrenSortByValue();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(wstring(L"dim=field path=[] value=101 childCount=6\n  "
                                 L"0 (20)\n  1 (20)\n  2 (20)\n  3 (20)\n  ") +
                             L"4 (20)\n  9223372036854775807 (1)\n",
                         result->toString());
  delete r;
  delete d;
}

void TestLongValueFacetCounts::testOnlyBigLongs() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  for (int64_t l = 0; l < 3; l++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(
        L"field", numeric_limits<int64_t>::max() - l));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<LongValueFacetCounts> facets =
      make_shared<LongValueFacetCounts>(L"field", fc, false);

  shared_ptr<FacetResult> result = facets->getAllChildrenSortByValue();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      wstring(L"dim=field path=[] value=3 childCount=3\n  9223372036854775805 "
              L"(1)\n  ") +
          L"9223372036854775806 (1)\n  9223372036854775807 (1)\n",
      result->toString());
  delete r;
  delete d;
}

void TestLongValueFacetCounts::testGetAllDims() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  for (int64_t l = 0; l < 100; l++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"field", l % 5));
    w->addDocument(doc);
  }

  // Also add Long.MAX_VALUE
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(
      L"field", numeric_limits<int64_t>::max()));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets =
      make_shared<LongValueFacetCounts>(L"field", fc, false);

  deque<std::shared_ptr<FacetResult>> result = facets->getAllDims(10);
  TestUtil::assertEquals(1, result.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(wstring(L"dim=field path=[] value=101 childCount=6\n  "
                                 L"0 (20)\n  1 (20)\n  2 (20)\n  ") +
                             L"3 (20)\n  4 (20)\n  9223372036854775807 (1)\n",
                         result[0]->toString());
  delete r;
  delete d;
}

void TestLongValueFacetCounts::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  int valueCount = atLeast(1000);
  double missingChance = random()->nextDouble();
  int64_t maxValue;
  if (random()->nextBoolean()) {
    maxValue = random()->nextLong() & numeric_limits<int64_t>::max();
  } else {
    maxValue = random()->nextInt(1000);
  }
  if (VERBOSE) {
    wcout << L"TEST: valueCount=" << valueCount << L" valueRange=-" << maxValue
          << L"-" << maxValue << L" missingChance=" << missingChance << endl;
  }
  std::deque<optional<int64_t>> values(valueCount);
  int missingCount = 0;
  for (int i = 0; i < valueCount; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"id", i));
    if (random()->nextDouble() > missingChance) {
      int64_t value = TestUtil::nextLong(random(), -maxValue, maxValue);
      doc->push_back(make_shared<NumericDocValuesField>(L"field", value));
      values[i] = value;
    } else {
      missingCount++;
    }
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
      wcout << L"  test all docs" << endl;
    }

    // all docs
    unordered_map<int64_t, int> expected = unordered_map<int64_t, int>();
    int expectedChildCount = 0;
    for (int i = 0; i < valueCount; i++) {
      if (values[i]) {
        optional<int> curCount = expected[values[i]];
        if (!curCount) {
          curCount = 0;
          expectedChildCount++;
        }
        expected.emplace(values[i], curCount + 1);
      }
    }

    deque<unordered_map::Entry<int64_t, int>> expectedCounts =
        deque<unordered_map::Entry<int64_t, int>>(expected.entrySet());

    // sort by value
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    // (Long.compare(a.getKey(), b.getKey())));
    sort(expectedCounts.begin(), expectedCounts.end(),
         [&](a, b) { (Long::compare(a::getKey(), b::getKey())); });

    shared_ptr<LongValueFacetCounts> facetCounts;
    if (random()->nextBoolean()) {
      s->search(make_shared<MatchAllDocsQuery>(), fc);
      if (random()->nextBoolean()) {
        if (VERBOSE) {
          wcout << L"  use value source" << endl;
        }
        facetCounts = make_shared<LongValueFacetCounts>(
            L"field", LongValuesSource::fromLongField(L"field"), fc);
      } else {
        if (VERBOSE) {
          wcout << L"  use doc values" << endl;
        }
        facetCounts = make_shared<LongValueFacetCounts>(L"field", fc, false);
      }
    } else {
      // optimized count all:
      if (random()->nextBoolean()) {
        if (VERBOSE) {
          wcout << L"  count all value source" << endl;
        }
        facetCounts = make_shared<LongValueFacetCounts>(
            L"field", LongValuesSource::fromLongField(L"field"), r);
      } else {
        if (VERBOSE) {
          wcout << L"  count all doc values" << endl;
        }
        facetCounts = make_shared<LongValueFacetCounts>(L"field", r, false);
      }
    }

    shared_ptr<FacetResult> actual = facetCounts->getAllChildrenSortByValue();
    assertSame(L"all docs, sort facets by value", expectedCounts,
               expectedChildCount, valueCount - missingCount, actual,
               numeric_limits<int>::max());

    // sort by count
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    sort(expectedCounts.begin(), expectedCounts.end(), [&](a, b) {
      int cmp = -Integer::compare(a::getValue(), b::getValue());
      if (cmp == 0) {
        cmp = Long::compare(a::getKey(), b::getKey());
      }
      return cmp;
    });
    int topN;
    if (random()->nextBoolean()) {
      topN = valueCount;
    } else {
      topN = random()->nextInt(valueCount);
    }
    if (VERBOSE) {
      wcout << L"  topN=" << topN << endl;
    }
    actual = facetCounts->getTopChildrenSortByCount(topN);
    assertSame(L"all docs, sort facets by count", expectedCounts,
               expectedChildCount, valueCount - missingCount, actual, topN);

    // subset of docs
    int minId = random()->nextInt(valueCount);
    int maxId = random()->nextInt(valueCount);
    if (minId > maxId) {
      int tmp = minId;
      minId = maxId;
      maxId = tmp;
    }
    if (VERBOSE) {
      wcout << L"  test id range " << minId << L"-" << maxId << endl;
    }

    fc = make_shared<FacetsCollector>();
    s->search(IntPoint::newRangeQuery(L"id", minId, maxId), fc);
    if (random()->nextBoolean()) {
      if (VERBOSE) {
        wcout << L"  use doc values" << endl;
      }
      facetCounts = make_shared<LongValueFacetCounts>(L"field", fc, false);
    } else {
      if (VERBOSE) {
        wcout << L"  use value source" << endl;
      }
      facetCounts = make_shared<LongValueFacetCounts>(
          L"field", LongValuesSource::fromLongField(L"field"), fc);
    }

    expected = unordered_map<>();
    expectedChildCount = 0;
    int totCount = 0;
    for (int i = minId; i <= maxId; i++) {
      if (values[i]) {
        totCount++;
        optional<int> curCount = expected[values[i]];
        if (!curCount) {
          expectedChildCount++;
          curCount = 0;
        }
        expected.emplace(values[i], curCount + 1);
      }
    }
    expectedCounts = deque<>(expected.entrySet());

    // sort by value
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    // (Long.compare(a.getKey(), b.getKey())));
    sort(expectedCounts.begin(), expectedCounts.end(),
         [&](a, b) { (Long::compare(a::getKey(), b::getKey())); });
    actual = facetCounts->getAllChildrenSortByValue();
    assertSame(L"id " + to_wstring(minId) + L"-" + to_wstring(maxId) +
                   L", sort facets by value",
               expectedCounts, expectedChildCount, totCount, actual,
               numeric_limits<int>::max());

    // sort by count
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    sort(expectedCounts.begin(), expectedCounts.end(), [&](a, b) {
      int cmp = -Integer::compare(a::getValue(), b::getValue());
      if (cmp == 0) {
        cmp = Long::compare(a::getKey(), b::getKey());
      }
      return cmp;
    });
    if (random()->nextBoolean()) {
      topN = valueCount;
    } else {
      topN = random()->nextInt(valueCount);
    }
    actual = facetCounts->getTopChildrenSortByCount(topN);
    assertSame(L"id " + to_wstring(minId) + L"-" + to_wstring(maxId) +
                   L", sort facets by count",
               expectedCounts, expectedChildCount, totCount, actual, topN);
  }
  delete r;
  delete dir;
}

void TestLongValueFacetCounts::testRandomMultiValued() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  int valueCount = atLeast(1000);
  double missingChance = random()->nextDouble();

  // sometimes exercise codec optimizations when a claimed multi valued field is
  // in fact single valued:
  bool allSingleValued = rarely();
  int64_t maxValue;

  if (random()->nextBoolean()) {
    maxValue = random()->nextLong() & numeric_limits<int64_t>::max();
  } else {
    maxValue = random()->nextInt(1000);
  }
  if (VERBOSE) {
    wcout << L"TEST: valueCount=" << valueCount << L" valueRange=-" << maxValue
          << L"-" << maxValue << L" missingChance=" << missingChance
          << L" allSingleValued=" << allSingleValued << endl;
  }

  std::deque<std::deque<int64_t>> values(valueCount);
  int missingCount = 0;
  for (int i = 0; i < valueCount; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<IntPoint>(L"id", i));
    if (random()->nextDouble() > missingChance) {
      if (allSingleValued) {
        values[i] = std::deque<int64_t>(1);
      } else {
        values[i] = std::deque<int64_t>(TestUtil::nextInt(random(), 1, 5));
      }

      for (int j = 0; j < values[i].size(); j++) {
        int64_t value = TestUtil::nextLong(random(), -maxValue, maxValue);
        values[i][j] = value;
        doc->push_back(
            make_shared<SortedNumericDocValuesField>(L"field", value));
      }

      if (VERBOSE) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"  doc=" << i << L" values=" << Arrays->toString(values[i])
              << endl;
      }

    } else {
      missingCount++;

      if (VERBOSE) {
        wcout << L"  doc=" << i << L" missing values" << endl;
      }
    }
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
      wcout << L"  test all docs" << endl;
    }

    // all docs
    unordered_map<int64_t, int> expected = unordered_map<int64_t, int>();
    int expectedChildCount = 0;
    int expectedTotalCount = 0;
    for (int i = 0; i < valueCount; i++) {
      if (values[i].size() > 0) {
        for (auto value : values[i]) {
          optional<int> curCount = expected[value];
          if (!curCount) {
            curCount = 0;
            expectedChildCount++;
          }
          expected.emplace(value, curCount + 1);
          expectedTotalCount++;
        }
      }
    }

    deque<unordered_map::Entry<int64_t, int>> expectedCounts =
        deque<unordered_map::Entry<int64_t, int>>(expected.entrySet());

    // sort by value
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    // (Long.compare(a.getKey(), b.getKey())));
    sort(expectedCounts.begin(), expectedCounts.end(),
         [&](a, b) { (Long::compare(a::getKey(), b::getKey())); });

    shared_ptr<LongValueFacetCounts> facetCounts;
    if (random()->nextBoolean()) {
      s->search(make_shared<MatchAllDocsQuery>(), fc);
      if (VERBOSE) {
        wcout << L"  use doc values" << endl;
      }
      facetCounts = make_shared<LongValueFacetCounts>(L"field", fc, true);
    } else {
      // optimized count all:
      if (VERBOSE) {
        wcout << L"  count all doc values" << endl;
      }
      facetCounts = make_shared<LongValueFacetCounts>(L"field", r, true);
    }

    shared_ptr<FacetResult> actual = facetCounts->getAllChildrenSortByValue();
    assertSame(L"all docs, sort facets by value", expectedCounts,
               expectedChildCount, expectedTotalCount, actual,
               numeric_limits<int>::max());

    // sort by count
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    sort(expectedCounts.begin(), expectedCounts.end(), [&](a, b) {
      int cmp = -Integer::compare(a::getValue(), b::getValue());
      if (cmp == 0) {
        cmp = Long::compare(a::getKey(), b::getKey());
      }
      return cmp;
    });
    int topN;
    if (random()->nextBoolean()) {
      topN = valueCount;
    } else {
      topN = random()->nextInt(valueCount);
    }
    if (VERBOSE) {
      wcout << L"  topN=" << topN << endl;
    }
    actual = facetCounts->getTopChildrenSortByCount(topN);
    assertSame(L"all docs, sort facets by count", expectedCounts,
               expectedChildCount, expectedTotalCount, actual, topN);

    // subset of docs
    int minId = random()->nextInt(valueCount);
    int maxId = random()->nextInt(valueCount);
    if (minId > maxId) {
      int tmp = minId;
      minId = maxId;
      maxId = tmp;
    }
    if (VERBOSE) {
      wcout << L"  test id range " << minId << L"-" << maxId << endl;
    }

    fc = make_shared<FacetsCollector>();
    s->search(IntPoint::newRangeQuery(L"id", minId, maxId), fc);
    // cannot use value source here because we are multi valued
    facetCounts = make_shared<LongValueFacetCounts>(L"field", fc, true);

    expected = unordered_map<>();
    expectedChildCount = 0;
    int totCount = 0;
    for (int i = minId; i <= maxId; i++) {
      if (values[i].size() > 0) {
        for (auto value : values[i]) {
          totCount++;
          optional<int> curCount = expected[value];
          if (!curCount) {
            expectedChildCount++;
            curCount = 0;
          }
          expected.emplace(value, curCount + 1);
        }
      }
    }
    expectedCounts = deque<>(expected.entrySet());

    // sort by value
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    // (Long.compare(a.getKey(), b.getKey())));
    sort(expectedCounts.begin(), expectedCounts.end(),
         [&](a, b) { (Long::compare(a::getKey(), b::getKey())); });
    actual = facetCounts->getAllChildrenSortByValue();
    assertSame(L"id " + to_wstring(minId) + L"-" + to_wstring(maxId) +
                   L", sort facets by value",
               expectedCounts, expectedChildCount, totCount, actual,
               numeric_limits<int>::max());

    // sort by count
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(expectedCounts, (a, b) ->
    sort(expectedCounts.begin(), expectedCounts.end(), [&](a, b) {
      int cmp = -Integer::compare(a::getValue(), b::getValue());
      if (cmp == 0) {
        cmp = Long::compare(a::getKey(), b::getKey());
      }
      return cmp;
    });
    if (random()->nextBoolean()) {
      topN = valueCount;
    } else {
      topN = random()->nextInt(valueCount);
    }
    actual = facetCounts->getTopChildrenSortByCount(topN);
    assertSame(L"id " + to_wstring(minId) + L"-" + to_wstring(maxId) +
                   L", sort facets by count",
               expectedCounts, expectedChildCount, totCount, actual, topN);
  }
  delete r;
  delete dir;
}

void TestLongValueFacetCounts::assertSame(
    const wstring &desc,
    deque<unordered_map::Entry<int64_t, int>> &expectedCounts,
    int expectedChildCount, int expectedTotalCount,
    shared_ptr<FacetResult> actual, int topN)
{
  int expectedTopN = min(topN, expectedCounts.size());
  if (VERBOSE) {
    wcout << L"  expected topN=" << expectedTopN << endl;
    for (int i = 0; i < expectedTopN; i++) {
      wcout << L"    " << i << L": value=" << expectedCounts[i].getKey()
            << L" count=" << expectedCounts[i].getValue() << endl;
    }
    wcout << L"  actual topN=" << actual->labelValues.size() << endl;
    for (int i = 0; i < actual->labelValues.size(); i++) {
      wcout << L"    " << i << L": value=" << actual->labelValues[i]->label
            << L" count=" << actual->labelValues[i]->value << endl;
    }
  }
  assertEquals(desc + L": topN", expectedTopN, actual->labelValues.size());
  assertEquals(desc + L": childCount", expectedChildCount, actual->childCount);
  assertEquals(desc + L": totCount", expectedTotalCount,
               actual->value->intValue());
  assertTrue(actual->labelValues.size() <= topN);

  for (int i = 0; i < expectedTopN; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(desc + L": label[" + to_wstring(i) + L"]",
                 Long::toString(expectedCounts[i].getKey()),
                 actual->labelValues[i]->label);
    assertEquals(desc + L": counts[" + to_wstring(i) + L"]",
                 expectedCounts[i].getValue().intValue(),
                 actual->labelValues[i]->value.intValue());
  }
}
} // namespace org::apache::lucene::facet