using namespace std;

#include "LatLonDocValuesField.h"

namespace org::apache::lucene::document
{
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexOrDocValuesQuery =
    org::apache::lucene::search::IndexOrDocValuesQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using SortField = org::apache::lucene::search::SortField;
const shared_ptr<FieldType> LatLonDocValuesField::TYPE =
    make_shared<FieldType>();

LatLonDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::SORTED_NUMERIC);
  TYPE->freeze();
}

LatLonDocValuesField::StaticConstructor LatLonDocValuesField::staticConstructor;

LatLonDocValuesField::LatLonDocValuesField(const wstring &name, double latitude,
                                           double longitude)
    : Field(name, TYPE)
{
  setLocationValue(latitude, longitude);
}

void LatLonDocValuesField::setLocationValue(double latitude, double longitude)
{
  int latitudeEncoded = encodeLatitude(latitude);
  int longitudeEncoded = encodeLongitude(longitude);
  fieldsData =
      static_cast<int64_t>(((static_cast<int64_t>(latitudeEncoded)) << 32) |
                             (longitudeEncoded & 0xFFFFFFFFLL));
}

void LatLonDocValuesField::checkCompatible(shared_ptr<FieldInfo> fieldInfo)
{
  // dv properties could be "unset", if you e.g. used only StoredField with this
  // same name in the segment.
  if (fieldInfo->getDocValuesType() != DocValuesType::NONE &&
      fieldInfo->getDocValuesType() != TYPE->docValuesType()) {
    throw invalid_argument(
        L"field=\"" + fieldInfo->name + L"\" was indexed with docValuesType=" +
        fieldInfo->getDocValuesType() + L" but this type has docValuesType=" +
        TYPE->docValuesType() +
        L", is the field really a LatLonDocValuesField?");
  }
}

wstring LatLonDocValuesField::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  int64_t currentValue = any_cast<optional<int64_t>>(fieldsData);
  result->append(decodeLatitude(static_cast<int>(currentValue >> 32)));
  result->append(L',');
  result->append(decodeLongitude(static_cast<int>(currentValue & 0xFFFFFFFF)));

  result->append(L'>');
  return result->toString();
}

shared_ptr<SortField>
LatLonDocValuesField::newDistanceSort(const wstring &field, double latitude,
                                      double longitude)
{
  return make_shared<LatLonPointSortField>(field, latitude, longitude);
}

shared_ptr<Query> LatLonDocValuesField::newSlowBoxQuery(const wstring &field,
                                                        double minLatitude,
                                                        double maxLatitude,
                                                        double minLongitude,
                                                        double maxLongitude)
{
  // exact double values of lat=90.0D and lon=180.0D must be treated special as
  // they are not represented in the encoding and should not drag in extra bogus
  // junk! TODO: should encodeCeil just throw ArithmeticException to be less
  // trappy here?
  if (minLatitude == 90.0) {
    // range cannot match as 90.0 can never exist
    return make_shared<MatchNoDocsQuery>(
        L"LatLonDocValuesField.newBoxQuery with minLatitude=90.0");
  }
  if (minLongitude == 180.0) {
    if (maxLongitude == 180.0) {
      // range cannot match as 180.0 can never exist
      return make_shared<MatchNoDocsQuery>(
          L"LatLonDocValuesField.newBoxQuery with "
          L"minLongitude=maxLongitude=180.0");
    } else if (maxLongitude < minLongitude) {
      // encodeCeil() with dateline wrapping!
      minLongitude = -180.0;
    }
  }
  return make_shared<LatLonDocValuesBoxQuery>(field, minLatitude, maxLatitude,
                                              minLongitude, maxLongitude);
}

shared_ptr<Query>
LatLonDocValuesField::newSlowDistanceQuery(const wstring &field,
                                           double latitude, double longitude,
                                           double radiusMeters)
{
  return make_shared<LatLonDocValuesDistanceQuery>(field, latitude, longitude,
                                                   radiusMeters);
}
} // namespace org::apache::lucene::document