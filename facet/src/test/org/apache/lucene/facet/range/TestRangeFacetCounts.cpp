using namespace std;

#include "TestRangeFacetCounts.h"

namespace org::apache::lucene::facet::range
{
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using DrillSideways = org::apache::lucene::facet::DrillSideways;
using DrillSidewaysResult =
    org::apache::lucene::facet::DrillSideways::DrillSidewaysResult;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using MultiFacets = org::apache::lucene::facet::MultiFacets;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestRangeFacetCounts::testBasicLong() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"field", 0LL);
  doc->push_back(field);
  for (int64_t l = 0; l < 100; l++) {
    field->setLongValue(l);
    w->addDocument(doc);
  }

  // Also add Long.MAX_VALUE
  field->setLongValue(numeric_limits<int64_t>::max());
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"field", fc,
      make_shared<LongRange>(L"less than 10", 0LL, true, 10LL, false),
      make_shared<LongRange>(L"less than or equal to 10", 0LL, true, 10LL,
                             true),
      make_shared<LongRange>(L"over 90", 90LL, false, 100LL, false),
      make_shared<LongRange>(L"90 or above", 90LL, true, 100LL, false),
      make_shared<LongRange>(L"over 1000", 1000LL, false,
                             numeric_limits<int64_t>::max(), true));

  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=22 childCount=5\n  less "
                         L"than 10 (10)\n  less than or equal to 10 (11)\n  "
                         L"over 90 (9)\n  90 or above (10)\n  over 1000 (1)\n",
                         result->toString());

  delete r;
  delete d;
}

void TestRangeFacetCounts::testLongGetAllDims() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"field", 0LL);
  doc->push_back(field);
  for (int64_t l = 0; l < 100; l++) {
    field->setLongValue(l);
    w->addDocument(doc);
  }

  // Also add Long.MAX_VALUE
  field->setLongValue(numeric_limits<int64_t>::max());
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"field", fc,
      make_shared<LongRange>(L"less than 10", 0LL, true, 10LL, false),
      make_shared<LongRange>(L"less than or equal to 10", 0LL, true, 10LL,
                             true),
      make_shared<LongRange>(L"over 90", 90LL, false, 100LL, false),
      make_shared<LongRange>(L"90 or above", 90LL, true, 100LL, false),
      make_shared<LongRange>(L"over 1000", 1000LL, false,
                             numeric_limits<int64_t>::max(), true));

  deque<std::shared_ptr<FacetResult>> result = facets->getAllDims(10);
  TestUtil::assertEquals(1, result.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=22 childCount=5\n  less "
                         L"than 10 (10)\n  less than or equal to 10 (11)\n  "
                         L"over 90 (9)\n  90 or above (10)\n  over 1000 (1)\n",
                         result[0]->toString());

  delete r;
  delete d;
}

void TestRangeFacetCounts::testUselessRange()
{
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<LongRange>(L"useless", 7, true, 6, true); });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<LongRange>(L"useless", 7, true, 7, false);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<DoubleRange>(L"useless", 7.0, true, 6.0, true);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<DoubleRange>(L"useless", 7.0, true, 7.0, false);
  });
}

void TestRangeFacetCounts::testLongMinMax() 
{

  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"field", 0LL);
  doc->push_back(field);
  field->setLongValue(numeric_limits<int64_t>::min());
  w->addDocument(doc);
  field->setLongValue(0);
  w->addDocument(doc);
  field->setLongValue(numeric_limits<int64_t>::max());
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"field", fc,
      make_shared<LongRange>(L"min", numeric_limits<int64_t>::min(), true,
                             numeric_limits<int64_t>::min(), true),
      make_shared<LongRange>(L"max", numeric_limits<int64_t>::max(), true,
                             numeric_limits<int64_t>::max(), true),
      make_shared<LongRange>(L"all0", numeric_limits<int64_t>::min(), true,
                             numeric_limits<int64_t>::max(), true),
      make_shared<LongRange>(L"all1", numeric_limits<int64_t>::min(), false,
                             numeric_limits<int64_t>::max(), true),
      make_shared<LongRange>(L"all2", numeric_limits<int64_t>::min(), true,
                             numeric_limits<int64_t>::max(), false),
      make_shared<LongRange>(L"all3", numeric_limits<int64_t>::min(), false,
                             numeric_limits<int64_t>::max(), false));

  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=field path=[] value=3 childCount=6\n  min (1)\n  max (1)\n  all0 "
      L"(3)\n  all1 (2)\n  all2 (2)\n  all3 (1)\n",
      result->toString());

  delete r;
  delete d;
}

