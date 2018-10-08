using namespace std;

#include "Geo3DPointOutsideSortField.h"

namespace org::apache::lucene::spatial3d
{
using FieldComparator = org::apache::lucene::search::FieldComparator;
using SortField = org::apache::lucene::search::SortField;
using GeoOutsideDistance =
    org::apache::lucene::spatial3d::geom::GeoOutsideDistance;

Geo3DPointOutsideSortField::Geo3DPointOutsideSortField(
    const wstring &field, shared_ptr<GeoOutsideDistance> distanceShape)
    : org::apache::lucene::search::SortField(field, SortField::Type::CUSTOM),
      distanceShape(distanceShape)
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (distanceShape == nullptr) {
    throw invalid_argument(L"distanceShape must not be null");
  }
  setMissingValue(numeric_limits<double>::infinity());
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public org.apache.lucene.search.FieldComparator<?>
// getComparator(int numHits, int sortPos)
shared_ptr < FieldComparator <
    ? >> Geo3DPointOutsideSortField::getComparator(int numHits, int sortPos)
{
  return make_shared<Geo3DPointOutsideDistanceComparator>(
      getField(), distanceShape, numHits);
}

optional<double> Geo3DPointOutsideSortField::getMissingValue()
{
  return static_cast<optional<double>>(SortField::getMissingValue());
}

void Geo3DPointOutsideSortField::setMissingValue(any missingValue)
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

int Geo3DPointOutsideSortField::hashCode()
{
  constexpr int prime = 31;
  int result = SortField::hashCode();
  int64_t temp;
  temp = distanceShape->hashCode();
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

bool Geo3DPointOutsideSortField::equals(any obj)
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
  shared_ptr<Geo3DPointSortField> *const other =
      any_cast<std::shared_ptr<Geo3DPointSortField>>(obj);
  return distanceShape->equals(other->distanceShape);
}

wstring Geo3DPointOutsideSortField::toString()
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(L"<outsideDistanceShape:");
  builder->append(L'"');
  builder->append(getField());
  builder->append(L'"');
  builder->append(L" shape=");
  builder->append(distanceShape);
  if (numeric_limits<double>::infinity() != getMissingValue()) {
    builder->append(L" missingValue=" + getMissingValue());
  }
  builder->append(L'>');
  return builder->toString();
}
} // namespace org::apache::lucene::spatial3d