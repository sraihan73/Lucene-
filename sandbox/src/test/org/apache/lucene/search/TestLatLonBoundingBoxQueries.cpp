using namespace std;

#include "TestLatLonBoundingBoxQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LatLonBoundingBox = org::apache::lucene::document::LatLonBoundingBox;
using GeoTestUtil = org::apache::lucene::geo::GeoTestUtil;
using Rectangle = org::apache::lucene::geo::Rectangle;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
const wstring TestLatLonBoundingBoxQueries::FIELD_NAME = L"geoBoundingBoxField";

shared_ptr<LatLonBoundingBox>
TestLatLonBoundingBoxQueries::newRangeField(shared_ptr<Range> r)
{
  // addRange is called instead of this method
  throw make_shared<UnsupportedOperationException>(
      L"this method should never be called");
}

void TestLatLonBoundingBoxQueries::addRange(shared_ptr<Document> doc,
                                            shared_ptr<Range> r)
{
  shared_ptr<GeoBBox> b = std::static_pointer_cast<GeoBBox>(r);
  doc->push_back(make_shared<LatLonBoundingBox>(
      FIELD_NAME, b->minLat, b->minLon, b->maxLat, b->maxLon));
}

void TestLatLonBoundingBoxQueries::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // Shared meridian test (disjoint)
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<LatLonBoundingBox>(FIELD_NAME, -20, -180, 20, -100));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LatLonBoundingBox>(
      FIELD_NAME, 0, 14.096488952636719, 10, 20));
  writer->addDocument(document);

  // intersects (contains, crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LatLonBoundingBox>(
      FIELD_NAME, -10.282592503353953, -1, 1, 14.096488952636719));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<LatLonBoundingBox>(FIELD_NAME, -1, -11, 1, 1));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LatLonBoundingBox>(
      FIELD_NAME, -1, 14.096488952636719, 5, 30));
  writer->addDocument(document);

  // intersects (within)
  document = make_shared<Document>();
  document->push_back(make_shared<LatLonBoundingBox>(FIELD_NAME, -5, 0, -1,
                                                     14.096488952636719));
  writer->addDocument(document);

  // search
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(
      5, searcher->count(LatLonBoundingBox::newIntersectsQuery(
             FIELD_NAME, -10.282592503353953, 0.0, 0.0, 14.096488952636719)));
  assertEquals(
      1, searcher->count(LatLonBoundingBox::newWithinQuery(
             FIELD_NAME, -10.282592503353953, 0.0, 0.0, 14.096488952636719)));
  assertEquals(
      1, searcher->count(LatLonBoundingBox::newContainsQuery(
             FIELD_NAME, -10.282592503353953, 0.0, 0.0, 14.096488952636719)));
  assertEquals(
      4, searcher->count(LatLonBoundingBox::newCrossesQuery(
             FIELD_NAME, -10.282592503353953, 0.0, 0.0, 14.096488952636719)));

  delete reader;
  delete writer;
  delete dir;
}

void TestLatLonBoundingBoxQueries::testToString()
{
  shared_ptr<LatLonBoundingBox> field =
      make_shared<LatLonBoundingBox>(FIELD_NAME, -20, -180, 20, -100);
  wstring expected = L"LatLonBoundingBox "
                     L"<geoBoundingBoxField:[-20.000000023283064,-180.0,19."
                     L"99999998137355,-100.0000000745058]>";
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(expected, field->toString());
}

int TestLatLonBoundingBoxQueries::dimension() { return 2; }

shared_ptr<Range> TestLatLonBoundingBoxQueries::nextRange(int dimensions)
{
  // create a random bounding box in 2 dimensions
  return make_shared<GeoBBox>(dimensions);
}

shared_ptr<Query>
TestLatLonBoundingBoxQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  shared_ptr<GeoBBox> b = std::static_pointer_cast<GeoBBox>(r);
  return LatLonBoundingBox::newIntersectsQuery(FIELD_NAME, b->minLat, b->minLon,
                                               b->maxLat, b->maxLon);
}

shared_ptr<Query>
TestLatLonBoundingBoxQueries::newContainsQuery(shared_ptr<Range> r)
{
  shared_ptr<GeoBBox> b = std::static_pointer_cast<GeoBBox>(r);
  return LatLonBoundingBox::newContainsQuery(FIELD_NAME, b->minLat, b->minLon,
                                             b->maxLat, b->maxLon);
}

shared_ptr<Query>
TestLatLonBoundingBoxQueries::newWithinQuery(shared_ptr<Range> r)
{
  shared_ptr<GeoBBox> b = std::static_pointer_cast<GeoBBox>(r);
  return LatLonBoundingBox::newWithinQuery(FIELD_NAME, b->minLat, b->minLon,
                                           b->maxLat, b->maxLon);
}

shared_ptr<Query>
TestLatLonBoundingBoxQueries::newCrossesQuery(shared_ptr<Range> r)
{
  shared_ptr<GeoBBox> b = std::static_pointer_cast<GeoBBox>(r);
  return LatLonBoundingBox::newCrossesQuery(FIELD_NAME, b->minLat, b->minLon,
                                            b->maxLat, b->maxLon);
}

