using namespace std;

#include "BaseGeoPointTestCase.h"

namespace org::apache::lucene::geo
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using Lucene60PointsReader =
    org::apache::lucene::codecs::lucene60::Lucene60PointsReader;
using Lucene60PointsWriter =
    org::apache::lucene::codecs::lucene60::Lucene60PointsWriter;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using MultiFields = org::apache::lucene::index::MultiFields;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using SloppyMath = org::apache::lucene::util::SloppyMath;
using TestUtil = org::apache::lucene::util::TestUtil;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;
const wstring BaseGeoPointTestCase::FIELD_NAME = L"point";

double BaseGeoPointTestCase::nextLongitude()
{
  return org::apache::lucene::geo::GeoTestUtil::nextLongitude();
}

double BaseGeoPointTestCase::nextLatitude()
{
  return org::apache::lucene::geo::GeoTestUtil::nextLatitude();
}

shared_ptr<Rectangle> BaseGeoPointTestCase::nextBox()
{
  return org::apache::lucene::geo::GeoTestUtil::nextBox();
}

shared_ptr<Polygon> BaseGeoPointTestCase::nextPolygon()
{
  return org::apache::lucene::geo::GeoTestUtil::nextPolygon();
}

bool BaseGeoPointTestCase::supportsPolygons() { return true; }

void BaseGeoPointTestCase::testIndexExtremeValues()
{
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"foo", document, 90.0, 180.0);
  addPointToDoc(L"foo", document, 90.0, -180.0);
  addPointToDoc(L"foo", document, -90.0, 180.0);
  addPointToDoc(L"foo", document, -90.0, -180.0);
}

void BaseGeoPointTestCase::testIndexOutOfRangeValues()
{
  shared_ptr<Document> document = make_shared<Document>();
  invalid_argument expected;

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, Math::nextUp(90.0), 50.0);
  });
  assertTrue(expected.what()->contains(L"invalid latitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, Math::nextDown(-90.0), 50.0);
  });
  assertTrue(expected.what()->contains(L"invalid latitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, 90.0, Math::nextUp(180.0));
  });
  assertTrue(expected.what()->contains(L"invalid longitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, 90.0, Math::nextDown(-180.0));
  });
  assertTrue(expected.what()->contains(L"invalid longitude"));
}

void BaseGeoPointTestCase::testIndexNaNValues()
{
  shared_ptr<Document> document = make_shared<Document>();
  invalid_argument expected;

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, NAN, 50.0);
  });
  assertTrue(expected.what()->contains(L"invalid latitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, 50.0, NAN);
  });
  assertTrue(expected.what()->contains(L"invalid longitude"));
}

void BaseGeoPointTestCase::testIndexInfValues()
{
  shared_ptr<Document> document = make_shared<Document>();
  invalid_argument expected;

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, numeric_limits<double>::infinity(), 50.0);
  });
  assertTrue(expected.what()->contains(L"invalid latitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, -numeric_limits<double>::infinity(), 50.0);
  });
  assertTrue(expected.what()->contains(L"invalid latitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, 50.0, numeric_limits<double>::infinity());
  });
  assertTrue(expected.what()->contains(L"invalid longitude"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    addPointToDoc(L"foo", document, 50.0, -numeric_limits<double>::infinity());
  });
  assertTrue(expected.what()->contains(L"invalid longitude"));
}

void BaseGeoPointTestCase::testBoxBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a point
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(
      1, searcher->count(newRectQuery(L"field", 18, 19, -66, -65)));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testBoxNull()
{
  invalid_argument expected = expectThrows(
      invalid_argument::typeid, [&]() { newRectQuery(L"", 18, 19, -66, -65); });
  assertTrue(expected.what()->contains(L"field must not be null"));
}

void BaseGeoPointTestCase::testBoxInvalidCoordinates() 
{
  expectThrows(runtime_error::typeid,
               [&]() { newRectQuery(L"field", -92.0, -91.0, 179.0, 181.0); });
}

void BaseGeoPointTestCase::testDistanceBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a location
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search within 50km and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(
      1, searcher->count(newDistanceQuery(L"field", 18, -65, 50'000)));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testDistanceNull()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    newDistanceQuery(L"", 18, -65, 50'000);
  });
  assertTrue(expected.what()->contains(L"field must not be null"));
}

void BaseGeoPointTestCase::testDistanceIllegal() 
{
  expectThrows(runtime_error::typeid,
               [&]() { newDistanceQuery(L"field", 92.0, 181.0, 120000); });
}

void BaseGeoPointTestCase::testDistanceNegative()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    newDistanceQuery(L"field", 18, 19, -1);
  });
  assertTrue(expected.what()->contains(L"radiusMeters"));
  assertTrue(expected.what()->contains(L"invalid"));
}

void BaseGeoPointTestCase::testDistanceNaN()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    newDistanceQuery(L"field", 18, 19, NAN);
  });
  assertTrue(expected.what()->contains(L"radiusMeters"));
  assertTrue(expected.what()->contains(L"invalid"));
}

void BaseGeoPointTestCase::testDistanceInf()
{
  invalid_argument expected;

  expected = expectThrows(invalid_argument::typeid, [&]() {
    newDistanceQuery(L"field", 18, 19, numeric_limits<double>::infinity());
  });
  assertTrue(expected.what()->contains(L"radiusMeters"));
  assertTrue(expected.what()->contains(L"invalid"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    newDistanceQuery(L"field", 18, 19, -numeric_limits<double>::infinity());
  });
  assertTrue(expected.what(), expected.what()->contains(L"radiusMeters"));
  assertTrue(expected.what()->contains(L"invalid"));
}

