using namespace std;

#include "LatLonDocValuesBoxQuery.h"

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

LatLonDocValuesBoxQuery::LatLonDocValuesBoxQuery(const wstring &field,
                                                 double minLatitude,
                                                 double maxLatitude,
                                                 double minLongitude,
                                                 double maxLongitude)
    : field(field),
      minLatitude(GeoEncodingUtils::encodeLatitudeCeil(minLatitude)),
      maxLatitude(GeoEncodingUtils::encodeLatitude(maxLatitude)),
      minLongitude(GeoEncodingUtils::encodeLongitudeCeil(minLongitude)),
      maxLongitude(GeoEncodingUtils::encodeLongitude(maxLongitude)),
      crossesDateline(minLongitude > maxLongitude) /
          *make sure to compute this before rounding * /
{
  GeoUtils::checkLatitude(minLatitude);
  GeoUtils::checkLatitude(maxLatitude);
  GeoUtils::checkLongitude(minLongitude);
  GeoUtils::checkLongitude(maxLongitude);
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
}

wstring LatLonDocValuesBoxQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (this->field != field) {
    sb->append(this->field);
    sb->append(L':');
  }
  sb->append(L"box(minLat=")
      ->append(GeoEncodingUtils::decodeLatitude(minLatitude));
  sb->append(L", maxLat=")
      ->append(GeoEncodingUtils::decodeLatitude(maxLatitude));
  sb->append(L", minLon=")
      ->append(GeoEncodingUtils::decodeLongitude(minLongitude));
  sb->append(L", maxLon=")
      ->append(GeoEncodingUtils::decodeLongitude(maxLongitude));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sb->append(L")")->toString();
}

bool LatLonDocValuesBoxQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<LatLonDocValuesBoxQuery> other =
      any_cast<std::shared_ptr<LatLonDocValuesBoxQuery>>(obj);
  return field == other->field && crossesDateline == other->crossesDateline &&
         minLatitude == other->minLatitude &&
         maxLatitude == other->maxLatitude &&
         minLongitude == other->minLongitude &&
         maxLongitude == other->maxLongitude;
}

int LatLonDocValuesBoxQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + field.hashCode();
  h = 31 * h + Boolean::hashCode(crossesDateline);
  h = 31 * h + Integer::hashCode(minLatitude);
  h = 31 * h + Integer::hashCode(maxLatitude);
  h = 31 * h + Integer::hashCode(minLongitude);
  h = 31 * h + Integer::hashCode(maxLongitude);
  return h;
}

shared_ptr<Weight>
LatLonDocValuesBoxQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                      bool needsScores,
                                      float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<LatLonDocValuesBoxQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->boost = boost;
}

shared_ptr<Scorer>
LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
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

LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
    : org::apache::lucene::search::TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  for (int i = 0, count = values->docValueCount(); i < count; ++i) {
    constexpr int64_t value = values->nextValue();
    constexpr int lat = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 32));
    if (lat < outerInstance->outerInstance.minLatitude ||
        lat > outerInstance->outerInstance.maxLatitude) {
      // not within latitude range
      continue;
    }

    constexpr int lon = static_cast<int>(value & 0xFFFFFFFF);
    if (outerInstance->outerInstance.crossesDateline) {
      if (lon > outerInstance->outerInstance.maxLongitude &&
          lon < outerInstance->outerInstance.minLongitude) {
        // not within longitude range
        continue;
      }
    } else {
      if (lon < outerInstance->outerInstance.minLongitude ||
          lon > outerInstance->outerInstance.maxLongitude) {
        // not within longitude range
        continue;
      }
    }

    return true;
  }
  return false;
}

float LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 5; // 5 comparisons
}

bool LatLonDocValuesBoxQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}
} // namespace org::apache::lucene::document