void TestRangeFacetCounts::testOverlappedEndStart() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"field", 0LL);
  doc->push_back(field);
  for (int64_t l = 0; l < 100; l++) {
    field->setLongValue(l);
    w->addDocument(doc);
  }
  field->setLongValue(numeric_limits<int64_t>::max());
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"field", fc, make_shared<LongRange>(L"0-10", 0LL, true, 10LL, true),
      make_shared<LongRange>(L"10-20", 10LL, true, 20LL, true),
      make_shared<LongRange>(L"20-30", 20LL, true, 30LL, true),
      make_shared<LongRange>(L"30-40", 30LL, true, 40LL, true));

  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=41 childCount=4\n  0-10 "
                         L"(11)\n  10-20 (11)\n  20-30 (11)\n  30-40 (11)\n",
                         result->toString());

  delete r;
  delete d;
}

void TestRangeFacetCounts::testMixedRangeAndNonRangeTaxonomy() throw(
    runtime_error)
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Directory> td = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> tw = make_shared<DirectoryTaxonomyWriter>(
      td, IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  for (int64_t l = 0; l < 100; l++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // For computing range facet counts:
    doc->push_back(make_shared<NumericDocValuesField>(L"field", l));
    // For drill down by numeric range:
    doc->push_back(make_shared<LongPoint>(L"field", l));

    if ((l & 3) == 0) {
      doc->push_back(make_shared<FacetField>(L"dim", L"a"));
    } else {
      doc->push_back(make_shared<FacetField>(L"dim", L"b"));
    }
    w->addDocument(config->build(tw, doc));
  }

  shared_ptr<IndexReader> *const r = w->getReader();

  shared_ptr<TaxonomyReader> *const tr =
      make_shared<DirectoryTaxonomyReader>(tw);

  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  if (VERBOSE) {
    wcout << L"TEST: searcher=" << s << endl;
  }

  shared_ptr<DrillSideways> ds = make_shared<DrillSidewaysAnonymousInnerClass>(
      shared_from_this(), s, config);

  // First search, no drill downs:
  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  shared_ptr<DrillSideways::DrillSidewaysResult> dsr =
      ds->search(nullptr, ddq, 10);

  TestUtil::assertEquals(100, dsr->hits->totalHits);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=dim path=[] value=100 childCount=2\n  b (75)\n  a (25)\n",
      dsr->facets->getTopChildren(10, L"dim")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=21 childCount=5\n  less "
                         L"than 10 (10)\n  less than or equal to 10 (11)\n  "
                         L"over 90 (9)\n  90 or above (10)\n  over 1000 (0)\n",
                         dsr->facets->getTopChildren(10, L"field")->toString());

  // Second search, drill down on dim=b:
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"dim", {L"b"});
  dsr = ds->search(nullptr, ddq, 10);

  TestUtil::assertEquals(75, dsr->hits->totalHits);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=dim path=[] value=100 childCount=2\n  b (75)\n  a (25)\n",
      dsr->facets->getTopChildren(10, L"dim")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=16 childCount=5\n  less "
                         L"than 10 (7)\n  less than or equal to 10 (8)\n  over "
                         L"90 (7)\n  90 or above (8)\n  over 1000 (0)\n",
                         dsr->facets->getTopChildren(10, L"field")->toString());

  // Third search, drill down on "less than or equal to 10":
  ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"field", LongPoint::newRangeQuery(L"field", 0LL, 10LL));
  dsr = ds->search(nullptr, ddq, 10);

  TestUtil::assertEquals(11, dsr->hits->totalHits);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=dim path=[] value=11 childCount=2\n  b (8)\n  a (3)\n",
      dsr->facets->getTopChildren(10, L"dim")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=21 childCount=5\n  less "
                         L"than 10 (10)\n  less than or equal to 10 (11)\n  "
                         L"over 90 (9)\n  90 or above (10)\n  over 1000 (0)\n",
                         dsr->facets->getTopChildren(10, L"field")->toString());
  delete w;
  IOUtils::close({tw, tr, td, r, d});
}

TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::
    DrillSidewaysAnonymousInnerClass(
        shared_ptr<TestRangeFacetCounts> outerInstance,
        shared_ptr<IndexSearcher> s, shared_ptr<FacetsConfig> config)
    : org::apache::lucene::facet::DrillSideways(s, config, tr)
{
  this->outerInstance = outerInstance;
  this->config = config;
}

shared_ptr<Facets>
TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::buildFacetsResult(
    shared_ptr<FacetsCollector> drillDowns,
    std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
    std::deque<wstring> &drillSidewaysDims) 
{
  shared_ptr<FacetsCollector> dimFC = drillDowns;
  shared_ptr<FacetsCollector> fieldFC = drillDowns;
  if (drillSideways.size() > 0) {
    for (int i = 0; i < drillSideways.size(); i++) {
      wstring dim = drillSidewaysDims[i];
      if (dim == L"field") {
        fieldFC = drillSideways[i];
      } else {
        dimFC = drillSideways[i];
      }
    }
  }

  unordered_map<wstring, std::shared_ptr<Facets>> byDim =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  byDim.emplace(
      L"field",
      make_shared<LongRangeFacetCounts>(
          L"field", fieldFC,
          make_shared<LongRange>(L"less than 10", 0LL, true, 10LL, false),
          make_shared<LongRange>(L"less than or equal to 10", 0LL, true, 10LL,
                                 true),
          make_shared<LongRange>(L"over 90", 90LL, false, 100LL, false),
          make_shared<LongRange>(L"90 or above", 90LL, true, 100LL, false),
          make_shared<LongRange>(L"over 1000", 1000LL, false,
                                 numeric_limits<int64_t>::max(), false)));
  byDim.emplace(
      L"dim", outerInstance->getTaxonomyFacetCounts(taxoReader, config, dimFC));
  return make_shared<MultiFacets>(byDim, nullptr);
}

bool TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::
    scoreSubDocsAtOnce()
{
  return random()->nextBoolean();
}

void TestRangeFacetCounts::testBasicDouble() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<DoubleDocValuesField> field =
      make_shared<DoubleDocValuesField>(L"field", 0.0);
  doc->push_back(field);
  for (int64_t l = 0; l < 100; l++) {
    field->setDoubleValue(l);
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);
  shared_ptr<Facets> facets = make_shared<DoubleRangeFacetCounts>(
      L"field", fc,
      make_shared<DoubleRange>(L"less than 10", 0.0, true, 10.0, false),
      make_shared<DoubleRange>(L"less than or equal to 10", 0.0, true, 10.0,
                               true),
      make_shared<DoubleRange>(L"over 90", 90.0, false, 100.0, false),
      make_shared<DoubleRange>(L"90 or above", 90.0, true, 100.0, false),
      make_shared<DoubleRange>(L"over 1000", 1000.0, false,
                               numeric_limits<double>::infinity(), false));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=21 childCount=5\n  less "
                         L"than 10 (10)\n  less than or equal to 10 (11)\n  "
                         L"over 90 (9)\n  90 or above (10)\n  over 1000 (0)\n",
                         facets->getTopChildren(10, L"field")->toString());
  delete w;
  IOUtils::close({r, d});
}