void BaseGeoPointTestCase::testPolygonBasics() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a point
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(
      1, searcher->count(newPolygonQuery(
             L"field", {make_shared<Polygon>(
                           std::deque<double>{18, 18, 19, 19, 18},
                           std::deque<double>{-66, -65, -65, -66, -66})})));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testPolygonHole() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a point
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Polygon> inner = make_shared<Polygon>(
      std::deque<double>{18.5, 18.5, 18.7, 18.7, 18.5},
      std::deque<double>{-65.7, -65.4, -65.4, -65.7, -65.7});
  shared_ptr<Polygon> outer =
      make_shared<Polygon>(std::deque<double>{18, 18, 19, 19, 18},
                           std::deque<double>{-66, -65, -65, -66, -66}, inner);
  TestUtil::assertEquals(1,
                         searcher->count(newPolygonQuery(L"field", {outer})));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testPolygonHoleExcludes() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a point
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Polygon> inner = make_shared<Polygon>(
      std::deque<double>{18.2, 18.2, 18.4, 18.4, 18.2},
      std::deque<double>{-65.3, -65.2, -65.2, -65.3, -65.3});
  shared_ptr<Polygon> outer =
      make_shared<Polygon>(std::deque<double>{18, 18, 19, 19, 18},
                           std::deque<double>{-66, -65, -65, -66, -66}, inner);
  TestUtil::assertEquals(0,
                         searcher->count(newPolygonQuery(L"field", {outer})));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testMultiPolygonBasics() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // add a doc with a point
  shared_ptr<Document> document = make_shared<Document>();
  addPointToDoc(L"field", document, 18.313694, -65.227444);
  writer->addDocument(document);

  // search and verify we found our doc
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<Polygon> a =
      make_shared<Polygon>(std::deque<double>{28, 28, 29, 29, 28},
                           std::deque<double>{-56, -55, -55, -56, -56});
  shared_ptr<Polygon> b =
      make_shared<Polygon>(std::deque<double>{18, 18, 19, 19, 18},
                           std::deque<double>{-66, -65, -65, -66, -66});
  TestUtil::assertEquals(1, searcher->count(newPolygonQuery(L"field", {a, b})));

  delete reader;
  delete writer;
  delete dir;
}

void BaseGeoPointTestCase::testPolygonNullField()
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    newPolygonQuery(L"", {make_shared<Polygon>(
                             std::deque<double>{18, 18, 19, 19, 18},
                             std::deque<double>{-66, -65, -65, -66, -66})});
  });
  assertTrue(expected.what()->contains(L"field must not be null"));
}

void BaseGeoPointTestCase::testSamePointManyTimes() 
{
  int numPoints = atLeast(1000);

  // Every doc has 2 points:
  double theLat = nextLatitude();
  double theLon = nextLongitude();

  std::deque<double> lats(numPoints);
  Arrays::fill(lats, theLat);

  std::deque<double> lons(numPoints);
  Arrays::fill(lons, theLon);

  verify(lats, lons);
}

void BaseGeoPointTestCase::testAllLatEqual() 
{
  int numPoints = atLeast(10000);
  double lat = nextLatitude();
  std::deque<double> lats(numPoints);
  std::deque<double> lons(numPoints);

  bool haveRealDoc = false;

  for (int docID = 0; docID < numPoints; docID++) {
    int x = random()->nextInt(20);
    if (x == 17) {
      // Some docs don't have a point:
      lats[docID] = NAN;
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" is missing" << endl;
      }
      continue;
    }

    if (docID > 0 && x == 14 && haveRealDoc) {
      int oldDocID;
      while (true) {
        oldDocID = random()->nextInt(docID);
        if (isnan(lats[oldDocID]) == false) {
          break;
        }
      }

      // Fully identical point:
      lons[docID] = lons[oldDocID];
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" lat=" << lat << L" lon="
              << lons[docID] << L" (same lat/lon as doc=" << oldDocID << L")"
              << endl;
      }
    } else {
      lons[docID] = nextLongitude();
      haveRealDoc = true;
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" lat=" << lat << L" lon="
              << lons[docID] << endl;
      }
    }
    lats[docID] = lat;
  }

  verify(lats, lons);
}

void BaseGeoPointTestCase::testAllLonEqual() 
{
  int numPoints = atLeast(10000);
  double theLon = nextLongitude();
  std::deque<double> lats(numPoints);
  std::deque<double> lons(numPoints);

  bool haveRealDoc = false;

  // System.out.println("theLon=" + theLon);

  for (int docID = 0; docID < numPoints; docID++) {
    int x = random()->nextInt(20);
    if (x == 17) {
      // Some docs don't have a point:
      lats[docID] = NAN;
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" is missing" << endl;
      }
      continue;
    }

    if (docID > 0 && x == 14 && haveRealDoc) {
      int oldDocID;
      while (true) {
        oldDocID = random()->nextInt(docID);
        if (isnan(lats[oldDocID]) == false) {
          break;
        }
      }

      // Fully identical point:
      lats[docID] = lats[oldDocID];
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" lat=" << lats[docID] << L" lon="
              << theLon << L" (same lat/lon as doc=" << oldDocID << L")"
              << endl;
      }
    } else {
      lats[docID] = nextLatitude();
      haveRealDoc = true;
      if (VERBOSE) {
        wcout << L"  doc=" << docID << L" lat=" << lats[docID] << L" lon="
              << theLon << endl;
      }
    }
    lons[docID] = theLon;
  }

  verify(lats, lons);
}

