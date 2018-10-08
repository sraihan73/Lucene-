using namespace std;

#include "LatLonDocValuesDistanceQuery.h"

namespace org::apache::lucene::document
{
using GeoEncodingUtils = org::apache::lucene::geo::GeoEncodingUtils;
using GeoUtils = org::apache::lucene::geo::GeoUtils;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

LatLonDocValuesDistanceQuery::LatLonDocValuesDistanceQuery(const wstring &field,
                                                           double latitude,
                                                           double longitude,
                                                           double radiusMeters)
    : field(field), latitude(latitude), longitude(longitude),
      radiusMeters(radiusMeters)
{
  if (Double::isFinite(radiusMeters) == false || radiusMeters < 0) {
    throw invalid_argument(L"radiusMeters: '" + to_wstring(radiusMeters) +
                           L"' is invalid");
  }
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
}

wstring LatLonDocValuesDistanceQuery::toString(const wstring &field)
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

bool LatLonDocValuesDistanceQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<LatLonDocValuesDistanceQuery> other =
      any_cast<std::shared_ptr<LatLonDocValuesDistanceQuery>>(obj);
  return field == other->field &&
         Double::doubleToLongBits(latitude) ==
             Double::doubleToLongBits(other->latitude) &&
         Double::doubleToLongBits(longitude) ==
             Double::doubleToLongBits(other->longitude) &&
         Double::doubleToLongBits(radiusMeters) ==
             Double::doubleToLongBits(other->radiusMeters);
}

int LatLonDocValuesDistanceQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + field.hashCode();
  h = 31 * h + Double::hashCode(latitude);
  h = 31 * h + Double::hashCode(longitude);
  h = 31 * h + Double::hashCode(radiusMeters);
  return h;
}

shared_ptr<Weight>
LatLonDocValuesDistanceQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                           bool needsScores,
                                           float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<LatLonDocValuesDistanceQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->boost = boost;
  distancePredicate = GeoEncodingUtils::createDistancePredicate(
      outerInstance->latitude, outerInstance->longitude,
      outerInstance->radiusMeters);
}

shared_ptr<Scorer>
LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedNumericDocValues> *const values =
      context->reader()->getSortedNumericDocValues(outerInstance->field);
  if (values == nullptr) {
    return nullptr;
  }

  shared_ptr<TwoPhaseIterator> *const iterator =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       values);
  return make_shared<ConstantScoreScorer>(shared_from_this(), boost, iterator);
}

LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  for (int i = 0, count = values->docValueCount(); i < count; ++i) {
    constexpr int64_t value = values->nextValue();
    constexpr int lat = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 32));
    constexpr int lon = static_cast<int>(value & 0xFFFFFFFF);
    if (distancePredicate::test(lat, lon)) {
      return true;
    }
  }
  return false;
}

float LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100.0f; // TODO: what should it be?
}

bool LatLonDocValuesDistanceQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}
} // namespace org::apache::lucene::document