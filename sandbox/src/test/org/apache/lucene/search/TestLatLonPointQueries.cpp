using namespace std;

#include "TestLatLonPointQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LatLonPoint = org::apache::lucene::document::LatLonPoint;
using BaseGeoPointTestCase = org::apache::lucene::geo::BaseGeoPointTestCase;
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using Polygon = org::apache::lucene::geo::Polygon;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

void TestLatLonPointQueries::addPointToDoc(const wstring &field,
                                           shared_ptr<Document> doc, double lat,
                                           double lon)
{
  doc->push_back(make_shared<LatLonPoint>(field, lat, lon));
}

shared_ptr<Query> TestLatLonPointQueries::newRectQuery(const wstring &field,
                                                       double minLat,
                                                       double maxLat,
                                                       double minLon,
                                                       double maxLon)
{
  return LatLonPoint::newBoxQuery(field, minLat, maxLat, minLon, maxLon);
}

shared_ptr<Query> TestLatLonPointQueries::newDistanceQuery(const wstring &field,
                                                           double centerLat,
                                                           double centerLon,
                                                           double radiusMeters)
{
  return LatLonPoint::newDistanceQuery(field, centerLat, centerLon,
                                       radiusMeters);
}

shared_ptr<Query>
TestLatLonPointQueries::newPolygonQuery(const wstring &field,
                                        deque<Polygon> &polygons)
{
  return LatLonPoint::newPolygonQuery(field, {polygons});
}

double TestLatLonPointQueries::quantizeLat(double latRaw)
{
  return GeoEncodingUtils::decodeLatitude(
      GeoEncodingUtils::encodeLatitude(latRaw));
}

double TestLatLonPointQueries::quantizeLon(double lonRaw)
{
  return GeoEncodingUtils::decodeLongitude(
      GeoEncodingUtils::encodeLongitude(lonRaw));
}

void TestLatLonPointQueries::testDistanceQueryWithInvertedIntersection() throw(
    IOException)
{
  constexpr int numMatchingDocs =
      atLeast(10 * BKDWriter::DEFAULT_MAX_POINTS_IN_LEAF_NODE);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory())
  {
    org::apache::lucene::store::Directory dir = newDirectory();

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter w =
    // new org.apache.lucene.index.IndexWriter(dir, newIndexWriterConfig()))
    {
      org::apache::lucene::index::IndexWriter w =
          org::apache::lucene::index::IndexWriter(dir, newIndexWriterConfig());
      for (int i = 0; i < numMatchingDocs; ++i) {
        shared_ptr<Document> doc = make_shared<Document>();
        addPointToDoc(L"field", doc, 18.313694, -65.227444);
        w->addDocument(doc);
      }

      // Add a handful of docs that don't match
      for (int i = 0; i < 11; ++i) {
        shared_ptr<Document> doc = make_shared<Document>();
        addPointToDoc(L"field", doc, 10, -65.227444);
        w->addDocument(doc);
      }
      w->forceMerge(1);
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader r =
    // org.apache.lucene.index.DirectoryReader.open(dir))
    {
      org::apache::lucene::index::IndexReader r =
          org::apache::lucene::index::DirectoryReader::open(dir);
      shared_ptr<IndexSearcher> searcher = newSearcher(r);
      assertEquals(numMatchingDocs, searcher->count(newDistanceQuery(
                                        L"field", 18, -65, 50'000)));
    }
  }
}
} // namespace org::apache::lucene::search