TestLatLonBoundingBoxQueries::GeoBBox::GeoBBox(int dimension)
{
  this->dimension = dimension;
  shared_ptr<Rectangle> *const box = GeoTestUtil::nextBoxNotCrossingDateline();
  minLat = quantizeLat(box->minLat);
  minLon = quantizeLon(box->minLon);
  maxLat = quantizeLat(box->maxLat);
  maxLon = quantizeLon(box->maxLon);

  //      minLat = quantizeLat(Math.min(box.minLat, box.maxLat));
  //      minLon = quantizeLon(Math.max(box.minLat, box.maxLat));
  //      maxLat = quantizeLat(box.maxLat);
  //      maxLon = quantizeLon(box.maxLon);

  //      if (maxLon == -180d) {
  //        // index and search handle this fine, but the test validator
  //        // struggles when maxLon == -180; so lets correct
  //        maxLon = 180d;
  //      }
}

double TestLatLonBoundingBoxQueries::GeoBBox::quantizeLat(double lat)
{
  return decodeLatitude(encodeLatitude(lat));
}

double TestLatLonBoundingBoxQueries::GeoBBox::quantizeLon(double lon)
{
  return decodeLongitude(encodeLongitude(lon));
}

int TestLatLonBoundingBoxQueries::GeoBBox::numDimensions() { return dimension; }

optional<double> TestLatLonBoundingBoxQueries::GeoBBox::getMin(int dim)
{
  if (dim == 0) {
    return minLat;
  } else if (dim == 1) {
    return minLon;
  }
  throw out_of_range(L"dimension " + to_wstring(dim) + L" is greater than " +
                     to_wstring(dimension));
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMin(int dim, any val)
{
  if (dim == 0) {
    setMinLat(any_cast<optional<double>>(val));
  } else if (dim == 1) {
    setMinLon(any_cast<optional<double>>(val));
  } else {
    throw out_of_range(L"dimension " + to_wstring(dim) + L" is greater than " +
                       to_wstring(dimension));
  }
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMinLat(double d)
{
  if (d > maxLat) {
    minLat = maxLat;
    maxLat = d;
  } else {
    minLat = d;
  }
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMinLon(double d)
{
  if (d > maxLon) {
    minLon = maxLon;
    maxLon = d;
  } else {
    minLon = d;
  }
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMaxLat(double d)
{
  if (d < minLat) {
    maxLat = minLat;
    minLat = d;
  } else {
    maxLat = d;
  }
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMaxLon(double d)
{
  if (d < minLon) {
    maxLon = minLon;
    minLon = d;
  } else {
    maxLon = d;
  }
}

optional<double> TestLatLonBoundingBoxQueries::GeoBBox::getMax(int dim)
{
  if (dim == 0) {
    return maxLat;
  } else if (dim == 1) {
    return maxLon;
  }
  throw out_of_range(L"dimension " + to_wstring(dim) + L" is greater than " +
                     to_wstring(dimension));
}

void TestLatLonBoundingBoxQueries::GeoBBox::setMax(int dim, any val)
{
  if (dim == 0) {
    setMaxLat(any_cast<optional<double>>(val));
  } else if (dim == 1) {
    setMaxLon(any_cast<optional<double>>(val));
  } else {
    throw out_of_range(L"dimension " + to_wstring(dim) + L" is greater than " +
                       to_wstring(dimension));
  }
}

bool TestLatLonBoundingBoxQueries::GeoBBox::isEqual(shared_ptr<Range> other)
{
  shared_ptr<GeoBBox> o = std::static_pointer_cast<GeoBBox>(other);
  if (this->dimension != o->dimension) {
    return false;
  }
  if (this->minLat != o->minLat) {
    return false;
  }
  if (this->minLon != o->minLon) {
    return false;
  }
  if (this->maxLat != o->maxLat) {
    return false;
  }
  if (this->maxLon != o->maxLon) {
    return false;
  }
  return true;
}

bool TestLatLonBoundingBoxQueries::GeoBBox::isDisjoint(shared_ptr<Range> other)
{
  shared_ptr<GeoBBox> o = std::static_pointer_cast<GeoBBox>(other);
  if (minLat > o->maxLat || maxLat < o->minLat) {
    return true;
  }
  if (minLon > o->maxLon || maxLon < o->minLon) {
    return true;
  }
  return false;
}

bool TestLatLonBoundingBoxQueries::GeoBBox::isWithin(shared_ptr<Range> other)
{
  shared_ptr<GeoBBox> o = std::static_pointer_cast<GeoBBox>(other);
  return o->contains(shared_from_this());
}

bool TestLatLonBoundingBoxQueries::GeoBBox::contains(shared_ptr<Range> other)
{
  shared_ptr<GeoBBox> o = std::static_pointer_cast<GeoBBox>(other);
  if (minLat > o->minLat || maxLat < o->maxLat) {
    return false;
  }
  if (minLon > o->minLon || maxLon < o->maxLon) {
    return false;
  }
  return true;
}

wstring TestLatLonBoundingBoxQueries::GeoBBox::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"GeoBoundingBox(lat: ");
  b->append(minLat);
  b->append(L" TO ");
  b->append(maxLat);
  b->append(L", lon: ");
  b->append(minLon);
  b->append(L" TO ");
  b->append(maxLon);
  b->append(L")");

  return b->toString();
}
} // namespace org::apache::lucene::search