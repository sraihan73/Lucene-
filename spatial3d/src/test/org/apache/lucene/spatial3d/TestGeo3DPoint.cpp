using namespace std;

#include "TestGeo3DPoint.h"

namespace org::apache::lucene::spatial3d
{
using Codec = org::apache::lucene::codecs::Codec;
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
using GeoTestUtil = org::apache::lucene::geo::GeoTestUtil;
using Polygon = org::apache::lucene::geo::Polygon;
using Rectangle = org::apache::lucene::geo::Rectangle;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using GeoArea = org::apache::lucene::spatial3d::geom::GeoArea;
using GeoAreaFactory = org::apache::lucene::spatial3d::geom::GeoAreaFactory;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoPathFactory = org::apache::lucene::spatial3d::geom::GeoPathFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPolygon = org::apache::lucene::spatial3d::geom::GeoPolygon;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using Plane = org::apache::lucene::spatial3d::geom::Plane;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using SidedPlane = org::apache::lucene::spatial3d::geom::SidedPlane;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;
using XYZSolid = org::apache::lucene::spatial3d::geom::XYZSolid;
using XYZSolidFactory = org::apache::lucene::spatial3d::geom::XYZSolidFactory;
using Directory = org::apache::lucene::store::Directory;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

shared_ptr<Codec> TestGeo3DPoint::getCodec()
{
  if (Codec::getDefault()->getName() == L"Lucene70") {
    int maxPointsInLeafNode = TestUtil::nextInt(random(), 16, 2048);
    double maxMBSortInHeap = 3.0 + (3 * random()->nextDouble());
    if (VERBOSE) {
      wcout << L"TEST: using Lucene60PointsFormat with maxPointsInLeafNode="
            << maxPointsInLeafNode << L" and maxMBSortInHeap="
            << maxMBSortInHeap << endl;
    }

    return make_shared<FilterCodecAnonymousInnerClass>(
        Codec::getDefault(), maxPointsInLeafNode, maxMBSortInHeap);
  } else {
    return Codec::getDefault();
  }
}

TestGeo3DPoint::FilterCodecAnonymousInnerClass::FilterCodecAnonymousInnerClass(
    shared_ptr<Codec> getDefault, int maxPointsInLeafNode,
    double maxMBSortInHeap)
    : org::apache::lucene::codecs::FilterCodec(L"Lucene70", getDefault)
{
  this->maxPointsInLeafNode = maxPointsInLeafNode;
  this->maxMBSortInHeap = maxMBSortInHeap;
}

shared_ptr<PointsFormat>
TestGeo3DPoint::FilterCodecAnonymousInnerClass::pointsFormat()
{
  return make_shared<PointsFormatAnonymousInnerClass>(shared_from_this());
}

TestGeo3DPoint::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::PointsFormatAnonymousInnerClass(
        shared_ptr<FilterCodecAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointsWriter> TestGeo3DPoint::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsWriter(
        shared_ptr<SegmentWriteState> writeState) 
{
  return make_shared<Lucene60PointsWriter>(writeState,
                                           outerInstance->maxPointsInLeafNode,
                                           outerInstance->maxMBSortInHeap);
}

shared_ptr<PointsReader> TestGeo3DPoint::FilterCodecAnonymousInnerClass::
    PointsFormatAnonymousInnerClass::fieldsReader(
        shared_ptr<SegmentReadState> readState) 
{
  return make_shared<Lucene60PointsReader>(readState);
}

void TestGeo3DPoint::testBasic() 
{
  shared_ptr<Directory> dir = getDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(getCodec());
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Geo3DPoint>(L"field", 50.7345267, -97.5303555));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  // We can't wrap with "exotic" readers because the query must see the
  // BKD3DDVFormat:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  TestUtil::assertEquals(
      1, s->search(Geo3DPoint::newShapeQuery(
                       L"field", GeoCircleFactory::makeGeoCircle(
                                     PlanetModel::WGS84, toRadians(50),
                                     toRadians(-97), M_PI / 180.0)),
                   1)
             ->totalHits);
  delete w;
  delete r;
  delete dir;
}

double TestGeo3DPoint::toRadians(double degrees)
{
  return degrees * Geo3DUtil::RADIANS_PER_DEGREE;
}

int TestGeo3DPoint::Cell::nextCellID = 0;

TestGeo3DPoint::Cell::Cell(shared_ptr<Cell> parent, int xMinEnc, int xMaxEnc,
                           int yMinEnc, int yMaxEnc, int zMinEnc, int zMaxEnc,
                           int splitCount)
    : parent(parent), cellID(nextCellID++), xMinEnc(xMinEnc), xMaxEnc(xMaxEnc),
      yMinEnc(yMinEnc), yMaxEnc(yMaxEnc), zMinEnc(zMinEnc), zMaxEnc(zMaxEnc),
      splitCount(splitCount)
{
}

bool TestGeo3DPoint::Cell::contains(shared_ptr<GeoPoint> point)
{
  int docX = Geo3DUtil::encodeValue(point->x);
  int docY = Geo3DUtil::encodeValue(point->y);
  int docZ = Geo3DUtil::encodeValue(point->z);

  return docX >= xMinEnc && docX <= xMaxEnc && docY >= yMinEnc &&
         docY <= yMaxEnc && docZ >= zMinEnc && docZ <= zMaxEnc;
}

wstring TestGeo3DPoint::Cell::toString()
{
  return L"cell=" + to_wstring(cellID) +
         (parent == nullptr ? L""
                            : L" parentCellID=" + to_wstring(parent->cellID)) +
         L" x: " + to_wstring(xMinEnc) + L" TO " + to_wstring(xMaxEnc) +
         L", y: " + to_wstring(yMinEnc) + L" TO " + to_wstring(yMaxEnc) +
         L", z: " + to_wstring(zMinEnc) + L" TO " + to_wstring(zMaxEnc) +
         L", splits: " + to_wstring(splitCount);
}

double TestGeo3DPoint::quantize(double xyzValue)
{
  return Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(xyzValue));
}

shared_ptr<GeoPoint> TestGeo3DPoint::quantize(shared_ptr<GeoPoint> point)
{
  return make_shared<GeoPoint>(quantize(point->x), quantize(point->y),
                               quantize(point->z));
}

