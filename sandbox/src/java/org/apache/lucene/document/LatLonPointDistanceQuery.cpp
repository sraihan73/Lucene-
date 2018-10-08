using namespace std;

#include "LatLonPointDistanceQuery.h"

namespace org::apache::lucene::document
{
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using GeoUtils = org::apache::lucene::geo::GeoUtils;
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
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using ScorerSupplier = org::apache::lucene::search::ScorerSupplier;
using Weight = org::apache::lucene::search::Weight;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

LatLonPointDistanceQuery::LatLonPointDistanceQuery(const wstring &field,
                                                   double latitude,
                                                   double longitude,
                                                   double radiusMeters)
    : field(field), latitude(latitude), longitude(longitude),
      radiusMeters(radiusMeters)
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (Double::isFinite(radiusMeters) == false || radiusMeters < 0) {
    throw invalid_argument(L"radiusMeters: '" + to_wstring(radiusMeters) +
                           L"' is invalid");
  }
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
}

shared_ptr<Weight>
LatLonPointDistanceQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost) 
{
  shared_ptr<Rectangle> box =
      Rectangle::fromPointDistance(latitude, longitude, radiusMeters);
  // create bounding box(es) for the distance range
  // these are pre-encoded with LatLonPoint's encoding
  const std::deque<char> minLat = std::deque<char>(Integer::BYTES);
  const std::deque<char> maxLat = std::deque<char>(Integer::BYTES);
  const std::deque<char> minLon = std::deque<char>(Integer::BYTES);
  const std::deque<char> maxLon = std::deque<char>(Integer::BYTES);
  // second set of longitude ranges to check (for cross-dateline case)
  const std::deque<char> minLon2 = std::deque<char>(Integer::BYTES);

  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLatitude(box->minLat), minLat, 0);
  NumericUtils::intToSortableBytes(
      GeoEncodingUtils::encodeLatitude(box->maxLat), maxLat, 0);

  // crosses dateline: split
  if (box->crossesDateline()) {
    // box1
    NumericUtils::intToSortableBytes(numeric_limits<int>::min(), minLon, 0);
    NumericUtils::intToSortableBytes(
        GeoEncodingUtils::encodeLongitude(box->maxLon), maxLon, 0);
    // box2
    NumericUtils::intToSortableBytes(
        GeoEncodingUtils::encodeLongitude(box->minLon), minLon2, 0);
  } else {
    NumericUtils::intToSortableBytes(
        GeoEncodingUtils::encodeLongitude(box->minLon), minLon, 0);
    NumericUtils::intToSortableBytes(
        GeoEncodingUtils::encodeLongitude(box->maxLon), maxLon, 0);
    // disable box2
    NumericUtils::intToSortableBytes(numeric_limits<int>::max(), minLon2, 0);
  }

  // compute exact sort key: avoid any asin() computations
  constexpr double sortKey = GeoUtils::distanceQuerySortKey(radiusMeters);

  constexpr double axisLat = Rectangle::axisLat(latitude, radiusMeters);

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(
      shared_from_this(), boost, minLat, maxLat, minLon, maxLon, minLon2,
      sortKey, axisLat);
}

LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<LatLonPointDistanceQuery> outerInstance, float boost,
        deque<char> &minLat, deque<char> &maxLat, deque<char> &minLon,
        deque<char> &maxLon, deque<char> &minLon2, double sortKey,
        double axisLat)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->minLat = minLat;
  this->maxLat = maxLat;
  this->minLon = minLon;
  this->maxLon = maxLon;
  this->minLon2 = minLon2;
  this->sortKey = sortKey;
  this->axisLat = axisLat;
  distancePredicate = GeoEncodingUtils::createDistancePredicate(
      outerInstance->latitude, outerInstance->longitude,
      outerInstance->radiusMeters);
}

shared_ptr<Scorer>
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<ScorerSupplier>
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    scorerSupplier(shared_ptr<LeafReaderContext> context) 
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
  shared_ptr<PointValues::IntersectVisitor> *const visitor =
      getIntersectVisitor(result);

  shared_ptr<Weight> *const weight = shared_from_this();
  return make_shared<ScorerSupplierAnonymousInnerClass>(
      shared_from_this(), reader, values, result, visitor, weight);
}

LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReader> reader, shared_ptr<PointValues> values,
        shared_ptr<DocIdSetBuilder> result,
        shared_ptr<PointValues::IntersectVisitor> visitor,
        shared_ptr<Weight> weight)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->values = values;
  this->result = result;
  this->visitor = visitor;
  this->weight = weight;
  cost = -1;
}