void BaseGeoPointTestCase::testMultiValued() 
{
  int numPoints = atLeast(10000);
  // Every doc has 2 points:
  std::deque<double> lats(2 * numPoints);
  std::deque<double> lons(2 * numPoints);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();

  // We rely on docID order:
  iwc->setMergePolicy(newLogMergePolicy());
  // and on seeds being able to reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  for (int id = 0; id < numPoints; id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    lats[2 * id] = quantizeLat(nextLatitude());
    lons[2 * id] = quantizeLon(nextLongitude());
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::YES));
    addPointToDoc(FIELD_NAME, doc, lats[2 * id], lons[2 * id]);
    lats[2 * id + 1] = quantizeLat(nextLatitude());
    lons[2 * id + 1] = quantizeLon(nextLongitude());
    addPointToDoc(FIELD_NAME, doc, lats[2 * id + 1], lons[2 * id + 1]);

    if (VERBOSE) {
      wcout << L"id=" << id << endl;
      wcout << L"  lat=" << lats[2 * id] << L" lon=" << lons[2 * id] << endl;
      wcout << L"  lat=" << lats[2 * id << 1] << L" lon=" << lons[2 * id << 1]
            << endl;
    }
    w->addDocument(doc);
  }

  // TODO: share w/ verify; just need parallel array of the expected ids
  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(25);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<Rectangle> rect = nextBox();

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" rect=" << rect << endl;
    }

    shared_ptr<Query> query = newRectQuery(
        FIELD_NAME, rect->minLat, rect->maxLat, rect->minLon, rect->maxLon);

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(r->maxDoc());
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(
                         shared_from_this(), hits));

    bool fail = false;

    for (int docID = 0; docID < lats.size() / 2; docID++) {
      double latDoc1 = lats[2 * docID];
      double lonDoc1 = lons[2 * docID];
      double latDoc2 = lats[2 * docID + 1];
      double lonDoc2 = lons[2 * docID + 1];

      bool result1 = rectContainsPoint(rect, latDoc1, lonDoc1);
      bool result2 = rectContainsPoint(rect, latDoc2, lonDoc2);

      bool expected = result1 || result2;

      if (hits->get(docID) != expected) {
        wstring id = s->doc(docID)[L"id"];
        if (expected) {
          wcout << L"TEST: id=" << id << L" docID=" << docID
                << L" should match but did not" << endl;
        } else {
          wcout << L"TEST: id=" << id << L" docID=" << docID
                << L" should not match but did" << endl;
        }
        wcout << L"  rect=" << rect << endl;
        wcout << L"  lat=" << latDoc1 << L" lon=" << lonDoc1 << L"\n  lat="
              << latDoc2 << L" lon=" << lonDoc2 << endl;
        wcout << L"  result1=" << result1 << L" result2=" << result2 << endl;
        fail = true;
      }
    }

    if (fail) {
      fail(L"some hits were wrong");
    }
  }
  delete r;
  delete dir;
}

BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<BaseGeoPointTestCase> outerInstance,
        shared_ptr<FixedBitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  hits->set(docBase + doc);
}

void BaseGeoPointTestCase::testRandomTiny() 
{
  // Make sure single-leaf-node case is OK:
  doTestRandom(10);
}