void TestRangeFacetCounts::testRandomLongs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  int numDocs = atLeast(1000);
  if (VERBOSE) {
    wcout << L"TEST: numDocs=" << numDocs << endl;
  }
  std::deque<int64_t> values(numDocs);
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int64_t v = random()->nextLong();
    values[i] = v;
    doc->push_back(make_shared<NumericDocValuesField>(L"field", v));
    doc->push_back(make_shared<LongPoint>(L"field", v));
    w->addDocument(doc);
    minValue = min(minValue, v);
    maxValue = max(maxValue, v);
  }
  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  int numIters = atLeast(10);
  for (int iter = 0; iter < numIters; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    int numRange = TestUtil::nextInt(random(), 1, 100);
    std::deque<std::shared_ptr<LongRange>> ranges(numRange);
    std::deque<int> expectedCounts(numRange);
    int64_t minAcceptedValue = numeric_limits<int64_t>::max();
    int64_t maxAcceptedValue = numeric_limits<int64_t>::min();
    for (int rangeID = 0; rangeID < numRange; rangeID++) {
      int64_t min;
      if (rangeID > 0 && random()->nextInt(10) == 7) {
        // Use an existing boundary:
        shared_ptr<LongRange> prevRange = ranges[random()->nextInt(rangeID)];
        if (random()->nextBoolean()) {
          min = prevRange->min;
        } else {
          min = prevRange->max;
        }
      } else {
        min = random()->nextLong();
      }
      int64_t max;
      if (rangeID > 0 && random()->nextInt(10) == 7) {
        // Use an existing boundary:
        shared_ptr<LongRange> prevRange = ranges[random()->nextInt(rangeID)];
        if (random()->nextBoolean()) {
          max = prevRange->min;
        } else {
          max = prevRange->max;
        }
      } else {
        max = random()->nextLong();
      }

      if (min > max) {
        int64_t x = min;
        min = max;
        max = x;
      }
      bool minIncl;
      bool maxIncl;

      // NOTE: max - min >= 0 is here to handle the common overflow case!
      if (max - min >= 0 && max - min < 2) {
        // If max == min or max == min+1, we always do inclusive, else we might
        // pass an empty range and hit exc from LongRange's ctor:
        minIncl = true;
        maxIncl = true;
      } else {
        minIncl = random()->nextBoolean();
        maxIncl = random()->nextBoolean();
      }
      ranges[rangeID] = make_shared<LongRange>(L"r" + to_wstring(rangeID), min,
                                               minIncl, max, maxIncl);
      if (VERBOSE) {
        wcout << L"  range " << rangeID << L": " << ranges[rangeID] << endl;
      }

      // Do "slow but hopefully correct" computation of
      // expected count:
      for (int i = 0; i < numDocs; i++) {
        bool accept = true;
        if (minIncl) {
          accept &= values[i] >= min;
        } else {
          accept &= values[i] > min;
        }
        if (maxIncl) {
          accept &= values[i] <= max;
        } else {
          accept &= values[i] < max;
        }
        if (accept) {
          expectedCounts[rangeID]++;
          minAcceptedValue = min(minAcceptedValue, values[i]);
          maxAcceptedValue = max(maxAcceptedValue, values[i]);
        }
      }
    }

    shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
    s->search(make_shared<MatchAllDocsQuery>(), sfc);
    shared_ptr<Query> fastMatchQuery;
    if (random()->nextBoolean()) {
      if (random()->nextBoolean()) {
        fastMatchQuery = LongPoint::newRangeQuery(L"field", minValue, maxValue);
      } else {
        fastMatchQuery = LongPoint::newRangeQuery(L"field", minAcceptedValue,
                                                  maxAcceptedValue);
      }
    } else {
      fastMatchQuery.reset();
    }
    shared_ptr<LongValuesSource> vs = LongValuesSource::fromLongField(L"field");
    shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
        L"field", vs, sfc, fastMatchQuery, ranges);
    shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
    TestUtil::assertEquals(numRange, result->labelValues.size());
    for (int rangeID = 0; rangeID < numRange; rangeID++) {
      if (VERBOSE) {
        wcout << L"  range " << rangeID << L" expectedCount="
              << expectedCounts[rangeID] << endl;
      }
      shared_ptr<LabelAndValue> subNode = result->labelValues[rangeID];
      TestUtil::assertEquals(L"r" + to_wstring(rangeID), subNode->label);
      TestUtil::assertEquals(expectedCounts[rangeID],
                             subNode->value->intValue());

      shared_ptr<LongRange> range = ranges[rangeID];

      // Test drill-down:
      shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
      if (random()->nextBoolean()) {
        ddq->add(L"field",
                 LongPoint::newRangeQuery(L"field", range->min, range->max));
      } else {
        ddq->add(L"field", range->getQuery(fastMatchQuery, vs));
      }
      TestUtil::assertEquals(expectedCounts[rangeID],
                             s->search(ddq, 10)->totalHits);
    }
  }

  delete w;
  IOUtils::close({r, dir});
}