shared_ptr<Scorer>
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::get(int64_t leadCost) throw(
        IOException)
{
  if (values->getDocCount() == reader->maxDoc() &&
      values->getDocCount() == values->size() &&
      cost() > reader->maxDoc() / 2) {
    // If all docs have exactly one value and the cost is greater
    // than half the leaf size then maybe we can make things faster
    // by computing the set of documents that do NOT match the range
    shared_ptr<FixedBitSet> *const result =
        make_shared<FixedBitSet>(reader->maxDoc());
    result->set(0, reader->maxDoc());
    std::deque<int> cost = {reader->maxDoc()};
    values->intersect(getInverseIntersectVisitor(result, cost));
    shared_ptr<DocIdSetIterator> *const iterator =
        make_shared<BitSetIterator>(result, cost[0]);
    return make_shared<ConstantScoreScorer>(weight, score(), iterator);
  }
  values->intersect(visitor);
  return make_shared<ConstantScoreScorer>(weight, score(),
                                          result->build()->begin());
}

int64_t LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::cost()
{
  if (cost == -1) {
    cost = values->estimatePointCount(visitor);
  }
  assert(cost >= 0);
  return cost;
}

shared_ptr<PointValues::IntersectVisitor>
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    getIntersectVisitor(shared_ptr<DocIdSetBuilder> result)
{
  return make_shared<IntersectVisitorAnonymousInnerClass>(shared_from_this(),
                                                          result);
}

LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetBuilder> result)
{
  this->outerInstance = outerInstance;
  this->result = result;
}

void LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::grow(int count)
{
  adder = result->grow(count);
}

void LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  adder->add(docID);
}

void LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID,
                                               std::deque<char> &packedValue)
{
  // bounding box check
  if (StringHelper::compare(Integer::BYTES, packedValue, 0,
                            outerInstance->maxLat, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, packedValue, 0,
                            outerInstance->minLat, 0) < 0) {
    // latitude out of bounding box range
    return;
  }

  if ((StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                             outerInstance->maxLon, 0) > 0 ||
       StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                             outerInstance->minLon, 0) < 0) &&
      StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                            outerInstance->minLon2, 0) < 0) {
    // longitude out of bounding box range
    return;
  }

  int docLatitude = NumericUtils::sortableBytesToInt(packedValue, 0);
  int docLongitude =
      NumericUtils::sortableBytesToInt(packedValue, Integer::BYTES);
  if (distancePredicate::test(docLatitude, docLongitude)) {
    adder->add(docID);
  }
}

PointValues::Relation
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (StringHelper::compare(Integer::BYTES, minPackedValue, 0,
                            outerInstance->maxLat, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, maxPackedValue, 0,
                            outerInstance->minLat, 0) < 0) {
    // latitude out of bounding box range
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  }

  if ((StringHelper::compare(Integer::BYTES, minPackedValue, Integer::BYTES,
                             outerInstance->maxLon, 0) > 0 ||
       StringHelper::compare(Integer::BYTES, maxPackedValue, Integer::BYTES,
                             outerInstance->minLon, 0) < 0) &&
      StringHelper::compare(Integer::BYTES, maxPackedValue, Integer::BYTES,
                            outerInstance->minLon2, 0) < 0) {
    // longitude out of bounding box range
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  }

  double latMin = GeoEncodingUtils::decodeLatitude(minPackedValue, 0);
  double lonMin =
      GeoEncodingUtils::decodeLongitude(minPackedValue, Integer::BYTES);
  double latMax = GeoEncodingUtils::decodeLatitude(maxPackedValue, 0);
  double lonMax =
      GeoEncodingUtils::decodeLongitude(maxPackedValue, Integer::BYTES);

  return GeoUtils::relate(latMin, latMax, lonMin, lonMax,
                          outerInstance->outerInstance.latitude,
                          outerInstance->outerInstance.longitude,
                          outerInstance->sortKey, outerInstance->axisLat);
}

shared_ptr<PointValues::IntersectVisitor>
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    getInverseIntersectVisitor(shared_ptr<FixedBitSet> result,
                               std::deque<int> &cost)
{
  return make_shared<IntersectVisitorAnonymousInnerClass2>(shared_from_this(),
                                                           result, cost);
}

LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::IntersectVisitorAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<FixedBitSet> result, deque<int> &cost)
{
  this->outerInstance = outerInstance;
  this->result = result;
  this->cost = cost;
}

void LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::visit(int docID)
{
  result->clear(docID);
  cost[0]--;
}

void LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::visit(int docID,
                                                std::deque<char> &packedValue)
{
  // bounding box check
  if (StringHelper::compare(Integer::BYTES, packedValue, 0,
                            outerInstance->maxLat, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, packedValue, 0,
                            outerInstance->minLat, 0) < 0) {
    // latitude out of bounding box range
    result->clear(docID);
    cost[0]--;
    return;
  }

  if ((StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                             outerInstance->maxLon, 0) > 0 ||
       StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                             outerInstance->minLon, 0) < 0) &&
      StringHelper::compare(Integer::BYTES, packedValue, Integer::BYTES,
                            outerInstance->minLon2, 0) < 0) {
    // longitude out of bounding box range
    result->clear(docID);
    cost[0]--;
    return;
  }

  int docLatitude = NumericUtils::sortableBytesToInt(packedValue, 0);
  int docLongitude =
      NumericUtils::sortableBytesToInt(packedValue, Integer::BYTES);
  if (!distancePredicate::test(docLatitude, docLongitude)) {
    result->clear(docID);
    cost[0]--;
  }
}

PointValues::Relation
LatLonPointDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  if (StringHelper::compare(Integer::BYTES, minPackedValue, 0,
                            outerInstance->maxLat, 0) > 0 ||
      StringHelper::compare(Integer::BYTES, maxPackedValue, 0,
                            outerInstance->minLat, 0) < 0) {
    // latitude out of bounding box range
    return PointValues::Relation::CELL_INSIDE_QUERY;
  }

  if ((StringHelper::compare(Integer::BYTES, minPackedValue, Integer::BYTES,
                             outerInstance->maxLon, 0) > 0 ||
       StringHelper::compare(Integer::BYTES, maxPackedValue, Integer::BYTES,
                             outerInstance->minLon, 0) < 0) &&
      StringHelper::compare(Integer::BYTES, maxPackedValue, Integer::BYTES,
                            outerInstance->minLon2, 0) < 0) {
    // latitude out of bounding box range
    return PointValues::Relation::CELL_INSIDE_QUERY;
  }

  double latMin = GeoEncodingUtils::decodeLatitude(minPackedValue, 0);
  double lonMin =
      GeoEncodingUtils::decodeLongitude(minPackedValue, Integer::BYTES);
  double latMax = GeoEncodingUtils::decodeLatitude(maxPackedValue, 0);
  double lonMax =
      GeoEncodingUtils::decodeLongitude(maxPackedValue, Integer::BYTES);

  PointValues::Relation relation = GeoUtils::relate(
      latMin, latMax, lonMin, lonMax, outerInstance->outerInstance.latitude,
      outerInstance->outerInstance.longitude, outerInstance->sortKey,
      outerInstance->axisLat);
  switch (relation) {
  case PointValues::Relation::CELL_INSIDE_QUERY:
    // all points match, skip this subtree
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  case PointValues::Relation::CELL_OUTSIDE_QUERY:
    // none of the points match, clear all documents
    return PointValues::Relation::CELL_INSIDE_QUERY;
  default:
    return relation;
  }
}

wstring LatLonPointDistanceQuery::getField() { return field; }

double LatLonPointDistanceQuery::getLatitude() { return latitude; }

double LatLonPointDistanceQuery::getLongitude() { return longitude; }

double LatLonPointDistanceQuery::getRadiusMeters() { return radiusMeters; }

int LatLonPointDistanceQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + field.hashCode();
  int64_t temp;
  temp = Double::doubleToLongBits(latitude);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(longitude);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(radiusMeters);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

bool LatLonPointDistanceQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool LatLonPointDistanceQuery::equalsTo(
    shared_ptr<LatLonPointDistanceQuery> other)
{
  return field == other->field &&
         Double::doubleToLongBits(latitude) ==
             Double::doubleToLongBits(other->latitude) &&
         Double::doubleToLongBits(longitude) ==
             Double::doubleToLongBits(other->longitude) &&
         Double::doubleToLongBits(radiusMeters) ==
             Double::doubleToLongBits(other->radiusMeters);
}

wstring LatLonPointDistanceQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (this->field != field) {
    sb->append(this->field);
    sb->append(L':');
  }
  sb->append(latitude);
  sb->append(L",");
  sb->append(longitude);
  sb->append(L" +/- ");
  sb->append(radiusMeters);
  sb->append(L" meters");
  return sb->toString();
}
} // namespace org::apache::lucene::document