using namespace std;

#include "Polygon.h"

namespace org::apache::lucene::geo
{

Polygon::Polygon(std::deque<double> &polyLats, std::deque<double> &polyLons,
                 deque<Polygon> &holes)
    : polyLats(polyLats.clone()), polyLons(polyLons.clone()),
      holes(holes.clone())
{
  if (polyLats.empty()) {
    throw invalid_argument(L"polyLats must not be null");
  }
  if (polyLons.empty()) {
    throw invalid_argument(L"polyLons must not be null");
  }
  if (holes.empty()) {
    throw invalid_argument(L"holes must not be null");
  }
  if (polyLats.size() != polyLons.size()) {
    throw invalid_argument(L"polyLats and polyLons must be equal length");
  }
  if (polyLats.size() != polyLons.size()) {
    throw invalid_argument(L"polyLats and polyLons must be equal length");
  }
  if (polyLats.size() < 4) {
    throw invalid_argument(L"at least 4 polygon points required");
  }
  if (polyLats[0] != polyLats[polyLats.size() - 1]) {
    throw invalid_argument(L"first and last points of the polygon must be the "
                           L"same (it must close itself): polyLats[0]=" +
                           to_wstring(polyLats[0]) + L" polyLats[" +
                           (polyLats.size() - 1) + L"]=" +
                           to_wstring(polyLats[polyLats.size() - 1]));
  }
  if (polyLons[0] != polyLons[polyLons.size() - 1]) {
    throw invalid_argument(L"first and last points of the polygon must be the "
                           L"same (it must close itself): polyLons[0]=" +
                           to_wstring(polyLons[0]) + L" polyLons[" +
                           (polyLons.size() - 1) + L"]=" +
                           to_wstring(polyLons[polyLons.size() - 1]));
  }
  for (int i = 0; i < polyLats.size(); i++) {
    GeoUtils::checkLatitude(polyLats[i]);
    GeoUtils::checkLongitude(polyLons[i]);
  }
  for (int i = 0; i < holes.size(); i++) {
    shared_ptr<Polygon> inner = holes[i];
    if (inner->holes.size() > 0) {
      throw invalid_argument(
          L"holes may not contain holes: polygons may not nest.");
    }
  }

  // compute bounding box
  double minLat = numeric_limits<double>::infinity();
  double maxLat = -numeric_limits<double>::infinity();
  double minLon = numeric_limits<double>::infinity();
  double maxLon = -numeric_limits<double>::infinity();

  for (int i = 0; i < polyLats.size(); i++) {
    minLat = min(polyLats[i], minLat);
    maxLat = max(polyLats[i], maxLat);
    minLon = min(polyLons[i], minLon);
    maxLon = max(polyLons[i], maxLon);
  }
  this->minLat = minLat;
  this->maxLat = maxLat;
  this->minLon = minLon;
  this->maxLon = maxLon;
}

std::deque<double> Polygon::getPolyLats() { return polyLats.clone(); }

std::deque<double> Polygon::getPolyLons() { return polyLons.clone(); }

std::deque<std::shared_ptr<Polygon>> Polygon::getHoles()
{
  return holes.clone();
}

int Polygon::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + Arrays::hashCode(holes);
  result = prime * result + Arrays::hashCode(polyLats);
  result = prime * result + Arrays::hashCode(polyLons);
  return result;
}

bool Polygon::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<Polygon> other = any_cast<std::shared_ptr<Polygon>>(obj);
  if (!Arrays::equals(holes, other->holes)) {
    return false;
  }
  if (!Arrays::equals(polyLats, other->polyLats)) {
    return false;
  }
  if (!Arrays::equals(polyLons, other->polyLons)) {
    return false;
  }
  return true;
}

wstring Polygon::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < polyLats.size(); i++) {
    sb->append(L"[")
        ->append(polyLats[i])
        ->append(L", ")
        ->append(polyLons[i])
        ->append(L"] ");
  }
  if (holes.size() > 0) {
    sb->append(L", holes=");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(Arrays->toString(holes));
  }
  return sb->toString();
}

std::deque<std::shared_ptr<Polygon>>
Polygon::fromGeoJSON(const wstring &geojson) 
{
  return (make_shared<SimpleGeoJSONPolygonParser>(geojson))->parse();
}
} // namespace org::apache::lucene::geo