void TestGeo3DPoint::testGeo3DRelations() 
{

  int numDocs = atLeast(1000);
  if (VERBOSE) {
    wcout << L"TEST: " << numDocs << L" docs" << endl;
  }

  std::deque<std::shared_ptr<GeoPoint>> docs(numDocs);
  std::deque<std::shared_ptr<GeoPoint>> unquantizedDocs(numDocs);
  for (int docID = 0; docID < numDocs; docID++) {
    unquantizedDocs[docID] = make_shared<GeoPoint>(
        PlanetModel::WGS84, toRadians(GeoTestUtil::nextLatitude()),
        toRadians(GeoTestUtil::nextLongitude()));
    docs[docID] = quantize(unquantizedDocs[docID]);
    if (VERBOSE) {
      wcout << L"  doc=" << docID << L": " << docs[docID] << L"; unquantized: "
            << unquantizedDocs[docID] << endl;
    }
  }

  int iters = atLeast(10);

  int recurseDepth = RandomNumbers::randomIntBetween(random(), 5, 15);

  iters = atLeast(50);

  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<GeoShape> shape = randomShape();

    shared_ptr<StringWriter> sw = make_shared<StringWriter>();
    shared_ptr<PrintWriter> log = make_shared<PrintWriter>(sw, true);

    if (VERBOSE) {
      log->println(L"TEST: iter=" + to_wstring(iter) + L" shape=" + shape);
    }

    shared_ptr<XYZBounds> bounds = make_shared<XYZBounds>();
    shape->getBounds(bounds);

    // Start with the root cell that fully contains the shape:
    shared_ptr<Cell> root =
        make_shared<Cell>(nullptr, encodeValueLenient(bounds->getMinimumX()),
                          encodeValueLenient(bounds->getMaximumX()),
                          encodeValueLenient(bounds->getMinimumY()),
                          encodeValueLenient(bounds->getMaximumY()),
                          encodeValueLenient(bounds->getMinimumZ()),
                          encodeValueLenient(bounds->getMaximumZ()), 0);

    if (VERBOSE) {
      log->println(L"  root cell: " + root);
    }

    {
      // make sure the root cell (XYZBounds) does in fact contain all points
      // that the shape contains
      bool fail = false;
      for (int docID = 0; docID < numDocs; docID++) {
        if (root->contains(docs[docID]) == false) {
          bool expected = shape->isWithin(unquantizedDocs[docID]);
          if (expected) {
            log->println(L"    doc=" + to_wstring(docID) +
                         L" is contained by shape but is outside the returned "
                         L"XYZBounds");
            log->println(L"      unquantized=" + unquantizedDocs[docID]);
            log->println(L"      quantized=" + docs[docID]);
            fail = true;
          }
        }
      }

      if (fail) {
        log->println(L"  shape=" + shape);
        log->println(L"  bounds=" + bounds);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << sw->toString();
        fail(L"invalid bounds for shape=" + shape);
      }
    }

    deque<std::shared_ptr<Cell>> queue = deque<std::shared_ptr<Cell>>();
    queue.push_back(root);
    shared_ptr<Set<int>> hits = unordered_set<int>();

    while (queue.size() > 0) {
      shared_ptr<Cell> cell = queue[queue.size() - 1];
      queue.pop_back();
      if (VERBOSE) {
        log->println(L"  cycle: " + cell + L" queue.size()=" + queue.size());
      }

      if (random()->nextInt(10) == 7 || cell->splitCount > recurseDepth) {
        if (VERBOSE) {
          log->println(L"    leaf");
        }
        // Leaf cell: brute force check all docs that fall within this cell:
        for (int docID = 0; docID < numDocs; docID++) {
          shared_ptr<GeoPoint> point = docs[docID];
          shared_ptr<GeoPoint> mappedPoint = unquantizedDocs[docID];
          bool pointWithinShape = shape->isWithin(point);
          bool mappedPointWithinShape = shape->isWithin(mappedPoint);
          if (cell->contains(point)) {
            if (mappedPointWithinShape) {
              if (VERBOSE) {
                log->println(L"    check doc=" + to_wstring(docID) +
                             L": match!  Actual quantized point within: " +
                             StringHelper::toString(pointWithinShape));
              }
              hits->add(docID);
            } else {
              if (VERBOSE) {
                log->println(L"    check doc=" + to_wstring(docID) +
                             L": no match.  Quantized point within: " +
                             StringHelper::toString(pointWithinShape));
              }
            }
          }
        }
      } else {

        shared_ptr<GeoArea> xyzSolid = GeoAreaFactory::makeGeoArea(
            PlanetModel::WGS84, Geo3DUtil::decodeValueFloor(cell->xMinEnc),
            Geo3DUtil::decodeValueCeil(cell->xMaxEnc),
            Geo3DUtil::decodeValueFloor(cell->yMinEnc),
            Geo3DUtil::decodeValueCeil(cell->yMaxEnc),
            Geo3DUtil::decodeValueFloor(cell->zMinEnc),
            Geo3DUtil::decodeValueCeil(cell->zMaxEnc));

        if (VERBOSE) {
          log->println(L"    minx=" +
                       to_wstring(Geo3DUtil::decodeValueFloor(cell->xMinEnc)) +
                       L" maxx=" +
                       to_wstring(Geo3DUtil::decodeValueCeil(cell->xMaxEnc)) +
                       L" miny=" +
                       to_wstring(Geo3DUtil::decodeValueFloor(cell->yMinEnc)) +
                       L" maxy=" +
                       to_wstring(Geo3DUtil::decodeValueCeil(cell->yMaxEnc)) +
                       L" minz=" +
                       to_wstring(Geo3DUtil::decodeValueFloor(cell->zMinEnc)) +
                       L" maxz=" +
                       to_wstring(Geo3DUtil::decodeValueCeil(cell->zMaxEnc)));
        }

        switch (xyzSolid->getRelationship(shape)) {
        case GeoArea::CONTAINS:
          // Shape fully contains the cell: blindly add all docs in this cell:
          if (VERBOSE) {
            log->println(L"    GeoArea.CONTAINS: now addAll");
          }
          for (int docID = 0; docID < numDocs; docID++) {
            if (cell->contains(docs[docID])) {
              if (VERBOSE) {
                log->println(L"    addAll doc=" + to_wstring(docID));
              }
              hits->add(docID);
            }
          }
          continue;
        case GeoArea::OVERLAPS:
          if (VERBOSE) {
            log->println(L"    GeoArea.OVERLAPS: keep splitting");
          }
          // They do overlap but neither contains the other:
          // log.println("    crosses1");
          break;
        case GeoArea::WITHIN:
          if (VERBOSE) {
            log->println(L"    GeoArea.WITHIN: keep splitting");
          }
          // Cell fully contains the shape:
          // log.println("    crosses2");
          break;
        case GeoArea::DISJOINT:
          // They do not overlap at all: don't recurse on this cell
          // log.println("    outside");
          if (VERBOSE) {
            log->println(L"    GeoArea.DISJOINT: drop this cell");
            for (int docID = 0; docID < numDocs; docID++) {
              if (cell->contains(docs[docID])) {
                log->println(L"    skip doc=" + to_wstring(docID));
              }
            }
          }
          continue;
        default:
          assert(false);
        }

        // Randomly split:
        switch (random()->nextInt(3)) {

        case 0: {
          // Split on X:
          int splitValue = RandomNumbers::randomIntBetween(
              random(), cell->xMinEnc, cell->xMaxEnc);
          if (VERBOSE) {
            log->println(L"    now split on x=" + to_wstring(splitValue));
          }
          shared_ptr<Cell> cell1 = make_shared<Cell>(
              cell, cell->xMinEnc, splitValue, cell->yMinEnc, cell->yMaxEnc,
              cell->zMinEnc, cell->zMaxEnc, cell->splitCount + 1);
          shared_ptr<Cell> cell2 = make_shared<Cell>(
              cell, splitValue, cell->xMaxEnc, cell->yMinEnc, cell->yMaxEnc,
              cell->zMinEnc, cell->zMaxEnc, cell->splitCount + 1);
          if (VERBOSE) {
            log->println(L"    split cell1: " + cell1);
            log->println(L"    split cell2: " + cell2);
          }
          queue.push_back(cell1);
          queue.push_back(cell2);
        } break;

        case 1: {
          // Split on Y:
          int splitValue = RandomNumbers::randomIntBetween(
              random(), cell->yMinEnc, cell->yMaxEnc);
          if (VERBOSE) {
            log->println(L"    now split on y=" + to_wstring(splitValue));
          }
          shared_ptr<Cell> cell1 = make_shared<Cell>(
              cell, cell->xMinEnc, cell->xMaxEnc, cell->yMinEnc, splitValue,
              cell->zMinEnc, cell->zMaxEnc, cell->splitCount + 1);
          shared_ptr<Cell> cell2 = make_shared<Cell>(
              cell, cell->xMinEnc, cell->xMaxEnc, splitValue, cell->yMaxEnc,
              cell->zMinEnc, cell->zMaxEnc, cell->splitCount + 1);
          if (VERBOSE) {
            log->println(L"    split cell1: " + cell1);
            log->println(L"    split cell2: " + cell2);
          }
          queue.push_back(cell1);
          queue.push_back(cell2);
        } break;

        case 2: {
          // Split on Z:
          int splitValue = RandomNumbers::randomIntBetween(
              random(), cell->zMinEnc, cell->zMaxEnc);
          if (VERBOSE) {
            log->println(L"    now split on z=" + to_wstring(splitValue));
          }
          shared_ptr<Cell> cell1 = make_shared<Cell>(
              cell, cell->xMinEnc, cell->xMaxEnc, cell->yMinEnc, cell->yMaxEnc,
              cell->zMinEnc, splitValue, cell->splitCount + 1);
          shared_ptr<Cell> cell2 = make_shared<Cell>(
              cell, cell->xMinEnc, cell->xMaxEnc, cell->yMinEnc, cell->yMaxEnc,
              splitValue, cell->zMaxEnc, cell->splitCount + 1);
          if (VERBOSE) {
            log->println(L"    split cell1: " + cell1);
            log->println(L"    split cell2: " + cell2);
          }
          queue.push_back(cell1);
          queue.push_back(cell2);
        } break;
        }
      }
    }

    if (VERBOSE) {
      log->println(L"  " + hits->size() + L" hits");
    }

    // Done matching, now verify:
    bool fail = false;
    for (int docID = 0; docID < numDocs; docID++) {
      shared_ptr<GeoPoint> point = docs[docID];
      shared_ptr<GeoPoint> mappedPoint = unquantizedDocs[docID];
      bool expected = shape->isWithin(mappedPoint);
      bool actual = hits->contains(docID);
      if (actual != expected) {
        if (actual) {
          log->println(L"doc=" + to_wstring(docID) +
                       L" should not have matched but did");
        } else {
          log->println(L"doc=" + to_wstring(docID) +
                       L" should match but did not");
        }
        log->println(L"  point=" + point);
        log->println(L"  mappedPoint=" + mappedPoint);
        fail = true;
      }
    }

    if (fail) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << sw->toString();
      fail(L"invalid hits for shape=" + shape);
    }
  }
}