void TestRangeFacetCounts::testRandomDoubles() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  int numDocs = atLeast(1000);
  std::deque<double> values(numDocs);
  double minValue = numeric_limits<double>::infinity();
  double maxValue = -numeric_limits<double>::infinity();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    double v = random()->nextDouble();
    values[i] = v;
    doc->push_back(make_shared<DoubleDocValuesField>(L"field", v));
    doc->push_back(make_shared<DoublePoint>(L"field", v));
    w->addDocument(doc);
    minValue = min(minValue, v);
    maxValue = max(maxValue, v);
  }
  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  int numIters = atLeast(10);
  for (int iter = 0; iter < numIters; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    int numRange = TestUtil::nextInt(random(), 1, 5);
    std::deque<std::shared_ptr<DoubleRange>> ranges(numRange);
    std::deque<int> expectedCounts(numRange);
    double minAcceptedValue = numeric_limits<double>::infinity();
    double maxAcceptedValue = -numeric_limits<double>::infinity();
    for (int rangeID = 0; rangeID < numRange; rangeID++) {
      double min;
      if (rangeID > 0 && random()->nextInt(10) == 7) {
        // Use an existing boundary:
        shared_ptr<DoubleRange> prevRange = ranges[random()->nextInt(rangeID)];
        if (random()->nextBoolean()) {
          min = prevRange->min;
        } else {
          min = prevRange->max;
        }
      } else {
        min = random()->nextDouble();
      }
      double max;
      if (rangeID > 0 && random()->nextInt(10) == 7) {
        // Use an existing boundary:
        shared_ptr<DoubleRange> prevRange = ranges[random()->nextInt(rangeID)];
        if (random()->nextBoolean()) {
          max = prevRange->min;
        } else {
          max = prevRange->max;
        }
      } else {
        max = random()->nextDouble();
      }

      if (min > max) {
        double x = min;
        min = max;
        max = x;
      }

      bool minIncl;
      bool maxIncl;

      int64_t minAsLong = NumericUtils::doubleToSortableLong(min);
      int64_t maxAsLong = NumericUtils::doubleToSortableLong(max);
      // NOTE: maxAsLong - minAsLong >= 0 is here to handle the common overflow
      // case!
      if (maxAsLong - minAsLong >= 0 && maxAsLong - minAsLong < 2) {
        minIncl = true;
        maxIncl = true;
      } else {
        minIncl = random()->nextBoolean();
        maxIncl = random()->nextBoolean();
      }
      ranges[rangeID] = make_shared<DoubleRange>(L"r" + to_wstring(rangeID),
                                                 min, minIncl, max, maxIncl);

      // Do "slow but hopefully correct" computation of
      // expected count:
      for (int i = 0; i < numDocs; i++) {
        bool accept = true;
        if (minIncl) {
          accept &= values[i] >= min;
        } else {
          accept &= values[i] > min;
        }
        if (maxIncl) {
          accept &= values[i] <= max;
        } else {
          accept &= values[i] < max;
        }
        if (accept) {
          expectedCounts[rangeID]++;
          minAcceptedValue = min(minAcceptedValue, values[i]);
          maxAcceptedValue = max(maxAcceptedValue, values[i]);
        }
      }
    }

    shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
    s->search(make_shared<MatchAllDocsQuery>(), sfc);
    shared_ptr<Query> fastMatchFilter;
    if (random()->nextBoolean()) {
      if (random()->nextBoolean()) {
        fastMatchFilter =
            DoublePoint::newRangeQuery(L"field", minValue, maxValue);
      } else {
        fastMatchFilter = DoublePoint::newRangeQuery(L"field", minAcceptedValue,
                                                     maxAcceptedValue);
      }
    } else {
      fastMatchFilter.reset();
    }
    shared_ptr<DoubleValuesSource> vs =
        DoubleValuesSource::fromDoubleField(L"field");
    shared_ptr<Facets> facets = make_shared<DoubleRangeFacetCounts>(
        L"field", vs, sfc, fastMatchFilter, ranges);
    shared_ptr<FacetResult> result = facets->getTopChildren(10, L"field");
    TestUtil::assertEquals(numRange, result->labelValues.size());
    for (int rangeID = 0; rangeID < numRange; rangeID++) {
      if (VERBOSE) {
        wcout << L"  range " << rangeID << L" expectedCount="
              << expectedCounts[rangeID] << endl;
      }
      shared_ptr<LabelAndValue> subNode = result->labelValues[rangeID];
      TestUtil::assertEquals(L"r" + to_wstring(rangeID), subNode->label);
      TestUtil::assertEquals(expectedCounts[rangeID],
                             subNode->value->intValue());

      shared_ptr<DoubleRange> range = ranges[rangeID];

      // Test drill-down:
      shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
      if (random()->nextBoolean()) {
        ddq->add(L"field",
                 DoublePoint::newRangeQuery(L"field", range->min, range->max));
      } else {
        ddq->add(L"field", range->getQuery(fastMatchFilter, vs));
      }

      TestUtil::assertEquals(expectedCounts[rangeID],
                             s->search(ddq, 10)->totalHits);
    }
  }

  delete w;
  IOUtils::close({r, dir});
}

