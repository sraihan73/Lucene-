using namespace std;

#include "TestNearest.h"

namespace org::apache::lucene::document
{
using Codec = org::apache::lucene::codecs::Codec;
using NearestHit = org::apache::lucene::document::NearestNeighbor::NearestHit;
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using GeoTestUtil = org::apache::lucene::geo::GeoTestUtil;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using SloppyMath = org::apache::lucene::util::SloppyMath;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestNearest::testNearestNeighborWithDeletedDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 40.0, 50.0));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 45.0, 55.0));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FieldDoc> hit = std::static_pointer_cast<FieldDoc>(
      LatLonPoint::nearest(s, L"point", 40.0, 50.0, 1)->scoreDocs[0]);
  TestUtil::assertEquals(L"0",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();

  w->deleteDocuments(make_shared<Term>(L"id", L"0"));
  r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  s = newSearcher(r, false);
  hit = std::static_pointer_cast<FieldDoc>(
      LatLonPoint::nearest(s, L"point", 40.0, 50.0, 1)->scoreDocs[0]);
  TestUtil::assertEquals(L"1",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();
  delete w;
  delete dir;
}

void TestNearest::testNearestNeighborWithAllDeletedDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 40.0, 50.0));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 45.0, 55.0));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FieldDoc> hit = std::static_pointer_cast<FieldDoc>(
      LatLonPoint::nearest(s, L"point", 40.0, 50.0, 1)->scoreDocs[0]);
  TestUtil::assertEquals(L"0",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();

  w->deleteDocuments(make_shared<Term>(L"id", L"0"));
  w->deleteDocuments(make_shared<Term>(L"id", L"1"));
  r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  s = newSearcher(r, false);
  TestUtil::assertEquals(
      0, LatLonPoint::nearest(s, L"point", 40.0, 50.0, 1)->scoreDocs.size());
  r->close();
  delete w;
  delete dir;
}

void TestNearest::testTieBreakByDocID() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 40.0, 50.0));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<LatLonPoint>(L"point", 40.0, 50.0));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      LatLonPoint::nearest(newSearcher(r, false), L"point", 45.0, 50.0, 2)
          ->scoreDocs;
  TestUtil::assertEquals(
      L"0", r->document(hits[0]->doc).getField(L"id").stringValue());
  TestUtil::assertEquals(
      L"1", r->document(hits[1]->doc).getField(L"id").stringValue());

  r->close();
  delete w;
  delete dir;
}

void TestNearest::testNearestNeighborWithNoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  TestUtil::assertEquals(
      0, LatLonPoint::nearest(newSearcher(r, false), L"point", 40.0, 50.0, 1)
             ->scoreDocs.size());
  r->close();
  delete w;
  delete dir;
}

double TestNearest::quantizeLat(double latRaw)
{
  return GeoEncodingUtils::decodeLatitude(
      GeoEncodingUtils::encodeLatitude(latRaw));
}

double TestNearest::quantizeLon(double lonRaw)
{
  return GeoEncodingUtils::decodeLongitude(
      GeoEncodingUtils::encodeLongitude(lonRaw));
}

void TestNearest::testNearestNeighborRandom() 
{

  int numPoints = atLeast(5000);
  shared_ptr<Directory> dir;
  if (numPoints > 100000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }
  std::deque<double> lats(numPoints);
  std::deque<double> lons(numPoints);

  shared_ptr<IndexWriterConfig> iwc = getIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  for (int id = 0; id < numPoints; id++) {
    lats[id] = quantizeLat(GeoTestUtil::nextLatitude());
    lons[id] = quantizeLon(GeoTestUtil::nextLongitude());
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<LatLonPoint>(L"point", lats[id], lons[id]));
    doc->push_back(
        make_shared<LatLonDocValuesField>(L"point", lats[id], lons[id]));
    doc->push_back(make_shared<StoredField>(L"id", id));
    w->addDocument(doc);
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }

  shared_ptr<DirectoryReader> r = w->getReader();
  if (VERBOSE) {
    wcout << L"TEST: reader=" << r << endl;
  }
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    double pointLat = GeoTestUtil::nextLatitude();
    double pointLon = GeoTestUtil::nextLongitude();

    // dumb brute force search to get the expected result:
    std::deque<std::shared_ptr<NearestHit>> expectedHits(lats.size());
    for (int id = 0; id < lats.size(); id++) {
      shared_ptr<NearestHit> hit = make_shared<NearestHit>();
      hit->distanceMeters =
          SloppyMath::haversinMeters(pointLat, pointLon, lats[id], lons[id]);
      hit->docID = id;
      expectedHits[id] = hit;
    }

    Arrays::sort(expectedHits, make_shared<ComparatorAnonymousInnerClass>(
                                   shared_from_this()));

    int topN = TestUtil::nextInt(random(), 1, lats.size());

    if (VERBOSE) {
      wcout << L"\nhits for pointLat=" << pointLat << L" pointLon=" << pointLon
            << endl;
    }

    // Also test with MatchAllDocsQuery, sorting by distance:
    shared_ptr<TopFieldDocs> fieldDocs =
        s->search(make_shared<MatchAllDocsQuery>(), topN,
                  make_shared<Sort>(LatLonDocValuesField::newDistanceSort(
                      L"point", pointLat, pointLon)));

    std::deque<std::shared_ptr<ScoreDoc>> hits =
        LatLonPoint::nearest(s, L"point", pointLat, pointLon, topN)->scoreDocs;
    for (int i = 0; i < topN; i++) {
      shared_ptr<NearestHit> expected = expectedHits[i];
      shared_ptr<FieldDoc> expected2 =
          std::static_pointer_cast<FieldDoc>(fieldDocs->scoreDocs[i]);
      shared_ptr<FieldDoc> actual = std::static_pointer_cast<FieldDoc>(hits[i]);
      shared_ptr<Document> actualDoc = r->document(actual->doc);

      if (VERBOSE) {
        wcout << L"hit " << i << endl;
        wcout << L"  expected id=" << expected->docID << L" lat="
              << lats[expected->docID] << L" lon=" << lons[expected->docID]
              << L" distance=" << expected->distanceMeters << L" meters"
              << endl;
        wcout << L"  actual id=" << actualDoc->getField(L"id") << L" distance="
              << actual->fields[0] << L" meters" << endl;
      }

      TestUtil::assertEquals(expected->docID, actual->doc);
      assertEquals(expected->distanceMeters,
                   (any_cast<optional<double>>(actual->fields[0])).value(),
                   0.0);

      TestUtil::assertEquals(expected->docID, expected->docID);
      assertEquals((any_cast<optional<double>>(expected2->fields[0])).value(),
                   expected->distanceMeters, 0.0);
    }
  }

  r->close();
  delete w;
  delete dir;
}

TestNearest::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestNearest> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestNearest::ComparatorAnonymousInnerClass::compare(
    shared_ptr<NearestHit> a, shared_ptr<NearestHit> b)
{
  int cmp = Double::compare(a->distanceMeters, b->distanceMeters);
  if (cmp != 0) {
    return cmp;
  }
  // tie break by smaller docID:
  return a->docID - b->docID;
}

shared_ptr<IndexWriterConfig> TestNearest::getIndexWriterConfig()
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(Codec::forName(L"Lucene70"));
  return iwc;
}
} // namespace org::apache::lucene::document