void TestGeo3DPoint::testRandomTiny() 
{
  // Make sure single-leaf-node case is OK:
  doTestRandom(10);
}

void TestGeo3DPoint::testRandomMedium() 
{
  doTestRandom(10000);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
void TestGeo3DPoint::testRandomBig() 
{
  doTestRandom(50000);
}

void TestGeo3DPoint::doTestRandom(int count) 
{
  int numPoints = atLeast(count);

  if (VERBOSE) {
    System::err::println(L"TEST: numPoints=" + to_wstring(numPoints));
  }

  std::deque<double> lats(numPoints);
  std::deque<double> lons(numPoints);

  bool haveRealDoc = false;

  for (int docID = 0; docID < numPoints; docID++) {
    int x = random()->nextInt(20);
    if (x == 17) {
      // Some docs don't have a point:
      lats[docID] = NAN;
      if (VERBOSE) {
        System::err::println(L"  doc=" + to_wstring(docID) + L" is missing");
      }
      continue;
    }

    if (docID > 0 && x < 3 && haveRealDoc) {
      int oldDocID;
      while (true) {
        oldDocID = random()->nextInt(docID);
        if (isnan(lats[oldDocID]) == false) {
          break;
        }
      }

      if (x == 0) {
        // Identical lat to old point
        lats[docID] = lats[oldDocID];
        lons[docID] = GeoTestUtil::nextLongitude();
        if (VERBOSE) {
          System::err::println(L"  doc=" + to_wstring(docID) + L" lat=" +
                               to_wstring(lats[docID]) + L" lon=" +
                               to_wstring(lons[docID]) + L" (same lat as doc=" +
                               to_wstring(oldDocID) + L")");
        }
      } else if (x == 1) {
        // Identical lon to old point
        lats[docID] = GeoTestUtil::nextLatitude();
        lons[docID] = lons[oldDocID];
        if (VERBOSE) {
          System::err::println(L"  doc=" + to_wstring(docID) + L" lat=" +
                               to_wstring(lats[docID]) + L" lon=" +
                               to_wstring(lons[docID]) + L" (same lon as doc=" +
                               to_wstring(oldDocID) + L")");
        }
      } else {
        assert(x == 2);
        // Fully identical point:
        lats[docID] = lats[oldDocID];
        lons[docID] = lons[oldDocID];
        if (VERBOSE) {
          System::err::println(
              L"  doc=" + to_wstring(docID) + L" lat=" +
              to_wstring(lats[docID]) + L" lon=" + to_wstring(lons[docID]) +
              L" (same lat/lon as doc=" + to_wstring(oldDocID) + L")");
        }
      }
    } else {
      lats[docID] = GeoTestUtil::nextLatitude();
      lons[docID] = GeoTestUtil::nextLongitude();
      haveRealDoc = true;
      if (VERBOSE) {
        System::err::println(L"  doc=" + to_wstring(docID) + L" lat=" +
                             to_wstring(lats[docID]) + L" lon=" +
                             to_wstring(lons[docID]));
      }
    }
  }

  verify(lats, lons);
}

void TestGeo3DPoint::testPolygonOrdering()
{
  const std::deque<double> lats = std::deque<double>{
      51.204382859999996, 50.89947531437482, 50.8093624806861,
      50.8093624806861,   50.89947531437482, 51.204382859999996,
      51.51015366140113,  51.59953838204167, 51.59953838204167,
      51.51015366140113,  51.204382859999996};
  const std::deque<double> lons = std::deque<double>{
      0.8747711978759765,   0.6509219832137298,   0.35960265165247807,
      0.10290284834752167,  -0.18841648321373008, -0.41226569787597667,
      -0.18960465285650027, 0.10285893781346236,  0.35964656218653757,
      0.6521101528565002,   0.8747711978759765};
  shared_ptr<Query> *const q =
      Geo3DPoint::newPolygonQuery(L"point", {make_shared<Polygon>(lats, lons)});
  // System.out.println(q);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(!q->toString()->find(L"GeoConcavePolygon") != wstring::npos);
}

shared_ptr<Query> TestGeo3DPoint::random3DQuery(const wstring &field)
{
  while (true) {
    constexpr int shapeType = random()->nextInt(5);
    switch (shapeType) {
    case 4: {
      // Large polygons
      constexpr bool isClockwise = random()->nextDouble() < 0.5;
      try {
        shared_ptr<Query> *const q = Geo3DPoint::newLargePolygonQuery(
            field, {makePoly(PlanetModel::WGS84,
                             make_shared<GeoPoint>(
                                 PlanetModel::WGS84,
                                 toRadians(GeoTestUtil::nextLatitude()),
                                 toRadians(GeoTestUtil::nextLongitude())),
                             isClockwise, true)});
        // System.err.println("Generated: "+q);
        // assertTrue(false);
        return q;
      } catch (const invalid_argument &e) {
        continue;
      }
    }

    case 0: {
      // Polygons
      constexpr bool isClockwise = random()->nextDouble() < 0.5;
      try {
        shared_ptr<Query> *const q = Geo3DPoint::newPolygonQuery(
            field, {makePoly(PlanetModel::WGS84,
                             make_shared<GeoPoint>(
                                 PlanetModel::WGS84,
                                 toRadians(GeoTestUtil::nextLatitude()),
                                 toRadians(GeoTestUtil::nextLongitude())),
                             isClockwise, true)});
        // System.err.println("Generated: "+q);
        // assertTrue(false);
        return q;
      } catch (const invalid_argument &e) {
        continue;
      }
    }

    case 1: {
      // Circles
      constexpr double widthMeters =
          random()->nextDouble() * M_PI * MEAN_EARTH_RADIUS_METERS;
      try {
        return Geo3DPoint::newDistanceQuery(field, GeoTestUtil::nextLatitude(),
                                            GeoTestUtil::nextLongitude(),
                                            widthMeters);
      } catch (const invalid_argument &e) {
        continue;
      }
    }

    case 2: {
      // Rectangles
      shared_ptr<Rectangle> *const r = GeoTestUtil::nextBox();
      try {
        return Geo3DPoint::newBoxQuery(field, r->minLat, r->maxLat, r->minLon,
                                       r->maxLon);
      } catch (const invalid_argument &e) {
        continue;
      }
    }

    case 3: {
      // Paths
      // TBD: Need to rework generation to be realistic
      constexpr int pointCount = random()->nextInt(5) + 1;
      constexpr double width =
          random()->nextDouble() * M_PI * 0.5 * MEAN_EARTH_RADIUS_METERS;
      const std::deque<double> latitudes = std::deque<double>(pointCount);
      const std::deque<double> longitudes = std::deque<double>(pointCount);
      for (int i = 0; i < pointCount; i++) {
        latitudes[i] = GeoTestUtil::nextLatitude();
        longitudes[i] = GeoTestUtil::nextLongitude();
      }
      try {
        return Geo3DPoint::newPathQuery(field, latitudes, longitudes, width);
      } catch (const invalid_argument &e) {
        // This is what happens when we create a shape that is invalid. Although
        // it is conceivable that there are cases where the exception is thrown
        // incorrectly, we aren't going to be able to do that in this random
        // test.
        continue;
      }
    }

    default:
      throw make_shared<IllegalStateException>(L"Unexpected shape type");
    }
  }
}

shared_ptr<GeoShape> TestGeo3DPoint::randomShape()
{
  while (true) {
    constexpr int shapeType = random()->nextInt(4);
    switch (shapeType) {
    case 0: {
      // Polygons
      constexpr int vertexCount = random()->nextInt(3) + 3;
      const deque<std::shared_ptr<GeoPoint>> geoPoints =
          deque<std::shared_ptr<GeoPoint>>();
      while (geoPoints.size() < vertexCount) {
        shared_ptr<GeoPoint> *const gPt = make_shared<GeoPoint>(
            PlanetModel::WGS84, toRadians(GeoTestUtil::nextLatitude()),
            toRadians(GeoTestUtil::nextLongitude()));
        geoPoints.push_back(gPt);
      }
      try {
        shared_ptr<GeoShape> *const rval =
            GeoPolygonFactory::makeGeoPolygon(PlanetModel::WGS84, geoPoints);
        if (rval == nullptr) {
          // Degenerate polygon
          continue;
        }
        return rval;
      } catch (const invalid_argument &e) {
        // This is what happens when we create a shape that is invalid. Although
        // it is conceivable that there are cases where the exception is thrown
        // incorrectly, we aren't going to be able to do that in this random
        // test.
        continue;
      }
    }

    case 1: {
      // Circles

      double lat = toRadians(GeoTestUtil::nextLatitude());
      double lon = toRadians(GeoTestUtil::nextLongitude());

      double angle = random()->nextDouble() * M_PI / 2.0;

      try {
        return GeoCircleFactory::makeGeoCircle(PlanetModel::WGS84, lat, lon,
                                               angle);
      } catch (const invalid_argument &iae) {
        // angle is too small; try again:
        continue;
      }
    }

    case 2: {
      // Rectangles
      double lat0 = toRadians(GeoTestUtil::nextLatitude());
      double lat1 = toRadians(GeoTestUtil::nextLatitude());
      if (lat1 < lat0) {
        double x = lat0;
        lat0 = lat1;
        lat1 = x;
      }
      double lon0 = toRadians(GeoTestUtil::nextLongitude());
      double lon1 = toRadians(GeoTestUtil::nextLongitude());
      if (lon1 < lon0) {
        double x = lon0;
        lon0 = lon1;
        lon1 = x;
      }

      return GeoBBoxFactory::makeGeoBBox(PlanetModel::WGS84, lat1, lat0, lon0,
                                         lon1);
    }

    case 3: {
      // Paths
      constexpr int pointCount = random()->nextInt(5) + 1;
      constexpr double width = toRadians(random()->nextInt(89) + 1);
      std::deque<std::shared_ptr<GeoPoint>> points(pointCount);
      for (int i = 0; i < pointCount; i++) {
        points[i] = make_shared<GeoPoint>(
            PlanetModel::WGS84, toRadians(GeoTestUtil::nextLatitude()),
            toRadians(GeoTestUtil::nextLongitude()));
      }
      try {
        return GeoPathFactory::makeGeoPath(PlanetModel::WGS84, width, points);
      } catch (const invalid_argument &e) {
        // This is what happens when we create a shape that is invalid. Although
        // it is conceivable that there are cases where the exception is thrown
        // incorrectly, we aren't going to be able to do that in this random
        // test.
        continue;
      }
    }

    default:
      throw make_shared<IllegalStateException>(L"Unexpected shape type");
    }
  }
}

void TestGeo3DPoint::verify(std::deque<double> &lats,
                            std::deque<double> &lons) 
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();

  std::deque<std::shared_ptr<GeoPoint>> points(lats.size());
  std::deque<std::shared_ptr<GeoPoint>> unquantizedPoints(lats.size());

  // Pre-quantize all lat/lons:
  for (int i = 0; i < lats.size(); i++) {
    if (isnan(lats[i]) == false) {
      // System.out.println("lats[" + i + "] = " + lats[i]);
      unquantizedPoints[i] = make_shared<GeoPoint>(
          PlanetModel::WGS84, toRadians(lats[i]), toRadians(lons[i]));
      points[i] = quantize(unquantizedPoints[i]);
    }
  }

  // Else we can get O(N^2) merging:
  int mbd = iwc->getMaxBufferedDocs();
  if (mbd != -1 && mbd < points.size() / 100) {
    iwc->setMaxBufferedDocs(points.size() / 100);
  }
  iwc->setCodec(getCodec());
  shared_ptr<Directory> dir;
  if (points.size() > 100000) {
    dir = newFSDirectory(createTempDir(L"TestBKDTree"));
  } else {
    dir = getDirectory();
  }
  shared_ptr<Set<int>> deleted = unordered_set<int>();
  // RandomIndexWriter is too slow here:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int id = 0; id < points.size(); id++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(id), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", id));
    shared_ptr<GeoPoint> point = points[id];
    if (point != nullptr) {
      doc->push_back(
          make_shared<Geo3DPoint>(L"point", point->x, point->y, point->z));
    }
    w->addDocument(doc);
    if (id > 0 && random()->nextInt(100) == 42) {
      int idToDelete = random()->nextInt(id);
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(idToDelete))});
      deleted->add(idToDelete);
      if (VERBOSE) {
        System::err::println(L"  delete id=" + to_wstring(idToDelete));
      }
    }
  }
  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> *const r = DirectoryReader::open(w);
  if (VERBOSE) {
    wcout << L"TEST: using reader " << r << endl;
  }
  delete w;

  // We can't wrap with "exotic" readers because the geo3d query must see the
  // Geo3DDVFormat:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);

  constexpr int iters = atLeast(100);

  for (int iter = 0; iter < iters; iter++) {

    /*
    GeoShape shape = randomShape();

    if (VERBOSE) {
      System.err.println("\nTEST: iter=" + iter + " shape="+shape);
    }
    */

    shared_ptr<Query> query =
        random3DQuery(L"point"); // Geo3DPoint.newShapeQuery("point", shape);

    if (VERBOSE) {
      System::err::println(L"  using query: " + query);
    }

    shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(r->maxDoc());

    s->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(hits));

    if (VERBOSE) {
      System::err::println(L"  hitCount: " + to_wstring(hits->cardinality()));
    }

    shared_ptr<NumericDocValues> docIDToID =
        MultiDocValues::getNumericValues(r, L"id");

    for (int docID = 0; docID < r->maxDoc(); docID++) {
      TestUtil::assertEquals(docID, docIDToID->nextDoc());
      int id = static_cast<int>(docIDToID->longValue());
      shared_ptr<GeoPoint> point = points[id];
      shared_ptr<GeoPoint> unquantizedPoint = unquantizedPoints[id];
      if (point != nullptr && unquantizedPoint != nullptr) {
        shared_ptr<GeoShape> shape =
            (std::static_pointer_cast<PointInGeo3DShapeQuery>(query))
                ->getShape();
        shared_ptr<XYZBounds> bounds = make_shared<XYZBounds>();
        shape->getBounds(bounds);
        shared_ptr<XYZSolid> solid = XYZSolidFactory::makeXYZSolid(
            PlanetModel::WGS84, bounds->getMinimumX(), bounds->getMaximumX(),
            bounds->getMinimumY(), bounds->getMaximumY(), bounds->getMinimumZ(),
            bounds->getMaximumZ());

        bool expected =
            ((deleted->contains(id) == false) && shape->isWithin(point));
        if (hits->get(docID) != expected) {
          shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
          if (expected) {
            b->append(L"FAIL: id=" + to_wstring(id) +
                      L" should have matched but did not\n");
          } else {
            b->append(L"FAIL: id=" + to_wstring(id) +
                      L" should not have matched but did\n");
          }
          b->append(L"  shape=" + shape + L"\n");
          b->append(L"  bounds=" + bounds + L"\n");
          b->append(
              wstring(L"  world bounds=(") + L" minX=" +
              to_wstring(PlanetModel::WGS84->getMinimumXValue()) + L" maxX=" +
              to_wstring(PlanetModel::WGS84->getMaximumXValue()) + L" minY=" +
              to_wstring(PlanetModel::WGS84->getMinimumYValue()) + L" maxY=" +
              to_wstring(PlanetModel::WGS84->getMaximumYValue()) + L" minZ=" +
              to_wstring(PlanetModel::WGS84->getMinimumZValue()) + L" maxZ=" +
              to_wstring(PlanetModel::WGS84->getMaximumZValue()) + L"\n");
          b->append(L"  quantized point=" + point + L" within shape? " +
                    shape->isWithin(point) + L" within bounds? " +
                    solid->isWithin(point) + L"\n");
          b->append(L"  unquantized point=" + unquantizedPoint +
                    L" within shape? " + shape->isWithin(unquantizedPoint) +
                    L" within bounds? " + solid->isWithin(unquantizedPoint) +
                    L"\n");
          b->append(L"  docID=" + to_wstring(docID) + L" deleted?=" +
                    StringHelper::toString(deleted->contains(id)) + L"\n");
          b->append(L"  query=" + query + L"\n");
          b->append(L"  explanation:\n    " +
                    StringHelper::replace(explain(L"point", shape, point,
                                                  unquantizedPoint, r, docID),
                                          L"\n", L"\n  "));
          fail(b->toString());
        }
      } else {
        assertFalse(hits->get(docID));
      }
    }
  }

  IOUtils::close({r, dir});
}

