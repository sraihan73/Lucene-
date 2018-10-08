using namespace std;

#include "LatLonPointInPolygonQuery.h"

namespace org::apache::lucene::document
{
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using Polygon = org::apache::lucene::geo::Polygon;
using Polygon2D = org::apache::lucene::geo::Polygon2D;
using Rectangle = org::apache::lucene::geo::Rectangle;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

LatLonPointInPolygonQuery::LatLonPointInPolygonQuery(
    const wstring &field, std::deque<std::shared_ptr<Polygon>> &polygons)
    : field(field), polygons(polygons.clone())
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (polygons.empty()) {
    throw invalid_argument(L"polygons must not be null");
  }
  if (polygons.empty()) {
    throw invalid_argument(L"polygons must not be empty");
  }
  for (int i = 0; i < polygons.size(); i++) {
    if (polygons[i] == nullptr) {
      throw invalid_argument(L"polygon[" + to_wstring(i) +
                             L"] must not be null");
    }
  }
  // TODO: we could also compute the maximal inner bounding box, to make
  // relations faster to compute?
}

shared_ptr<Weight>
LatLonPointInPolygonQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                        bool needsScores,
                                        float boost) 
{

  // I don't use RandomAccessWeight here: it's no good to approximate with
  // "match all docs"; this is an inverted structure and should be used in the
  // first pass:

  // bounding box over all polygons, this can speed up tree intersection/cheaply
  // improve approximation for complex multi-polygons these are pre-encoded with
  // LatLonPoint's encoding
  shared_ptr<Rectangle> *const box = Rectangle::fromPolygon(polygons);
  const std::deque<char> minLat = std::deque<char>(Integer::BYTES);
  const std::deque<char> maxLat = std::deque<char>(Integer::BYTES);
  const std::deque<char> minLon = std::deque<char>(Integer::BYTES);
  const std::deque<char> maxLon = std::deque<char>(Integer::BYTES);
  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLatitude(box->minLat), minLat, 0);
  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLatitude(box->maxLat), maxLat, 0);
  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLongitude(box->minLon), minLon, 0);
  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLongitude(box->maxLon), maxLon, 0);

  shared_ptr<Polygon2D> *const tree = Polygon2D::create(polygons);
  shared_ptr<GeoEncodingUtils::PolygonPredicate> *const polygonPredicate =
      GeoEncodingUtils::createPolygonPredicate(polygons, tree);

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), boost, minLat, maxLat, minLon, maxLon, tree,
      polygonPredicate);
}

LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<LatLonPointInPolygonQuery> outerInstance, float boost,
        deque<char> &minLat, deque<char> &maxLat, deque<char> &minLon,
        deque<char> &maxLon, shared_ptr<Polygon2D> tree,
        shared_ptr<GeoEncodingUtils::PolygonPredicate> polygonPredicate)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->minLat = minLat;
  this->maxLat = maxLat;
  this->minLon = minLon;
  this->maxLon = maxLon;
  this->tree = tree;
  this->polygonPredicate = polygonPredicate;
}

shared_ptr<Scorer>
LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<PointValues> values = reader->getPointValues(outerInstance->field);
  if (values == nullptr) {
    // No docs in this segment had any points fields
    return nullptr;
  }
  shared_ptr<FieldInfo> fieldInfo =
      reader->getFieldInfos()->fieldInfo(outerInstance->field);
  if (fieldInfo == nullptr) {
    // No docs in this segment indexed this field at all
    return nullptr;
  }
  LatLonPoint::checkCompatible(fieldInfo);

  // matching docids
  shared_ptr<DocIdSetBuilder> result = make_shared<DocIdSetBuilder>(
      reader->maxDoc(), values, outerInstance->field);

  values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), result));

  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          result->build()->begin());
}

LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetBuilder> result)
{
  this->outerInstance = outerInstance;
  this->result = result;
}

void LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::grow(int count)
{
  adder = result->grow(count);
}

void LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  adder->add(docID);
}

void LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID,
                                               std::deque<char> &packedValue)
{
  if (outerInstance->polygonPredicate.test(
          NumericUtils::sortableBytesToInt(packedValue, 0),
          NumericUtils::sortableBytesToInt(packedValue, Integer::BYTES))) {
    adder->add(docID);
  }
}

PointValues::Relation
LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (StringHelper::compare(Integer::BYTES, minPackedValue, 0,
                            outerInstance->maxLat, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, maxPackedValue, 0,
                            outerInstance->minLat, 0) < 0 ||
      StringHelper::compare(Integer::BYTES, minPackedValue, Integer::BYTES,
                            outerInstance->maxLon, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, maxPackedValue, Integer::BYTES,
                            outerInstance->minLon, 0) < 0) {
    // outside of global bounding box range
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  }

  double cellMinLat = GeoEncodingUtils::decodeLatitude(minPackedValue, 0);
  double cellMinLon =
      GeoEncodingUtils::decodeLongitude(minPackedValue, Integer::BYTES);
  double cellMaxLat = GeoEncodingUtils::decodeLatitude(maxPackedValue, 0);
  double cellMaxLon =
      GeoEncodingUtils::decodeLongitude(maxPackedValue, Integer::BYTES);

  return outerInstance->tree.relate(cellMinLat, cellMaxLat, cellMinLon,
                                    cellMaxLon);
}

bool LatLonPointInPolygonQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

wstring LatLonPointInPolygonQuery::getField() { return field; }

std::deque<std::shared_ptr<Polygon>> LatLonPointInPolygonQuery::getPolygons()
{
  return polygons.clone();
}

int LatLonPointInPolygonQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + field.hashCode();
  result = prime * result + Arrays::hashCode(polygons);
  return result;
}

bool LatLonPointInPolygonQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool LatLonPointInPolygonQuery::equalsTo(
    shared_ptr<LatLonPointInPolygonQuery> other)
{
  return field == other->field && Arrays::equals(polygons, other->polygons);
}

wstring LatLonPointInPolygonQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  sb->append(getClass().getSimpleName());
  sb->append(L':');
  if (this->field == field == false) {
    sb->append(L" field=");
    sb->append(this->field);
    sb->append(L':');
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(Arrays->toString(polygons));
  return sb->toString();
}
} // namespace org::apache::lucene::document