void BaseGeoPointTestCase::testRandomMedium() 
{
  doTestRandom(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
void BaseGeoPointTestCase::testRandomBig() 
{
  assumeFalse(L"Direct codec can OOME on this test",
              TestUtil::getDocValuesFormat(FIELD_NAME) == L"Direct");
  assumeFalse(L"Memory codec can OOME on this test",
              TestUtil::getDocValuesFormat(FIELD_NAME) == L"Memory");
  doTestRandom(200000);
}

void BaseGeoPointTestCase::doTestRandom(int count) 
{

  int numPoints = atLeast(count);

  if (VERBOSE) {
    wcout << L"TEST: numPoints=" << numPoints << endl;
  }

  std::deque<double> lats(numPoints);
  std::deque<double> lons(numPoints);

  bool haveRealDoc = false;

  for (int id = 0; id < numPoints; id++) {
    int x = random()->nextInt(20);
    if (x == 17) {
      // Some docs don't have a point:
      lats[id] = NAN;
      if (VERBOSE) {
        wcout << L"  id=" << id << L" is missing" << endl;
      }
      continue;
    }

    if (id > 0 && x < 3 && haveRealDoc) {
      int oldID;
      while (true) {
        oldID = random()->nextInt(id);
        if (isnan(lats[oldID]) == false) {
          break;
        }
      }

      if (x == 0) {
        // Identical lat to old point
        lats[id] = lats[oldID];
        lons[id] = nextLongitude();
        if (VERBOSE) {
          wcout << L"  id=" << id << L" lat=" << lats[id] << L" lon="
                << lons[id] << L" (same lat as doc=" << oldID << L")" << endl;
        }
      } else if (x == 1) {
        // Identical lon to old point
        lats[id] = nextLatitude();
        lons[id] = lons[oldID];
        if (VERBOSE) {
          wcout << L"  id=" << id << L" lat=" << lats[id] << L" lon="
                << lons[id] << L" (same lon as doc=" << oldID << L")" << endl;
        }
      } else {
        assert(x == 2);
        // Fully identical point:
        lats[id] = lats[oldID];
        lons[id] = lons[oldID];
        if (VERBOSE) {
          wcout << L"  id=" << id << L" lat=" << lats[id] << L" lon="
                << lons[id] << L" (same lat/lon as doc=" << oldID << L")"
                << endl;
        }
      }
    } else {
      lats[id] = nextLatitude();
      lons[id] = nextLongitude();
      haveRealDoc = true;
      if (VERBOSE) {
        wcout << L"  id=" << id << L" lat=" << lats[id] << L" lon=" << lons[id]
              << endl;
      }
    }
  }

  verify(lats, lons);
}

double BaseGeoPointTestCase::quantizeLat(double lat) { return lat; }

double BaseGeoPointTestCase::quantizeLon(double lon) { return lon; }

bool BaseGeoPointTestCase::rectContainsPoint(shared_ptr<Rectangle> rect,
                                             double pointLat, double pointLon)
{
  assert(isnan(pointLat) == false);

  if (pointLat < rect->minLat || pointLat > rect->maxLat) {
    return false;
  }

  if (rect->minLon <= rect->maxLon) {
    return pointLon >= rect->minLon && pointLon <= rect->maxLon;
  } else {
    // Rect crosses dateline:
    return pointLon <= rect->maxLon || pointLon >= rect->minLon;
  }
}

void BaseGeoPointTestCase::verify(
    std::deque<double> &lats, std::deque<double> &lons) 
{
  // quantize each value the same way the index does
  // NaN means missing for the doc!!!!!
  for (int i = 0; i < lats.size(); i++) {
    if (!isnan(lats[i])) {
      lats[i] = quantizeLat(lats[i]);
    }
  }
  for (int i = 0; i < lons.size(); i++) {
    if (!isnan(lons[i])) {
      lons[i] = quantizeLon(lons[i]);
    }
  }
  verifyRandomRectangles(lats, lons);
  verifyRandomDistances(lats, lons);
  if (supportsPolygons()) {
    verifyRandomPolygons(lats, lons);
  }
}

void BaseGeoPointTestCase::verifyRandomRectangles(
    std::deque<double> &lats, std::deque<double> &lons) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < lats.size() / 100) {
    iwc->setMaxBufferedDocs(lats.size() / 100);
  }
  shared_ptr<Directory> dir;
  if (lats.size() > 100000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }

  shared_ptr<Set<int>> deleted = unordered_set<int>();
  // RandomIndexWriter is too slow here:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < lats.size(); id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    if (isnan(lats[id]) == false) {
      addPointToDoc(FIELD_NAME, doc, lats[id], lons[id]);
    }
    w->addDocument(doc);
    if (id > 0 && random()->nextInt(100) == 42) {
      int idToDelete = random()->nextInt(id);
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(idToDelete))});
      deleted->add(idToDelete);
      if (VERBOSE) {
        wcout << L"  delete id=" << idToDelete << endl;
      }
    }
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(25);

  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(s->getIndexReader());
  int maxDoc = s->getIndexReader()->maxDoc();

  for (int iter = 0; iter < iters; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" s=" << s << endl;
    }

    shared_ptr<Rectangle> rect = nextBox();

    shared_ptr<Query> query = newRectQuery(
        FIELD_NAME, rect->minLat, rect->maxLat, rect->minLon, rect->maxLon);

    if (VERBOSE) {
      wcout << L"  query=" << query << endl;
    }

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(maxDoc);
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass2>(
                         shared_from_this(), hits));

    bool fail = false;
    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");
    for (int docID = 0; docID < maxDoc; docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      bool expected;
      if (liveDocs != nullptr && liveDocs->get(docID) == false) {
        // document is deleted
        expected = false;
      } else if (isnan(lats[id])) {
        expected = false;
      } else {
        expected = rectContainsPoint(rect, lats[id], lons[id]);
      }

      if (hits->get(docID) != expected) {
        shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
        b->append(L"docID=(" + to_wstring(docID) + L")\n");

        if (expected) {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should match but did not\n");
        } else {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should not match but did\n");
        }
        b->append(L"  box=" + rect + L"\n");
        b->append(L"  query=" + query + L" docID=" + to_wstring(docID) + L"\n");
        b->append(L"  lat=" + to_wstring(lats[id]) + L" lon=" +
                  to_wstring(lons[id]) + L"\n");
        b->append(L"  deleted?=" +
                  StringHelper::toString(liveDocs != nullptr &&
                                         liveDocs->get(docID) == false));
        if (true) {
          fail(L"wrong hit (first of possibly more):\n\n" + b);
        } else {
          wcout << b->toString() << endl;
          fail = true;
        }
      }
    }
    if (fail) {
      fail(L"some hits were wrong");
    }
  }

  IOUtils::close({r, dir});
}

BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<BaseGeoPointTestCase> outerInstance,
        shared_ptr<FixedBitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return false;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass2::collect(int doc)
{
  hits->set(docBase + doc);
}