TestGeo3DPoint::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<FixedBitSet> hits)
{
  this->hits = hits;
}

bool TestGeo3DPoint::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void TestGeo3DPoint::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

void TestGeo3DPoint::SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  hits->set(docBase + doc);
}

void TestGeo3DPoint::testToString()
{
  // Don't compare entire strings because Java 9 and Java 8 have slightly
  // different values
  shared_ptr<Geo3DPoint> point =
      make_shared<Geo3DPoint>(L"point", 44.244272, 7.769736);
  const wstring stringToCompare = L"Geo3DPoint <point: x=";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      stringToCompare, point->toString()->substr(0, stringToCompare.length()));
}

void TestGeo3DPoint::testShapeQueryToString()
{
  // Don't compare entire strings because Java 9 and Java 8 have slightly
  // different values
  const wstring stringToCompare =
      L"PointInGeo3DShapeQuery: field=point: Shape: GeoStandardCircle: "
      L"{planetmodel=PlanetModel.WGS84, center=[lat=0.7";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      stringToCompare,
      Geo3DPoint::newShapeQuery(
          L"point", GeoCircleFactory::makeGeoCircle(PlanetModel::WGS84,
                                                    toRadians(44.244272),
                                                    toRadians(7.769736), 0.1))
          ->toString()
          ->substr(0, stringToCompare.length()));
}