void TestRangeFacetCounts::testMissingValues() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> field =
      make_shared<NumericDocValuesField>(L"field", 0LL);
  doc->push_back(field);
  for (int64_t l = 0; l < 100; l++) {
    if (l % 5 == 0) {
      // Every 5th doc is missing the value:
      w->addDocument(make_shared<Document>());
      continue;
    }
    field->setLongValue(l);
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);
  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"field", fc,
      make_shared<LongRange>(L"less than 10", 0LL, true, 10LL, false),
      make_shared<LongRange>(L"less than or equal to 10", 0LL, true, 10LL,
                             true),
      make_shared<LongRange>(L"over 90", 90LL, false, 100LL, false),
      make_shared<LongRange>(L"90 or above", 90LL, true, 100LL, false),
      make_shared<LongRange>(L"over 1000", 1000LL, false,
                             numeric_limits<int64_t>::max(), false));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=field path=[] value=16 childCount=5\n  less "
                         L"than 10 (8)\n  less than or equal to 10 (8)\n  over "
                         L"90 (8)\n  90 or above (8)\n  over 1000 (0)\n",
                         facets->getTopChildren(10, L"field")->toString());

  delete w;
  IOUtils::close({r, d});
}

TestRangeFacetCounts::UsedQuery::UsedQuery(shared_ptr<Query> in_,
                                           shared_ptr<AtomicBoolean> used)
    : used(used), in_(in_)
{
}

bool TestRangeFacetCounts::UsedQuery::equals(any other)
{
  return sameClassAs(other) &&
         in_->equals((any_cast<std::shared_ptr<UsedQuery>>(other)).in_);
}

int TestRangeFacetCounts::UsedQuery::hashCode()
{
  return classHash() + in_->hashCode();
}

shared_ptr<Query> TestRangeFacetCounts::UsedQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const inRewritten = in_->rewrite(reader);
  if (in_ != inRewritten) {
    return make_shared<UsedQuery>(inRewritten, used);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight> TestRangeFacetCounts::UsedQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  shared_ptr<Weight> *const in_ =
      this->in_->createWeight(searcher, needsScores, boost);
  return make_shared<FilterWeightAnonymousInnerClass>(shared_from_this(), in_);
}

TestRangeFacetCounts::UsedQuery::FilterWeightAnonymousInnerClass::
    FilterWeightAnonymousInnerClass(shared_ptr<UsedQuery> outerInstance,
                                    shared_ptr<Weight> in_)
    : org::apache::lucene::search::FilterWeight(in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
}

shared_ptr<Scorer>
TestRangeFacetCounts::UsedQuery::FilterWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  outerInstance->used->set(true);
  return in_->scorer(context);
}

wstring TestRangeFacetCounts::UsedQuery::toString(const wstring &field)
{
  return L"UsedQuery(" + in_ + L")";
}

shared_ptr<DoubleValues> TestRangeFacetCounts::PlusOneValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this());
}

TestRangeFacetCounts::PlusOneValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<PlusOneValuesSource> outerInstance)
{
  this->outerInstance = outerInstance;
  doc = -1;
}

double TestRangeFacetCounts::PlusOneValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return doc + 1;
}

bool TestRangeFacetCounts::PlusOneValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  this->doc = doc;
  return true;
}

bool TestRangeFacetCounts::PlusOneValuesSource::needsScores() { return false; }

bool TestRangeFacetCounts::PlusOneValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation> TestRangeFacetCounts::PlusOneValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  return Explanation::match(docId + 1, L"");
}

shared_ptr<DoubleValuesSource>
TestRangeFacetCounts::PlusOneValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

int TestRangeFacetCounts::PlusOneValuesSource::hashCode() { return 0; }

bool TestRangeFacetCounts::PlusOneValuesSource::equals(any obj)
{
  return obj.type() == PlusOneValuesSource::typeid;
}