void BaseGeoPointTestCase::verifyRandomDistances(
    std::deque<double> &lats, std::deque<double> &lons) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < lats.size() / 100) {
    iwc->setMaxBufferedDocs(lats.size() / 100);
  }
  shared_ptr<Directory> dir;
  if (lats.size() > 100000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }

  shared_ptr<Set<int>> deleted = unordered_set<int>();
  // RandomIndexWriter is too slow here:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < lats.size(); id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    if (isnan(lats[id]) == false) {
      addPointToDoc(FIELD_NAME, doc, lats[id], lons[id]);
    }
    w->addDocument(doc);
    if (id > 0 && random()->nextInt(100) == 42) {
      int idToDelete = random()->nextInt(id);
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(idToDelete))});
      deleted->add(idToDelete);
      if (VERBOSE) {
        wcout << L"  delete id=" << idToDelete << endl;
      }
    }
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);

  int iters = atLeast(25);

  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(s->getIndexReader());
  int maxDoc = s->getIndexReader()->maxDoc();

  for (int iter = 0; iter < iters; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" s=" << s << endl;
    }

    // Distance
    constexpr double centerLat = nextLatitude();
    constexpr double centerLon = nextLongitude();

    // So the query can cover at most 50% of the earth's surface:
    constexpr double radiusMeters = random()->nextDouble() *
                                        GeoUtils::EARTH_MEAN_RADIUS_METERS *
                                        M_PI / 2.0 +
                                    1.0;

    if (VERBOSE) {
      shared_ptr<DecimalFormat> *const df = make_shared<DecimalFormat>(
          L"#,###.00", DecimalFormatSymbols::getInstance(Locale::ENGLISH));
      wcout << L"  radiusMeters = " << df->format(radiusMeters) << endl;
    }

    shared_ptr<Query> query =
        newDistanceQuery(FIELD_NAME, centerLat, centerLon, radiusMeters);

    if (VERBOSE) {
      wcout << L"  query=" << query << endl;
    }

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(maxDoc);
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass3>(
                         shared_from_this(), hits));

    bool fail = false;
    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");
    for (int docID = 0; docID < maxDoc; docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      bool expected;
      if (liveDocs != nullptr && liveDocs->get(docID) == false) {
        // document is deleted
        expected = false;
      } else if (isnan(lats[id])) {
        expected = false;
      } else {
        expected = SloppyMath::haversinMeters(centerLat, centerLon, lats[id],
                                              lons[id]) <= radiusMeters;
      }

      if (hits->get(docID) != expected) {
        shared_ptr<StringBuilder> b = make_shared<StringBuilder>();

        if (expected) {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should match but did not\n");
        } else {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should not match but did\n");
        }
        b->append(L"  query=" + query + L" docID=" + to_wstring(docID) + L"\n");
        b->append(L"  lat=" + to_wstring(lats[id]) + L" lon=" +
                  to_wstring(lons[id]) + L"\n");
        b->append(L"  deleted?=" +
                  StringHelper::toString(liveDocs != nullptr &&
                                         liveDocs->get(docID) == false));
        if (isnan(lats[id]) == false) {
          double distanceMeters = SloppyMath::haversinMeters(
              centerLat, centerLon, lats[id], lons[id]);
          b->append(L"  centerLat=" + to_wstring(centerLat) + L" centerLon=" +
                    to_wstring(centerLon) + L" distanceMeters=" +
                    to_wstring(distanceMeters) + L" vs radiusMeters=" +
                    to_wstring(radiusMeters));
        }
        if (true) {
          fail(L"wrong hit (first of possibly more):\n\n" + b);
        } else {
          wcout << b->toString() << endl;
          fail = true;
        }
      }
    }
    if (fail) {
      fail(L"some hits were wrong");
    }
  }

  IOUtils::close({r, dir});
}

BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass3::
    SimpleCollectorAnonymousInnerClass3(
        shared_ptr<BaseGeoPointTestCase> outerInstance,
        shared_ptr<FixedBitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass3::needsScores()
{
  return false;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass3::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass3::collect(int doc)
{
  hits->set(docBase + doc);
}

void BaseGeoPointTestCase::verifyRandomPolygons(
    std::deque<double> &lats, std::deque<double> &lons) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < lats.size() / 100) {
    iwc->setMaxBufferedDocs(lats.size() / 100);
  }
  shared_ptr<Directory> dir;
  if (lats.size() > 100000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }

  shared_ptr<Set<int>> deleted = unordered_set<int>();
  // RandomIndexWriter is too slow here:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < lats.size(); id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    if (isnan(lats[id]) == false) {
      addPointToDoc(FIELD_NAME, doc, lats[id], lons[id]);
    }
    w->addDocument(doc);
    if (id > 0 && random()->nextInt(100) == 42) {
      int idToDelete = random()->nextInt(id);
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(idToDelete))});
      deleted->add(idToDelete);
      if (VERBOSE) {
        wcout << L"  delete id=" << idToDelete << endl;
      }
    }
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  delete w;

  // We can't wrap with "exotic" readers because points needs to work:
  shared_ptr<IndexSearcher> s = newSearcher(r);

  constexpr int iters = atLeast(75);

  shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(s->getIndexReader());
  int maxDoc = s->getIndexReader()->maxDoc();

  for (int iter = 0; iter < iters; iter++) {

    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" s=" << s << endl;
    }

    // Polygon
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Query> query = newPolygonQuery(FIELD_NAME, {polygon});

    if (VERBOSE) {
      wcout << L"  query=" << query << endl;
    }

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(maxDoc);
    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass4>(
                         shared_from_this(), hits));

    bool fail = false;
    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");
    for (int docID = 0; docID < maxDoc; docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      bool expected;
      if (liveDocs != nullptr && liveDocs->get(docID) == false) {
        // document is deleted
        expected = false;
      } else if (isnan(lats[id])) {
        expected = false;
      } else {
        expected = GeoTestUtil::containsSlowly(polygon, lats[id], lons[id]);
      }

      if (hits->get(docID) != expected) {
        shared_ptr<StringBuilder> b = make_shared<StringBuilder>();

        if (expected) {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should match but did not\n");
        } else {
          b->append(L"FAIL: id=" + to_wstring(id) +
                    L" should not match but did\n");
        }
        b->append(L"  query=" + query + L" docID=" + to_wstring(docID) + L"\n");
        b->append(L"  lat=" + to_wstring(lats[id]) + L" lon=" +
                  to_wstring(lons[id]) + L"\n");
        b->append(L"  deleted?=" +
                  StringHelper::toString(liveDocs != nullptr &&
                                         liveDocs->get(docID) == false));
        b->append(L"  polygon=" + polygon);
        if (true) {
          fail(L"wrong hit (first of possibly more):\n\n" + b);
        } else {
          wcout << b->toString() << endl;
          fail = true;
        }
      }
    }
    if (fail) {
      fail(L"some hits were wrong");
    }
  }

  IOUtils::close({r, dir});
}

BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass4::
    SimpleCollectorAnonymousInnerClass4(
        shared_ptr<BaseGeoPointTestCase> outerInstance,
        shared_ptr<FixedBitSet> hits)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
}

bool BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass4::needsScores()
{
  return false;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass4::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void BaseGeoPointTestCase::SimpleCollectorAnonymousInnerClass4::collect(int doc)
{
  hits->set(docBase + doc);
}

void BaseGeoPointTestCase::testRectBoundariesAreInclusive() 
{
  shared_ptr<Rectangle> rect;
  // TODO: why this dateline leniency???
  while (true) {
    rect = nextBox();
    if (rect->crossesDateline() == false) {
      break;
    }
  }
  // this test works in quantized space: for testing inclusiveness of exact
  // edges it must be aware of index-time quantization!
  rect = make_shared<Rectangle>(
      quantizeLat(rect->minLat), quantizeLat(rect->maxLat),
      quantizeLon(rect->minLon), quantizeLon(rect->maxLon));
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  for (int x = 0; x < 3; x++) {
    double lat;
    if (x == 0) {
      lat = rect->minLat;
    } else if (x == 1) {
      lat = quantizeLat((rect->minLat + rect->maxLat) / 2.0);
    } else {
      lat = rect->maxLat;
    }
    for (int y = 0; y < 3; y++) {
      double lon;
      if (y == 0) {
        lon = rect->minLon;
      } else if (y == 1) {
        if (x == 1) {
          continue;
        }
        lon = quantizeLon((rect->minLon + rect->maxLon) / 2.0);
      } else {
        lon = rect->maxLon;
      }

      shared_ptr<Document> doc = make_shared<Document>();
      addPointToDoc(FIELD_NAME, doc, lat, lon);
      w->addDocument(doc);
    }
  }
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  // exact edge cases
  TestUtil::assertEquals(
      8, s->count(newRectQuery(FIELD_NAME, rect->minLat, rect->maxLat,
                               rect->minLon, rect->maxLon)));

  // expand 1 ulp in each direction if possible and test a slightly larger box!
  if (rect->minLat != -90) {
    TestUtil::assertEquals(
        8, s->count(newRectQuery(FIELD_NAME, Math::nextDown(rect->minLat),
                                 rect->maxLat, rect->minLon, rect->maxLon)));
  }
  if (rect->maxLat != 90) {
    TestUtil::assertEquals(8,
                           s->count(newRectQuery(FIELD_NAME, rect->minLat,
                                                 Math::nextUp(rect->maxLat),
                                                 rect->minLon, rect->maxLon)));
  }
  if (rect->minLon != -180) {
    TestUtil::assertEquals(
        8, s->count(newRectQuery(FIELD_NAME, rect->minLat, rect->maxLat,
                                 Math::nextDown(rect->minLon), rect->maxLon)));
  }
  if (rect->maxLon != 180) {
    TestUtil::assertEquals(
        8, s->count(newRectQuery(FIELD_NAME, rect->minLat, rect->maxLat,
                                 rect->minLon, Math::nextUp(rect->maxLon))));
  }

  // now shrink 1 ulp in each direction if possible: it should not include bogus
  // stuff we can't shrink if values are already at extremes, and we can't do
  // this if rectangle is actually a line or we will create a cross-dateline
  // query
  if (rect->minLat != 90 && rect->maxLat != -90 && rect->minLon != 80 &&
      rect->maxLon != -180 && rect->minLon != rect->maxLon) {
    // note we put points on "sides" not just "corners" so we just shrink all 4
    // at once for now: it should exclude all points!
    TestUtil::assertEquals(
        0, s->count(newRectQuery(FIELD_NAME, Math::nextUp(rect->minLat),
                                 Math::nextDown(rect->maxLat),
                                 Math::nextUp(rect->minLon),
                                 Math::nextDown(rect->maxLon))));
  }

  delete r;
  delete w;
  delete dir;
}

void BaseGeoPointTestCase::testRandomDistance() 
{
  for (int iters = 0; iters < 100; iters++) {
    doRandomDistanceTest(10, 100);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomDistanceHuge() throws Exception
void BaseGeoPointTestCase::testRandomDistanceHuge() 
{
  for (int iters = 0; iters < 10; iters++) {
    doRandomDistanceTest(2000, 100);
  }
}

void BaseGeoPointTestCase::doRandomDistanceTest(
    int numDocs, int numQueries) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  int pointsInLeaf = 2 + random()->nextInt(4);
  iwc->setCodec(make_shared<FilterCodecAnonymousInnerClass>(
      shared_from_this(), TestUtil::getDefaultCodec(), pointsInLeaf));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  for (int i = 0; i < numDocs; i++) {
    double latRaw = nextLatitude();
    double lonRaw = nextLongitude();
    // pre-normalize up front, so we can just use quantized value for testing
    // and do simple exact comparisons
    double lat = quantizeLat(latRaw);
    double lon = quantizeLon(lonRaw);
    shared_ptr<Document> doc = make_shared<Document>();
    addPointToDoc(L"field", doc, lat, lon);
    doc->push_back(make_shared<StoredField>(L"lat", lat));
    doc->push_back(make_shared<StoredField>(L"lon", lon));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  for (int i = 0; i < numQueries; i++) {
    double lat = nextLatitude();
    double lon = nextLongitude();
    double radius = 50000000 * random()->nextDouble();

    shared_ptr<BitSet> expected = make_shared<BitSet>();
    for (int doc = 0; doc < reader->maxDoc(); doc++) {
      double docLatitude = reader->document(doc)
                               ->getField(L"lat")
                               ->numericValue()
                               ->doubleValue();
      double docLongitude = reader->document(doc)
                                ->getField(L"lon")
                                ->numericValue()
                                ->doubleValue();
      double distance =
          SloppyMath::haversinMeters(lat, lon, docLatitude, docLongitude);
      if (distance <= radius) {
        expected->set(doc);
      }
    }

    shared_ptr<TopDocs> topDocs =
        searcher->search(newDistanceQuery(L"field", lat, lon, radius),
                         reader->maxDoc(), Sort::INDEXORDER);
    shared_ptr<BitSet> actual = make_shared<BitSet>();
    for (auto doc : topDocs->scoreDocs) {
      actual->set(doc->doc);
    }

    try {
      TestUtil::assertEquals(expected, actual);
    } catch (const AssertionError &e) {
      wcout << L"center: (" << lat << L"," << lon << L"), radius=" << radius
            << endl;
      for (int doc = 0; doc < reader->maxDoc(); doc++) {
        double docLatitude = reader->document(doc)
                                 ->getField(L"lat")
                                 ->numericValue()
                                 ->doubleValue();
        double docLongitude = reader->document(doc)
                                  ->getField(L"lon")
                                  ->numericValue()
                                  ->doubleValue();
        double distance =
            SloppyMath::haversinMeters(lat, lon, docLatitude, docLongitude);
        wcout << L"" << doc << L": (" << docLatitude << L"," << docLongitude
              << L"), distance=" << distance << endl;
      }
      throw e;
    }
  }
  delete reader;
  delete writer;
  delete dir;
}

BaseGeoPointTestCase::FilterCodecAnonymousInnerClass::
    FilterCodecAnonymousInnerClass(
        shared_ptr<BaseGeoPointTestCase> outerInstance,
        shared_ptr<org::apache::lucene::codecs::Codec> getDefaultCodec,
        int pointsInLeaf)
    : org::apache::lucene::codecs::FilterCodec(L"Lucene70", getDefaultCodec)
{
  this->outerInstance = outerInstance;
  this->pointsInLeaf = pointsInLeaf;
}

shared_ptr<PointsFormat>
BaseGeoPointTestCase::FilterCodecAnonymousInnerClass::pointsFormat()
{
  return make_shared<PointsFormatAnonymousInnerClass>(shared_from_this());
}

BaseGeoPointTestCase::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
        shared_ptr<FilterCodecAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointsWriter> BaseGeoPointTestCase::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsWriter(
        shared_ptr<SegmentWriteState> writeState) 
{
  return make_shared<Lucene60PointsWriter>(
      writeState, outerInstance->pointsInLeaf,
      BKDWriter::DEFAULT_MAX_MB_SORT_IN_HEAP);
}

shared_ptr<PointsReader> BaseGeoPointTestCase::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsReader(
        shared_ptr<SegmentReadState> readState) 
{
  return make_shared<Lucene60PointsReader>(readState);
}

void BaseGeoPointTestCase::testEquals() 
{
  shared_ptr<Query> q1, q2;

  shared_ptr<Rectangle> rect = nextBox();

  q1 = newRectQuery(L"field", rect->minLat, rect->maxLat, rect->minLon,
                    rect->maxLon);
  q2 = newRectQuery(L"field", rect->minLat, rect->maxLat, rect->minLon,
                    rect->maxLon);
  TestUtil::assertEquals(q1, q2);
  // for "impossible" ranges LatLonPoint.newBoxQuery will return
  // MatchNoDocsQuery changing the field is unrelated to that.
  if (std::dynamic_pointer_cast<MatchNoDocsQuery>(q1) != nullptr == false) {
    assertFalse(q1->equals(newRectQuery(L"field2", rect->minLat, rect->maxLat,
                                        rect->minLon, rect->maxLon)));
  }

  double lat = nextLatitude();
  double lon = nextLongitude();
  q1 = newDistanceQuery(L"field", lat, lon, 10000.0);
  q2 = newDistanceQuery(L"field", lat, lon, 10000.0);
  TestUtil::assertEquals(q1, q2);
  assertFalse(q1->equals(newDistanceQuery(L"field2", lat, lon, 10000.0)));

  std::deque<double> lats(5);
  std::deque<double> lons(5);
  lats[0] = rect->minLat;
  lons[0] = rect->minLon;
  lats[1] = rect->maxLat;
  lons[1] = rect->minLon;
  lats[2] = rect->maxLat;
  lons[2] = rect->maxLon;
  lats[3] = rect->minLat;
  lons[3] = rect->maxLon;
  lats[4] = rect->minLat;
  lons[4] = rect->minLon;
  if (supportsPolygons()) {
    q1 = newPolygonQuery(L"field", {make_shared<Polygon>(lats, lons)});
    q2 = newPolygonQuery(L"field", {make_shared<Polygon>(lats, lons)});
    TestUtil::assertEquals(q1, q2);
    assertFalse(q1->equals(
        newPolygonQuery(L"field2", {make_shared<Polygon>(lats, lons)})));
  }
}

shared_ptr<TopDocs>
BaseGeoPointTestCase::searchSmallSet(shared_ptr<Query> query,
                                     int size) 
{
  // this is a simple systematic test, indexing these points
  // TODO: fragile: does not understand quantization in any way yet uses
  // extremely high precision!
  std::deque<std::deque<double>> pts = {
      std::deque<double>{32.763420, -96.774},
      std::deque<double>{32.7559529921407, -96.7759895324707},
      std::deque<double>{32.77866942010977, -96.77701950073242},
      std::deque<double>{32.7756745755423, -96.7706036567688},
      std::deque<double>{27.703618681345585, -139.73458170890808},
      std::deque<double>{32.94823588839368, -96.4538113027811},
      std::deque<double>{33.06047141970814, -96.65084838867188},
      std::deque<double>{32.778650, -96.7772},
      std::deque<double>{-88.56029371730983, -177.23537676036358},
      std::deque<double>{33.541429799076354, -26.779373834241003},
      std::deque<double>{26.774024500421728, -77.35379276106497},
      std::deque<double>{-90.0, -14.796283808944777},
      std::deque<double>{32.94823588839368, -178.8538113027811},
      std::deque<double>{32.94823588839368, 178.8538113027811},
      std::deque<double>{40.720611, -73.998776},
      std::deque<double>{-44.5, -179.5}};

  shared_ptr<Directory> directory = newDirectory();

  // TODO: must these simple tests really rely on docid order?
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(TestUtil::nextInt(random(), 100, 1000));
  iwc->setMergePolicy(newLogMergePolicy());
  // Else seeds may not reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory, iwc);

  for (auto p : pts) {
    shared_ptr<Document> doc = make_shared<Document>();
    addPointToDoc(L"point", doc, p[0], p[1]);
    writer->addDocument(doc);
  }

  // add explicit multi-valued docs
  for (int i = 0; i < pts.size(); i += 2) {
    shared_ptr<Document> doc = make_shared<Document>();
    addPointToDoc(L"point", doc, pts[i][0], pts[i][1]);
    addPointToDoc(L"point", doc, pts[i + 1][0], pts[i + 1][1]);
    writer->addDocument(doc);
  }

  // index random string documents
  for (int i = 0; i < random()->nextInt(10); ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"string", Integer::toString(i),
                                            Field::Store::NO));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TopDocs> topDocs = searcher->search(query, size);
  delete reader;
  delete directory;
  return topDocs;
}

void BaseGeoPointTestCase::testSmallSetRect() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newRectQuery(L"point", 32.778, 32.779, -96.778, -96.777), 5);
  TestUtil::assertEquals(4, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetDateline() 
{
  shared_ptr<TopDocs> td =
      searchSmallSet(newRectQuery(L"point", -45.0, -44.0, 179.0, -179.0), 20);
  TestUtil::assertEquals(2, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetMultiValued() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newRectQuery(L"point", 32.755, 32.776, -96.454, -96.770), 20);
  // 3 single valued docs + 2 multi-valued docs
  TestUtil::assertEquals(5, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetWholeMap() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newRectQuery(L"point", GeoUtils::MIN_LAT_INCL, GeoUtils::MAX_LAT_INCL,
                   GeoUtils::MIN_LON_INCL, GeoUtils::MAX_LON_INCL),
      20);
  TestUtil::assertEquals(24, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetPoly() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<TopDocs> td = searchSmallSet(
      newPolygonQuery(
          L"point",
          {make_shared<Polygon>(
              std::deque<double>{33.073130, 32.9942669, 32.938386, 33.0374494,
                                  33.1369762, 33.1162747, 33.073130, 33.073130},
              std::deque<double>{-96.7682647, -96.8280029, -96.6288757,
                                  -96.4929199, -96.6041564, -96.7449188,
                                  -96.76826477, -96.7682647})}),
      5);
  TestUtil::assertEquals(2, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetPolyWholeMap() 
{
  assumeTrue(L"Impl does not support polygons", supportsPolygons());
  shared_ptr<TopDocs> td = searchSmallSet(
      newPolygonQuery(L"point",
                      {make_shared<Polygon>(
                          std::deque<double>{
                              GeoUtils::MIN_LAT_INCL, GeoUtils::MAX_LAT_INCL,
                              GeoUtils::MAX_LAT_INCL, GeoUtils::MIN_LAT_INCL,
                              GeoUtils::MIN_LAT_INCL},
                          std::deque<double>{
                              GeoUtils::MIN_LON_INCL, GeoUtils::MIN_LON_INCL,
                              GeoUtils::MAX_LON_INCL, GeoUtils::MAX_LON_INCL,
                              GeoUtils::MIN_LON_INCL})}),
      20);
  assertEquals(L"testWholeMap failed", 24, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetDistance() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newDistanceQuery(L"point", 32.94823588839368, -96.4538113027811, 6000),
      20);
  TestUtil::assertEquals(2, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetTinyDistance() 
{
  shared_ptr<TopDocs> td =
      searchSmallSet(newDistanceQuery(L"point", 40.720611, -73.998776, 1), 20);
  TestUtil::assertEquals(2, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetDistanceNotEmpty() 
{
  shared_ptr<TopDocs> td =
      searchSmallSet(newDistanceQuery(L"point", -88.56029371730983,
                                      -177.23537676036358, 7757.999232959935),
                     20);
  TestUtil::assertEquals(2, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetHugeDistance() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newDistanceQuery(L"point", 32.94823588839368, -96.4538113027811, 6000000),
      20);
  TestUtil::assertEquals(16, td->totalHits);
}

void BaseGeoPointTestCase::testSmallSetDistanceDateline() 
{
  shared_ptr<TopDocs> td = searchSmallSet(
      newDistanceQuery(L"point", 32.94823588839368, -179.9538113027811, 120000),
      20);
  TestUtil::assertEquals(3, td->totalHits);
}
} // namespace org::apache::lucene::geo