shared_ptr<Directory> TestGeo3DPoint::getDirectory() { return newDirectory(); }

void TestGeo3DPoint::testEquals()
{
  shared_ptr<GeoShape> shape = randomShape();
  shared_ptr<Query> q = Geo3DPoint::newShapeQuery(L"point", shape);
  TestUtil::assertEquals(q, Geo3DPoint::newShapeQuery(L"point", shape));
  assertFalse(q->equals(Geo3DPoint::newShapeQuery(L"point2", shape)));

  // make a different random shape:
  shared_ptr<GeoShape> shape2;
  do {
    shape2 = randomShape();
  } while (shape->equals(shape2));

  assertFalse(q->equals(Geo3DPoint::newShapeQuery(L"point", shape2)));
}

void TestGeo3DPoint::testComplexPolygons()
{
  shared_ptr<PlanetModel> *const pm = PlanetModel::WGS84;
  // Pick a random pole
  shared_ptr<GeoPoint> *const randomPole =
      make_shared<GeoPoint>(pm, toRadians(GeoTestUtil::nextLatitude()),
                            toRadians(GeoTestUtil::nextLongitude()));
  int iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    // Create a polygon that's less than 180 degrees
    shared_ptr<Polygon> *const clockWise = makePoly(pm, randomPole, true, true);
  }
  iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    // Create a polygon that's greater than 180 degrees
    shared_ptr<Polygon> *const counterClockWise =
        makePoly(pm, randomPole, false, true);
  }
}

double TestGeo3DPoint::MINIMUM_EDGE_ANGLE = toRadians(5.0);
double TestGeo3DPoint::MINIMUM_ARC_ANGLE = toRadians(1.0);