wstring TestRangeFacetCounts::PlusOneValuesSource::toString() { return L""; }

void TestRangeFacetCounts::testCustomDoubleValuesSource() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->addDocument(doc);

  // Test wants 3 docs in one segment:
  writer->forceMerge(1);

  shared_ptr<DoubleValuesSource> *const vs = make_shared<PlusOneValuesSource>();

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  shared_ptr<IndexReader> r = writer->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->search(make_shared<MatchAllDocsQuery>(), fc);

  std::deque<std::shared_ptr<DoubleRange>> ranges = {
      make_shared<DoubleRange>(L"< 1", 0.0, true, 1.0, false),
      make_shared<DoubleRange>(L"< 2", 0.0, true, 2.0, false),
      make_shared<DoubleRange>(L"< 5", 0.0, true, 5.0, false),
      make_shared<DoubleRange>(L"< 10", 0.0, true, 10.0, false),
      make_shared<DoubleRange>(L"< 20", 0.0, true, 20.0, false),
      make_shared<DoubleRange>(L"< 50", 0.0, true, 50.0, false)};

  shared_ptr<Query> *const fastMatchFilter;
  shared_ptr<AtomicBoolean> *const filterWasUsed = make_shared<AtomicBoolean>();
  if (random()->nextBoolean()) {
    // Sort of silly:
    shared_ptr<Query> *const in_ = make_shared<MatchAllDocsQuery>();
    fastMatchFilter = make_shared<UsedQuery>(in_, filterWasUsed);
  } else {
    fastMatchFilter.reset();
  }

  if (VERBOSE) {
    wcout << L"TEST: fastMatchFilter=" << fastMatchFilter << endl;
  }

  shared_ptr<Facets> facets = make_shared<DoubleRangeFacetCounts>(
      L"field", vs, fc, fastMatchFilter, ranges);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=field path=[] value=3 childCount=6\n  < 1 (0)\n  < 2 (1)\n  < 5 "
      L"(3)\n  < 10 (3)\n  < 20 (3)\n  < 50 (3)\n",
      facets->getTopChildren(10, L"field")->toString());
  assertTrue(fastMatchFilter == nullptr || filterWasUsed->get());

  shared_ptr<DrillDownQuery> ddq = make_shared<DrillDownQuery>(config);
  ddq->add(L"field", ranges[1]->getQuery(fastMatchFilter, vs));

  // Test simple drill-down:
  TestUtil::assertEquals(1, s->search(ddq, 10)->totalHits);

  // Test drill-sideways after drill-down
  shared_ptr<DrillSideways> ds = make_shared<DrillSidewaysAnonymousInnerClass>(
      shared_from_this(), s, config,
      std::static_pointer_cast<TaxonomyReader>(nullptr), vs, ranges,
      fastMatchFilter);

  shared_ptr<DrillSideways::DrillSidewaysResult> dsr = ds->search(ddq, 10);
  TestUtil::assertEquals(1, dsr->hits->totalHits);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=field path=[] value=3 childCount=6\n  < 1 (0)\n  < 2 (1)\n  < 5 "
      L"(3)\n  < 10 (3)\n  < 20 (3)\n  < 50 (3)\n",
      dsr->facets->getTopChildren(10, L"field")->toString());

  delete writer;
  IOUtils::close({r, dir});
}

    TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::DrillSidewaysAnonymousInnerClass(shared_ptr<TestRangeFacetCounts> outerInstance, shared_ptr<IndexSearcher> s, shared_ptr<FacetsConfig> config, shared_ptr<TaxonomyReader> org) : org->apache.lucene.facet.DrillSideways(s, config, TaxonomyReader) nullptr)
    {
      this->outerInstance = outerInstance;
      this->vs = vs;
      this->ranges = ranges;
      this->fastMatchFilter = fastMatchFilter;
    }

    shared_ptr<Facets>
    TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::buildFacetsResult(
        shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<wstring> &drillSidewaysDims) 
    {
      assert(drillSideways.size() == 1);
      return make_shared<DoubleRangeFacetCounts>(L"field", vs, drillSideways[0],
                                                 fastMatchFilter, ranges);
    }

    bool
    TestRangeFacetCounts::DrillSidewaysAnonymousInnerClass::scoreSubDocsAtOnce()
    {
      return random()->nextBoolean();
    }
    } // namespace org::apache::lucene::facet::range