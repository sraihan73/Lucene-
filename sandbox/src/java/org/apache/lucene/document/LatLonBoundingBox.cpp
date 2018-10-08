using namespace std;

#include "LatLonBoundingBox.h"

namespace org::apache::lucene::document
{
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

LatLonBoundingBox::LatLonBoundingBox(const wstring &name, double const minLat,
                                     double const minLon, double const maxLat,
                                     double const maxLon)
    : Field(name, getType(2))
{
  setRangeValues(minLat, minLon, maxLat, maxLon);
}

shared_ptr<FieldType> LatLonBoundingBox::getType(int geoDimensions)
{
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setDimensions(geoDimensions * 2, BYTES);
  ft->freeze();
  return ft;
}

void LatLonBoundingBox::setRangeValues(double minLat, double minLon,
                                       double maxLat, double maxLon)
{
  checkArgs(minLat, minLon, maxLat, maxLon);
  const std::deque<char> bytes;
  if (fieldsData == nullptr) {
    bytes = std::deque<char>(4 * BYTES);
    fieldsData = make_shared<BytesRef>(bytes);
  } else {
    bytes = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  }
  encode(minLat, minLon, bytes, 0);
  encode(maxLat, maxLon, bytes, 2 * BYTES);
}

void LatLonBoundingBox::checkArgs(double const minLat, double const minLon,
                                  double const maxLat, double const maxLon)
{
  // dateline crossing not supported
  if (minLon > maxLon) {
    throw invalid_argument(L"cannot have minLon [" + to_wstring(minLon) +
                           L"] exceed maxLon [" + to_wstring(maxLon) + L"].");
  }
  // pole crossing not supported
  if (minLat > maxLat) {
    throw invalid_argument(L"cannot have minLat [" + to_wstring(minLat) +
                           L"] exceed maxLat [" + to_wstring(maxLat) + L"].");
  }
}

shared_ptr<Query> LatLonBoundingBox::newIntersectsQuery(const wstring &field,
                                                        double const minLat,
                                                        double const minLon,
                                                        double const maxLat,
                                                        double const maxLon)
{
  return newRangeQuery(field, minLat, minLon, maxLat, maxLon,
                       RangeFieldQuery::QueryType::INTERSECTS);
}

shared_ptr<Query> LatLonBoundingBox::newWithinQuery(const wstring &field,
                                                    double const minLat,
                                                    double const minLon,
                                                    double const maxLat,
                                                    double const maxLon)
{
  return newRangeQuery(field, minLat, minLon, maxLat, maxLon,
                       RangeFieldQuery::QueryType::WITHIN);
}

shared_ptr<Query> LatLonBoundingBox::newContainsQuery(const wstring &field,
                                                      double const minLat,
                                                      double const minLon,
                                                      double const maxLat,
                                                      double const maxLon)
{
  return newRangeQuery(field, minLat, minLon, maxLat, maxLon,
                       RangeFieldQuery::QueryType::CONTAINS);
}

shared_ptr<Query> LatLonBoundingBox::newCrossesQuery(const wstring &field,
                                                     double const minLat,
                                                     double const minLon,
                                                     double const maxLat,
                                                     double const maxLon)
{
  return newRangeQuery(field, minLat, minLon, maxLat, maxLon,
                       RangeFieldQuery::QueryType::CROSSES);
}

shared_ptr<Query>
LatLonBoundingBox::newRangeQuery(const wstring &field, double const minLat,
                                 double const minLon, double const maxLat,
                                 double const maxLon,
                                 RangeFieldQuery::QueryType const queryType)
{
  checkArgs(minLat, minLon, maxLat, maxLon);
  return make_shared<RangeFieldQueryAnonymousInnerClass>(
      field, encode(minLat, minLon, maxLat, maxLon), queryType);
}

LatLonBoundingBox::RangeFieldQueryAnonymousInnerClass::
    RangeFieldQueryAnonymousInnerClass(
        const wstring &field, deque<char> &encode,
        org::apache::lucene::document::RangeFieldQuery::QueryType queryType)
    : RangeFieldQuery(field, encode, 2, queryType)
{
}

wstring LatLonBoundingBox::RangeFieldQueryAnonymousInnerClass::toString(
    std::deque<char> &ranges, int dimension)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return LatLonBoundingBox::toString(ranges, dimension);
}

std::deque<char> LatLonBoundingBox::encode(double minLat, double minLon,
                                            double maxLat, double maxLon)
{
  std::deque<char> b(BYTES * 4);
  encode(minLat, minLon, b, 0);
  encode(maxLat, maxLon, b, BYTES * 2);
  return b;
}

void LatLonBoundingBox::encode(double lat, double lon,
                               std::deque<char> &result, int offset)
{
  if (result.empty()) {
    result = std::deque<char>(BYTES * 4);
  }
  NumericUtils::intToSortableBytes(encodeLatitude(lat), result, offset);
  NumericUtils::intToSortableBytes(encodeLongitude(lon), result,
                                   offset + BYTES);
}

wstring LatLonBoundingBox::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(getClass().getSimpleName());
  sb->append(L" <");
  sb->append(name_);
  sb->append(L':');
  sb->append(L'[');
  std::deque<char> b = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  sb->append(toString(b, 0));
  sb->append(L',');
  sb->append(toString(b, 1));
  sb->append(L']');
  sb->append(L'>');
  return sb->toString();
}

wstring LatLonBoundingBox::toString(std::deque<char> &ranges, int dimension)
{
  double lat, lon;
  switch (dimension) {
  case 0:
    lat = decodeLatitude(ranges, 0);
    lon = decodeLongitude(ranges, 4);
    break;
  case 1:
    lat = decodeLatitude(ranges, 8);
    lon = decodeLongitude(ranges, 12);
    break;
  default:
    throw invalid_argument(L"invalid dimension [" + to_wstring(dimension) +
                           L"] in toString");
  }
  return to_wstring(lat) + L"," + to_wstring(lon);
}
} // namespace org::apache::lucene::document