shared_ptr<Polygon> TestGeo3DPoint::makePoly(shared_ptr<PlanetModel> pm,
                                             shared_ptr<GeoPoint> pole,
                                             bool const clockwiseDesired,
                                             bool const createHoles)
{

  // Polygon edges will be arranged around the provided pole, and holes will
  // each have a pole selected within the parent polygon.
  constexpr int pointCount = TestUtil::nextInt(random(), 3, 10);
  // The point angles we pick next.  The only requirement is that they are not
  // all on one side of the pole. We arrange that by picking the next point
  // within what's left of the remaining angle, but never more than 180 degrees,
  // and never less than what is needed to insure that the remaining point
  // choices are less than 180 degrees always. These are all picked in the
  // context of the pole,
  const std::deque<double> angles = std::deque<double>(pointCount);
  const std::deque<double> arcDistance = std::deque<double>(pointCount);
  // Pick a set of points
  while (true) {
    double accumulatedAngle = 0.0;
    for (int i = 0; i < pointCount; i++) {
      constexpr int remainingEdgeCount = pointCount - i;
      constexpr double remainingAngle = 2.0 * M_PI - accumulatedAngle;
      if (remainingEdgeCount == 1) {
        angles[i] = remainingAngle;
      } else {
        // The maximum angle is 180 degrees, or what's left when you give a
        // minimal amount to each edge.
        double maximumAngle =
            remainingAngle - (remainingEdgeCount - 1) * MINIMUM_EDGE_ANGLE;
        if (maximumAngle > M_PI) {
          maximumAngle = M_PI;
        }
        // The minimum angle is MINIMUM_EDGE_ANGLE, or enough to be sure nobody
        // afterwards needs more than 180 degrees.  And since we have three
        // points to start with, we already know that.
        constexpr double minimumAngle = MINIMUM_EDGE_ANGLE;
        // Pick the angle
        constexpr double angle =
            random()->nextDouble() * (maximumAngle - minimumAngle) +
            minimumAngle;
        angles[i] = angle;
        accumulatedAngle += angle;
      }
      // Pick the arc distance randomly; not quite the full range though
      arcDistance[i] =
          random()->nextDouble() * (M_PI * 0.5 - MINIMUM_ARC_ANGLE) +
          MINIMUM_ARC_ANGLE;
    }
    if (clockwiseDesired) {
      // Reverse the signs
      for (int i = 0; i < pointCount; i++) {
        angles[i] = -angles[i];
      }
    }

    // Now, use the pole's information plus angles and arcs to create GeoPoints
    // in the right order.
    const deque<std::shared_ptr<GeoPoint>> polyPoints =
        convertToPoints(pm, pole, angles, arcDistance);

    // Next, do some holes.  No more than 2 of these.  The poles for holes must
    // always be within the polygon, so we're going to use Geo3D to help us
    // select those given the points we just made.

    constexpr int holeCount =
        createHoles ? TestUtil::nextInt(random(), 0, 2) : 0;

    const deque<std::shared_ptr<Polygon>> holeList =
        deque<std::shared_ptr<Polygon>>();

    /* Hole logic is broken and needs rethinking

    // Create the geo3d polygon, so we can test out our poles.
    final GeoPolygon poly;
    try {
      poly = GeoPolygonFactory.makeGeoPolygon(pm, polyPoints, null);
    } catch (IllegalArgumentException e) {
      // This is what happens when three adjacent points are colinear, so try
    again. continue;
    }

    for (int i = 0; i < holeCount; i++) {
      // Choose a pole.  The poly has to be within the polygon, but it also
    cannot be on the polygon edge.
      // If we can't find a good pole we have to give it up and not do the hole.
      for (int k = 0; k < 500; k++) {
        final GeoPoint poleChoice = new GeoPoint(pm,
    toRadians(GeoTestUtil.nextLatitude()),
    toRadians(GeoTestUtil.nextLongitude())); if (!poly.isWithin(poleChoice)) {
          continue;
        }
        // We have a pole within the polygon.  Now try 100 times to build a
    polygon that does not intersect the outside ring.
        // After that we give up and pick a new pole.
        bool foundOne = false;
        for (int j = 0; j < 100; j++) {
          final Polygon insidePoly = makePoly(pm, poleChoice, !clockwiseDesired,
    false);
          // Verify that the inside polygon is OK.  If not, discard and repeat.
          if (!verifyPolygon(pm, insidePoly, poly)) {
            continue;
          }
          holeList.add(insidePoly);
          foundOne = true;
        }
        if (foundOne) {
          break;
        }
      }
    }
    */

    std::deque<std::shared_ptr<Polygon>> holes =
        holeList.toArray(std::deque<std::shared_ptr<Polygon>>(0));

    // Finally, build the polygon and return it
    const std::deque<double> lats = std::deque<double>(polyPoints.size() + 1);
    const std::deque<double> lons = std::deque<double>(polyPoints.size() + 1);

    for (int i = 0; i < polyPoints.size(); i++) {
      lats[i] = polyPoints[i]->getLatitude() * 180.0 / M_PI;
      lons[i] = polyPoints[i]->getLongitude() * 180.0 / M_PI;
    }
    lats[polyPoints.size()] = lats[0];
    lons[polyPoints.size()] = lons[0];
    return make_shared<Polygon>(lats, lons, holes);
  }
}

deque<std::shared_ptr<GeoPoint>> TestGeo3DPoint::convertToPoints(
    shared_ptr<PlanetModel> pm, shared_ptr<GeoPoint> pole,
    std::deque<double> &angles, std::deque<double> &arcDistances)
{
  // To do the point rotations, we need the sine and cosine of the pole latitude
  // and longitude.  Get it here for performance.
  constexpr double sinLatitude = sin(pole->getLatitude());
  constexpr double cosLatitude = cos(pole->getLatitude());
  constexpr double sinLongitude = sin(pole->getLongitude());
  constexpr double cosLongitude = cos(pole->getLongitude());
  const deque<std::shared_ptr<GeoPoint>> rval =
      deque<std::shared_ptr<GeoPoint>>();
  for (int i = 0; i < angles.size(); i++) {
    rval.push_back(createPoint(pm, angles[i], arcDistances[i], sinLatitude,
                               cosLatitude, sinLongitude, cosLongitude));
  }
  return rval;
}

shared_ptr<GeoPoint>
TestGeo3DPoint::createPoint(shared_ptr<PlanetModel> pm, double const angle,
                            double const arcDistance, double const sinLatitude,
                            double const cosLatitude, double const sinLongitude,
                            double const cosLongitude)
{
  // From the angle and arc distance, convert to (x,y,z) in unit space.
  // We want the perspective to be looking down the x axis.  The "angle"
  // measurement is thus in the Y-Z plane. The arcdistance is in X.
  constexpr double x = cos(arcDistance);
  constexpr double yzScale = sin(arcDistance);
  constexpr double y = cos(angle) * yzScale;
  constexpr double z = sin(angle) * yzScale;
  // Now, rotate coordinates so that we shift everything from pole = x-axis to
  // actual coordinates. This transformation should take the point (1,0,0) and
  // transform it to the pole's actual (x,y,z) coordinates. Coordinate rotation
  // formula: x1 = x0 cos T - y0 sin T y1 = x0 sin T + y0 cos T We're in essence
  // undoing the following transformation (from GeoPolygonFactory): x1 = x0 cos
  // az + y0 sin az y1 = - x0 sin az + y0 cos az z1 = z0 x2 = x1 cos al + z1 sin
  // al y2 = y1 z2 = - x1 sin al + z1 cos al So, we reverse the order of the
  // transformations, AND we transform backwards. Transforming backwards means
  // using these identities: sin(-angle) = -sin(angle), cos(-angle) = cos(angle)
  // So:
  // x1 = x0 cos al - z0 sin al
  // y1 = y0
  // z1 = x0 sin al + z0 cos al
  // x2 = x1 cos az - y1 sin az
  // y2 = x1 sin az + y1 cos az
  // z2 = z1
  constexpr double x1 = x * cosLatitude - z * sinLatitude;
  constexpr double y1 = y;
  constexpr double z1 = x * sinLatitude + z * cosLatitude;
  constexpr double x2 = x1 * cosLongitude - y1 * sinLongitude;
  constexpr double y2 = x1 * sinLongitude + y1 * cosLongitude;
  constexpr double z2 = z1;
  // Scale to put the point on the surface
  return pm->createSurfacePoint(x2, y2, z2);
}

bool TestGeo3DPoint::verifyPolygon(shared_ptr<PlanetModel> pm,
                                   shared_ptr<Polygon> polygon,
                                   shared_ptr<GeoPolygon> outsidePolygon)
{
  // Each point in the new poly should be inside the outside poly, and each edge
  // should not intersect the outside poly edge
  const std::deque<double> lats = polygon->getPolyLats();
  const std::deque<double> lons = polygon->getPolyLons();
  const deque<std::shared_ptr<GeoPoint>> polyPoints =
      deque<std::shared_ptr<GeoPoint>>(lats.size() - 1);
  for (int i = 0; i < lats.size() - 1; i++) {
    shared_ptr<GeoPoint> *const newPoint =
        make_shared<GeoPoint>(pm, toRadians(lats[i]), toRadians(lons[i]));
    if (!outsidePolygon->isWithin(newPoint)) {
      return false;
    }
    polyPoints.push_back(newPoint);
  }
  // We don't need to construct the world to find intersections -- just the
  // bordering planes.
  for (int planeIndex = 0; planeIndex < polyPoints.size(); planeIndex++) {
    shared_ptr<GeoPoint> *const startPoint = polyPoints[planeIndex];
    shared_ptr<GeoPoint> *const endPoint =
        polyPoints[legalIndex(planeIndex + 1, polyPoints.size())];
    shared_ptr<GeoPoint> *const beforeStartPoint =
        polyPoints[legalIndex(planeIndex - 1, polyPoints.size())];
    shared_ptr<GeoPoint> *const afterEndPoint =
        polyPoints[legalIndex(planeIndex + 2, polyPoints.size())];
    shared_ptr<SidedPlane> *const beforePlane =
        make_shared<SidedPlane>(endPoint, beforeStartPoint, startPoint);
    shared_ptr<SidedPlane> *const afterPlane =
        make_shared<SidedPlane>(startPoint, endPoint, afterEndPoint);
    shared_ptr<Plane> *const plane = make_shared<Plane>(startPoint, endPoint);

    // Check for intersections!!
    if (outsidePolygon->intersects(plane, nullptr, beforePlane, afterPlane)) {
      return false;
    }
  }
  return true;
}

