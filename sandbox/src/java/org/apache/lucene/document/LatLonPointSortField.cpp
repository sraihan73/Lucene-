using namespace std;

#include "LatLonPointSortField.h"

namespace org::apache::lucene::document
{
using GeoUtils = org::apache::lucene::geo::GeoUtils;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using SortField = org::apache::lucene::search::SortField;

LatLonPointSortField::LatLonPointSortField(const wstring &field,
                                           double latitude, double longitude)
    : org::apache::lucene::search::SortField(field, SortField::Type::CUSTOM),
      latitude(latitude), longitude(longitude)
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
  setMissingValue(numeric_limits<double>::infinity());
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public org.apache.lucene.search.FieldComparator<?>
// getComparator(int numHits, int sortPos)
shared_ptr < FieldComparator <
    ? >> LatLonPointSortField::getComparator(int numHits, int sortPos)
{
  return make_shared<LatLonPointDistanceComparator>(getField(), latitude,
                                                    longitude, numHits);
}

optional<double> LatLonPointSortField::getMissingValue()
{
  return static_cast<optional<double>>(SortField::getMissingValue());
}

void LatLonPointSortField::setMissingValue(any missingValue)
{
  if (static_cast<Double>(numeric_limits<double>::infinity())
          .equals(missingValue) == false) {
    throw invalid_argument(
        L"Missing value can only be Double.POSITIVE_INFINITY (missing values "
        L"last), but got " +
        missingValue);
  }
  this->missingValue = missingValue;
}

int LatLonPointSortField::hashCode()
{
  constexpr int prime = 31;
  int result = SortField::hashCode();
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
  return result;
}

bool LatLonPointSortField::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!SortField::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<LatLonPointSortField> other =
      any_cast<std::shared_ptr<LatLonPointSortField>>(obj);
  if (Double::doubleToLongBits(latitude) !=
      Double::doubleToLongBits(other->latitude)) {
    return false;
  }
  if (Double::doubleToLongBits(longitude) !=
      Double::doubleToLongBits(other->longitude)) {
    return false;
  }
  return true;
}

wstring LatLonPointSortField::toString()
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(L"<distance:");
  builder->append(L'"');
  builder->append(getField());
  builder->append(L'"');
  builder->append(L" latitude=");
  builder->append(latitude);
  builder->append(L" longitude=");
  builder->append(longitude);
  if (numeric_limits<double>::infinity() != getMissingValue()) {
    builder->append(L" missingValue=" + getMissingValue());
  }
  builder->append(L'>');
  return builder->toString();
}
} // namespace org::apache::lucene::document