int TestGeo3DPoint::legalIndex(int index, int size)
{
  if (index >= size) {
    index -= size;
  }
  if (index < 0) {
    index += size;
  }
  return index;
}

void TestGeo3DPoint::testEncodeDecodeCeil() 
{

  // just for testing quantization error
  constexpr double ENCODING_TOLERANCE = Geo3DUtil::DECODE;

  int iters = atLeast(10000);
  for (int iter = 0; iter < iters; iter++) {
    shared_ptr<GeoPoint> point = make_shared<GeoPoint>(
        PlanetModel::WGS84, toRadians(GeoTestUtil::nextLatitude()),
        toRadians(GeoTestUtil::nextLongitude()));
    double xEnc = Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->x));
    assertEquals(L"x=" + to_wstring(point->x) + L" xEnc=" + to_wstring(xEnc) +
                     L" diff=" + to_wstring(point->x - xEnc),
                 point->x, xEnc, ENCODING_TOLERANCE);

    double yEnc = Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->y));
    assertEquals(L"y=" + to_wstring(point->y) + L" yEnc=" + to_wstring(yEnc) +
                     L" diff=" + to_wstring(point->y - yEnc),
                 point->y, yEnc, ENCODING_TOLERANCE);

    double zEnc = Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->z));
    assertEquals(L"z=" + to_wstring(point->z) + L" zEnc=" + to_wstring(zEnc) +
                     L" diff=" + to_wstring(point->z - zEnc),
                 point->z, zEnc, ENCODING_TOLERANCE);
  }

  // check edge/interesting cases explicitly
  double planetMax = PlanetModel::WGS84->getMaximumMagnitude();
  for (auto value : std::deque<double>{0.0, -planetMax, planetMax}) {
    assertEquals(value, Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(value)),
                 ENCODING_TOLERANCE);
  }
}

void TestGeo3DPoint::testEncodeDecodeRoundsDown() 
{

  int iters = atLeast(1000);
  for (int iter = 0; iter < iters; iter++) {
    constexpr double latBase = GeoTestUtil::nextLatitude();
    constexpr double lonBase = GeoTestUtil::nextLongitude();

    // test above the value
    double lat = latBase;
    double lon = lonBase;
    for (int i = 0; i < 1000; i++) {
      lat = min(90, Math::nextUp(lat));
      lon = min(180, Math::nextUp(lon));
      shared_ptr<GeoPoint> point = make_shared<GeoPoint>(
          PlanetModel::WGS84, toRadians(lat), toRadians(lon));
      shared_ptr<GeoPoint> pointEnc = make_shared<GeoPoint>(
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->x)),
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->y)),
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->z)));
      assertTrue(pointEnc->x <= point->x);
      assertTrue(pointEnc->y <= point->y);
      assertTrue(pointEnc->z <= point->z);
    }

    // test below the value
    lat = latBase;
    lon = lonBase;
    for (int i = 0; i < 1000; i++) {
      lat = max(-90, Math::nextDown(lat));
      lon = max(-180, Math::nextDown(lon));
      shared_ptr<GeoPoint> point = make_shared<GeoPoint>(
          PlanetModel::WGS84, toRadians(lat), toRadians(lon));
      shared_ptr<GeoPoint> pointEnc = make_shared<GeoPoint>(
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->x)),
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->y)),
          Geo3DUtil::decodeValueFloor(Geo3DUtil::encodeValue(point->z)));
      assertTrue(pointEnc->x <= point->x);
      assertTrue(pointEnc->y <= point->y);
      assertTrue(pointEnc->z <= point->z);
    }
  }
}

void TestGeo3DPoint::testQuantization() 
{
  shared_ptr<Random> random = TestGeo3DPoint::random();
  for (int i = 0; i < 10000; i++) {
    int encoded = random->nextInt();
    if (encoded < Geo3DUtil::MIN_ENCODED_VALUE) {
      continue;
    }
    if (encoded > Geo3DUtil::MAX_ENCODED_VALUE) {
      continue;
    }
    double min = encoded * Geo3DUtil::DECODE;
    double decoded = Geo3DUtil::decodeValueFloor(encoded);
    // should exactly equal expected value
    assertEquals(min, decoded, 0.0);
    // should round-trip
    TestUtil::assertEquals(encoded, Geo3DUtil::encodeValue(decoded));
    // test within the range
    if (encoded != numeric_limits<int>::max()) {
      // this is the next representable value
      // all double values between [min .. max) should encode to the current
      // integer
      double max = min + Geo3DUtil::DECODE;
      assertEquals(max, Geo3DUtil::decodeValueFloor(encoded + 1), 0.0);
      TestUtil::assertEquals(encoded + 1, Geo3DUtil::encodeValue(max));

      // first and last doubles in range that will be quantized
      double minEdge = Math::nextUp(min);
      double maxEdge = Math::nextDown(max);
      TestUtil::assertEquals(encoded, Geo3DUtil::encodeValue(minEdge));
      TestUtil::assertEquals(encoded, Geo3DUtil::encodeValue(maxEdge));

      // check random values within the double range
      int64_t minBits = NumericUtils::doubleToSortableLong(minEdge);
      int64_t maxBits = NumericUtils::doubleToSortableLong(maxEdge);
      for (int j = 0; j < 100; j++) {
        double value = NumericUtils::sortableLongToDouble(
            TestUtil::nextLong(random, minBits, maxBits));
        // round down
        TestUtil::assertEquals(encoded, Geo3DUtil::encodeValue(value));
      }
    }
  }
}

void TestGeo3DPoint::testEncodeDecodeIsStable() 
{

  int iters = atLeast(1000);
  for (int iter = 0; iter < iters; iter++) {
    double lat = GeoTestUtil::nextLatitude();
    double lon = GeoTestUtil::nextLongitude();

    shared_ptr<GeoPoint> point = make_shared<GeoPoint>(
        PlanetModel::WGS84, toRadians(lat), toRadians(lon));

    // encode point
    shared_ptr<GeoPoint> pointEnc = make_shared<GeoPoint>(
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->x)),
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->y)),
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(point->z)));

    // encode it again (double encode)
    shared_ptr<GeoPoint> pointEnc2 = make_shared<GeoPoint>(
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(pointEnc->x)),
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(pointEnc->y)),
        Geo3DUtil::decodeValue(Geo3DUtil::encodeValue(pointEnc->z)));
    // System.out.println("TEST " + iter + ":\n  point    =" + point + "\n
    // pointEnc =" + pointEnc + "\n  pointEnc2=" + pointEnc2);

    assertEquals(pointEnc->x, pointEnc2->x, 0.0);
    assertEquals(pointEnc->y, pointEnc2->y, 0.0);
    assertEquals(pointEnc->z, pointEnc2->z, 0.0);
  }
}

int TestGeo3DPoint::encodeValueLenient(double x)
{
  double planetMax = PlanetModel::WGS84->getMaximumMagnitude();
  if (x > planetMax) {
    x = planetMax;
  } else if (x < -planetMax) {
    x = -planetMax;
  }
  return Geo3DUtil::encodeValue(x);
}

TestGeo3DPoint::ExplainingVisitor::ExplainingVisitor(
    shared_ptr<GeoShape> shape, shared_ptr<GeoPoint> targetDocPoint,
    shared_ptr<GeoPoint> scaledDocPoint, shared_ptr<IntersectVisitor> in_,
    int targetDocID, int numDims, int bytesPerDim, shared_ptr<StringBuilder> b)
    : shape(shape), targetDocPoint(targetDocPoint),
      scaledDocPoint(scaledDocPoint), in_(in_), targetDocID(targetDocID),
      numDims(numDims), bytesPerDim(bytesPerDim), b(b)
{
}

void TestGeo3DPoint::ExplainingVisitor::startSecondPhase()
{
  if (firstPhase == false) {
    throw make_shared<IllegalStateException>(L"already started second phase");
  }
  if (stackToTargetDoc.empty()) {
    b->append(L"target docID=" + to_wstring(targetDocID) +
              L" was never seen in points!\n");
  }
  firstPhase = false;
  stack.clear();
}

void TestGeo3DPoint::ExplainingVisitor::visit(int docID) 
{
  assert(firstPhase == false);
  if (docID == targetDocID) {
    b->append(L"leaf visit docID=" + to_wstring(docID) + L"\n");
  }
}

void TestGeo3DPoint::ExplainingVisitor::visit(
    int docID, std::deque<char> &packedValue) 
{
  if (firstPhase) {
    if (docID == targetDocID) {
      assert(stackToTargetDoc.empty());
      stackToTargetDoc = deque<>(stack);
      b->append(L"  full BKD path to target doc:\n");
      for (auto cell : stack) {
        b->append(L"    " + cell + L"\n");
      }
    }
  } else {
    if (docID == targetDocID) {
      double x = Geo3DPoint::decodeDimension(packedValue, 0);
      double y = Geo3DPoint::decodeDimension(packedValue, Integer::BYTES);
      double z = Geo3DPoint::decodeDimension(packedValue, 2 * Integer::BYTES);
      b->append(L"leaf visit docID=" + to_wstring(docID) + L" x=" +
                to_wstring(x) + L" y=" + to_wstring(y) + L" z=" +
                to_wstring(z) + L"\n");
      in_->visit(docID, packedValue);
    }
  }
}

Relation
TestGeo3DPoint::ExplainingVisitor::compare(std::deque<char> &minPackedValue,
                                           std::deque<char> &maxPackedValue)
{
  shared_ptr<Cell> cell =
      make_shared<Cell>(shared_from_this(), minPackedValue, maxPackedValue);
  // System.out.println("compare: " + cell);

  // TODO: this is a bit hacky, having to reverse-engineer where we are in the
  // BKD tree's recursion ... but it's the lesser evil vs e.g. polluting this
  // visitor API, or implementing this "under the hood" in BKDReader instead?
  if (firstPhase) {

    // Pop stack:
    while (stack.size() > 0 &&
           stack[stack.size() - 1]->contains(cell) == false) {
      stack.pop_back();
      // System.out.println("  pop");
    }

    // Push stack:
    stack.push_back(cell);
    // System.out.println("  push");

    return Relation::CELL_CROSSES_QUERY;
  } else {
    Relation result = in_->compare(minPackedValue, maxPackedValue);
    if (targetStackUpto < stackToTargetDoc.size() &&
        cell->equals(stackToTargetDoc[targetStackUpto])) {
      b->append(L"  on cell " + stackToTargetDoc[targetStackUpto] +
                L", wrapped visitor returned " + result + L"\n");
      targetStackUpto++;
    }
    return result;
  }
}

TestGeo3DPoint::ExplainingVisitor::Cell::Cell(
    shared_ptr<TestGeo3DPoint::ExplainingVisitor> outerInstance,
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
    : minPackedValue(minPackedValue.clone()),
      maxPackedValue(maxPackedValue.clone()), outerInstance(outerInstance)
{
}

bool TestGeo3DPoint::ExplainingVisitor::Cell::contains(shared_ptr<Cell> other)
{
  for (int dim = 0; dim < outerInstance->numDims; dim++) {
    int offset = outerInstance->bytesPerDim * dim;
    // other.min < this.min?
    if (StringHelper::compare(outerInstance->bytesPerDim, other->minPackedValue,
                              offset, minPackedValue, offset) < 0) {
      return false;
    }
    // other.max < this.max?
    if (StringHelper::compare(outerInstance->bytesPerDim, other->maxPackedValue,
                              offset, maxPackedValue, offset) > 0) {
      return false;
    }
  }

  return true;
}

wstring TestGeo3DPoint::ExplainingVisitor::Cell::toString()
{
  double xMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 0));
  double xMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 0));
  double yMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 1 * Integer::BYTES));
  double yMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 1 * Integer::BYTES));
  double zMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 2 * Integer::BYTES));
  double zMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 2 * Integer::BYTES));
  shared_ptr<XYZSolid> *const xyzSolid = XYZSolidFactory::makeXYZSolid(
      PlanetModel::WGS84, xMin, xMax, yMin, yMax, zMin, zMax);
  constexpr int relationship = xyzSolid->getRelationship(outerInstance->shape);
  constexpr bool pointWithinCell =
      xyzSolid->isWithin(outerInstance->targetDocPoint);
  constexpr bool scaledWithinCell =
      xyzSolid->isWithin(outerInstance->scaledDocPoint);

  const wstring relationshipString;
  switch (relationship) {
  case GeoArea::CONTAINS:
    relationshipString = L"CONTAINS";
    break;
  case GeoArea::WITHIN:
    relationshipString = L"WITHIN";
    break;
  case GeoArea::OVERLAPS:
    relationshipString = L"OVERLAPS";
    break;
  case GeoArea::DISJOINT:
    relationshipString = L"DISJOINT";
    break;
  default:
    relationshipString = L"UNKNOWN";
    break;
  }
  return L"Cell(x=" + to_wstring(xMin) + L" TO " + to_wstring(xMax) + L" y=" +
         to_wstring(yMin) + L" TO " + to_wstring(yMax) + L" z=" +
         to_wstring(zMin) + L" TO " + to_wstring(zMax) +
         L"); Shape relationship = " + relationshipString +
         L"; Quantized point within cell = " +
         StringHelper::toString(pointWithinCell) +
         L"; Unquantized point within cell = " +
         StringHelper::toString(scaledWithinCell);
}

bool TestGeo3DPoint::ExplainingVisitor::Cell::equals(any other)
{
  if (std::dynamic_pointer_cast<Cell>(other) != nullptr == false) {
    return false;
  }

  shared_ptr<Cell> otherCell = any_cast<std::shared_ptr<Cell>>(other);
  return Arrays::equals(minPackedValue, otherCell->minPackedValue) &&
         Arrays::equals(maxPackedValue, otherCell->maxPackedValue);
}

int TestGeo3DPoint::ExplainingVisitor::Cell::hashCode()
{
  return Arrays::hashCode(minPackedValue) + Arrays::hashCode(maxPackedValue);
}

wstring TestGeo3DPoint::explain(const wstring &fieldName,
                                shared_ptr<GeoShape> shape,
                                shared_ptr<GeoPoint> targetDocPoint,
                                shared_ptr<GeoPoint> scaledDocPoint,
                                shared_ptr<IndexReader> reader,
                                int docID) 
{

  shared_ptr<XYZBounds> *const bounds = make_shared<XYZBounds>();
  shape->getBounds(bounds);

  // First find the leaf reader that owns this doc:
  int subIndex = ReaderUtil::subIndex(docID, reader->leaves());
  shared_ptr<LeafReader> leafReader = reader->leaves()[subIndex]->reader();

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"target is in leaf " + leafReader + L" of full reader " + reader +
            L"\n");

  shared_ptr<DocIdSetBuilder> hits =
      make_shared<DocIdSetBuilder>(leafReader->maxDoc());
  shared_ptr<ExplainingVisitor> visitor = make_shared<ExplainingVisitor>(
      shape, targetDocPoint, scaledDocPoint,
      make_shared<PointInShapeIntersectVisitor>(hits, shape, bounds),
      docID - reader->leaves()[subIndex]->docBase, 3, Integer::BYTES, b);

  // Do first phase, where we just figure out the "path" that leads to the
  // target docID:
  leafReader->getPointValues(fieldName)->intersect(visitor);

  // Do second phase, where we we see how the wrapped visitor responded along
  // that path:
  visitor->startSecondPhase();
  leafReader->getPointValues(fieldName)->intersect(visitor);

  return b->toString();
}
} // namespace org::apache::lucene::spatial3d