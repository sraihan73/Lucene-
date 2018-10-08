using namespace std;

#include "GeoTestUtil.h"

namespace org::apache::lucene::geo
{
using NumericUtils = org::apache::lucene::util::NumericUtils;
using SloppyMath = org::apache::lucene::util::SloppyMath;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::RandomizedContext;

double GeoTestUtil::nextLatitude() { return nextDoubleInternal(-90, 90); }

double GeoTestUtil::nextLongitude() { return nextDoubleInternal(-180, 180); }

double GeoTestUtil::nextDoubleInternal(double low, double high)
{
  assert((low >= std, : numeric_limits<int>::min()));
  assert((high <= std, : numeric_limits<int>::max()));
  assert(Double::isFinite(low));
  assert(Double::isFinite(high));
  assert(
      (high >= low, L"low=" + to_wstring(low) + L" high=" + to_wstring(high)));

  // if they are equal, not much we can do
  if (low == high) {
    return low;
  }

  // first pick a base value.
  constexpr double baseValue;
  int surpriseMe = random()->nextInt(17);
  if (surpriseMe == 0) {
    // random bits
    int64_t lowBits = NumericUtils::doubleToSortableLong(low);
    int64_t highBits = NumericUtils::doubleToSortableLong(high);
    baseValue = NumericUtils::sortableLongToDouble(
        TestUtil::nextLong(random(), lowBits, highBits));
  } else if (surpriseMe == 1) {
    // edge case
    baseValue = low;
  } else if (surpriseMe == 2) {
    // edge case
    baseValue = high;
  } else if (surpriseMe == 3 && low <= 0 && high >= 0) {
    // may trigger divide by 0
    baseValue = 0.0;
  } else if (surpriseMe == 4) {
    // divide up space into block of 360
    double delta = (high - low) / 360;
    int block = random()->nextInt(360);
    baseValue = low + delta * block;
  } else {
    // distributed ~ evenly
    baseValue = low + (high - low) * random()->nextDouble();
  }

  assert(baseValue >= low);
  assert(baseValue <= high);

  // either return the base value or adjust it by 1 ulp in a random direction
  // (if possible)
  int adjustMe = random()->nextInt(17);
  if (adjustMe == 0) {
    return Math::nextAfter(adjustMe, high);
  } else if (adjustMe == 1) {
    return Math::nextAfter(adjustMe, low);
  } else {
    return baseValue;
  }
}

double GeoTestUtil::nextLatitudeNear(double otherLatitude, double delta)
{
  delta = abs(delta);
  GeoUtils::checkLatitude(otherLatitude);
  int surpriseMe = random()->nextInt(97);
  if (surpriseMe == 0) {
    // purely random
    return nextLatitude();
  } else if (surpriseMe < 49) {
    // upper half of region (the exact point or 1 ulp difference is still
    // likely)
    return nextDoubleInternal(otherLatitude, min(90, otherLatitude + delta));
  } else {
    // lower half of region (the exact point or 1 ulp difference is still
    // likely)
    return nextDoubleInternal(max(-90, otherLatitude - delta), otherLatitude);
  }
}

double GeoTestUtil::nextLongitudeNear(double otherLongitude, double delta)
{
  delta = abs(delta);
  GeoUtils::checkLongitude(otherLongitude);
  int surpriseMe = random()->nextInt(97);
  if (surpriseMe == 0) {
    // purely random
    return nextLongitude();
  } else if (surpriseMe < 49) {
    // upper half of region (the exact point or 1 ulp difference is still
    // likely)
    return nextDoubleInternal(otherLongitude, min(180, otherLongitude + delta));
  } else {
    // lower half of region (the exact point or 1 ulp difference is still
    // likely)
    return nextDoubleInternal(max(-180, otherLongitude - delta),
                              otherLongitude);
  }
}

double GeoTestUtil::nextLatitudeBetween(double minLatitude, double maxLatitude)
{
  assert(maxLatitude >= minLatitude);
  GeoUtils::checkLatitude(minLatitude);
  GeoUtils::checkLatitude(maxLatitude);
  if (random()->nextInt(47) == 0) {
    // purely random
    return nextLatitude();
  } else {
    // extend the range by 1%
    double difference = (maxLatitude - minLatitude) / 100;
    double lower = max(-90, minLatitude - difference);
    double upper = min(90, maxLatitude + difference);
    return nextDoubleInternal(lower, upper);
  }
}

double GeoTestUtil::nextLongitudeBetween(double minLongitude,
                                         double maxLongitude)
{
  assert(maxLongitude >= minLongitude);
  GeoUtils::checkLongitude(minLongitude);
  GeoUtils::checkLongitude(maxLongitude);
  if (random()->nextInt(47) == 0) {
    // purely random
    return nextLongitude();
  } else {
    // extend the range by 1%
    double difference = (maxLongitude - minLongitude) / 100;
    double lower = max(-180, minLongitude - difference);
    double upper = min(180, maxLongitude + difference);
    return nextDoubleInternal(lower, upper);
  }
}

std::deque<double> GeoTestUtil::nextPointAroundLine(double lat1, double lon1,
                                                     double lat2, double lon2)
{
  double x1 = lon1;
  double x2 = lon2;
  double y1 = lat1;
  double y2 = lat2;
  double minX = min(x1, x2);
  double maxX = max(x1, x2);
  double minY = min(y1, y2);
  double maxY = max(y1, y2);
  if (minX == maxX) {
    return std::deque<double>{nextLatitudeBetween(minY, maxY),
                               nextLongitudeNear(minX, 0.01 * (maxY - minY))};
  } else if (minY == maxY) {
    return std::deque<double>{nextLatitudeNear(minY, 0.01 * (maxX - minX)),
                               nextLongitudeBetween(minX, maxX)};
  } else {
    double x = nextLongitudeBetween(minX, maxX);
    double y = (y1 - y2) / (x1 - x2) * (x - x1) + y1;
    if (Double::isFinite(y) == false) {
      // this can happen due to underflow when delta between x values is
      // wonderfully tiny!
      y = Math::copySign(90, x1);
    }
    double delta = (maxY - minY) * 0.01;
    // our formula may put the targeted Y out of bounds
    y = min(90, y);
    y = max(-90, y);
    return std::deque<double>{nextLatitudeNear(y, delta), x};
  }
}

std::deque<double> GeoTestUtil::nextPointNear(shared_ptr<Rectangle> rectangle)
{
  if (rectangle->crossesDateline()) {
    // pick a "side" of the two boxes we really are
    if (random()->nextBoolean()) {
      return nextPointNear(make_shared<Rectangle>(
          rectangle->minLat, rectangle->maxLat, -180, rectangle->maxLon));
    } else {
      return nextPointNear(make_shared<Rectangle>(
          rectangle->minLat, rectangle->maxLat, rectangle->minLon, 180));
    }
  } else {
    return nextPointNear(boxPolygon(rectangle));
  }
}

std::deque<double> GeoTestUtil::nextPointNear(shared_ptr<Polygon> polygon)
{
  std::deque<double> polyLats = polygon->getPolyLats();
  std::deque<double> polyLons = polygon->getPolyLons();
  std::deque<std::shared_ptr<Polygon>> holes = polygon->getHoles();

  // if there are any holes, target them aggressively
  if (holes.size() > 0 && random()->nextInt(3) == 0) {
    return nextPointNear(holes[random()->nextInt(holes.size())]);
  }

  int surpriseMe = random()->nextInt(97);
  if (surpriseMe == 0) {
    // purely random
    return std::deque<double>{nextLatitude(), nextLongitude()};
  } else if (surpriseMe < 5) {
    // purely random within bounding box
    return std::deque<double>{
        nextLatitudeBetween(polygon->minLat, polygon->maxLat),
        nextLongitudeBetween(polygon->minLon, polygon->maxLon)};
  } else if (surpriseMe < 20) {
    // target a vertex
    int vertex = random()->nextInt(polyLats.size() - 1);
    return std::deque<double>{
        nextLatitudeNear(polyLats[vertex],
                         polyLats[vertex + 1] - polyLats[vertex]),
        nextLongitudeNear(polyLons[vertex],
                          polyLons[vertex + 1] - polyLons[vertex])};
  } else if (surpriseMe < 30) {
    // target points around the bounding box edges
    shared_ptr<Polygon> container = boxPolygon(make_shared<Rectangle>(
        polygon->minLat, polygon->maxLat, polygon->minLon, polygon->maxLon));
    std::deque<double> containerLats = container->getPolyLats();
    std::deque<double> containerLons = container->getPolyLons();
    int startVertex = random()->nextInt(containerLats.size() - 1);
    return nextPointAroundLine(
        containerLats[startVertex], containerLons[startVertex],
        containerLats[startVertex + 1], containerLons[startVertex + 1]);
  } else {
    // target points around diagonals between vertices
    int startVertex = random()->nextInt(polyLats.size() - 1);
    // but favor edges heavily
    int endVertex = random()->nextBoolean()
                        ? startVertex + 1
                        : random()->nextInt(polyLats.size() - 1);
    return nextPointAroundLine(polyLats[startVertex], polyLons[startVertex],
                               polyLats[endVertex], polyLons[endVertex]);
  }
}

shared_ptr<Rectangle> GeoTestUtil::nextBoxNear(shared_ptr<Polygon> polygon)
{
  const std::deque<double> point1;
  const std::deque<double> point2;

  // if there are any holes, target them aggressively
  std::deque<std::shared_ptr<Polygon>> holes = polygon->getHoles();
  if (holes.size() > 0 && random()->nextInt(3) == 0) {
    return nextBoxNear(holes[random()->nextInt(holes.size())]);
  }

  int surpriseMe = random()->nextInt(97);
  if (surpriseMe == 0) {
    // formed from two interesting points
    point1 = nextPointNear(polygon);
    point2 = nextPointNear(polygon);
  } else {
    // formed from one interesting point: then random within delta.
    point1 = nextPointNear(polygon);
    point2 = std::deque<double>(2);
    // now figure out a good delta: we use a rough heuristic, up to the length
    // of an edge
    std::deque<double> polyLats = polygon->getPolyLats();
    std::deque<double> polyLons = polygon->getPolyLons();
    int vertex = random()->nextInt(polyLats.size() - 1);
    double deltaX = polyLons[vertex + 1] - polyLons[vertex];
    double deltaY = polyLats[vertex + 1] - polyLats[vertex];
    double edgeLength = sqrt(deltaX * deltaX + deltaY * deltaY);
    point2[0] = nextLatitudeNear(point1[0], edgeLength);
    point2[1] = nextLongitudeNear(point1[1], edgeLength);
  }

  // form a box from the two points
  double minLat = min(point1[0], point2[0]);
  double maxLat = max(point1[0], point2[0]);
  double minLon = min(point1[1], point2[1]);
  double maxLon = max(point1[1], point2[1]);
  return make_shared<Rectangle>(minLat, maxLat, minLon, maxLon);
}

shared_ptr<Rectangle> GeoTestUtil::nextBox()
{
  return nextBoxInternal(nextLatitude(), nextLatitude(), nextLongitude(),
                         nextLongitude(), true);
}

shared_ptr<Rectangle> GeoTestUtil::nextBoxNotCrossingDateline()
{
  return nextBoxInternal(nextLatitude(), nextLatitude(), nextLongitude(),
                         nextLongitude(), false);
}

shared_ptr<Polygon> GeoTestUtil::createRegularPolygon(double centerLat,
                                                      double centerLon,
                                                      double radiusMeters,
                                                      int gons)
{

  // System.out.println("MAKE POLY: centerLat=" + centerLat + " centerLon=" +
  // centerLon + " radiusMeters=" + radiusMeters + " gons=" + gons);

  std::deque<std::deque<double>> result(2);
  result[0] = std::deque<double>(gons + 1);
  result[1] = std::deque<double>(gons + 1);
  // System.out.println("make gon=" + gons);
  for (int i = 0; i < gons; i++) {
    double angle = 360.0 - i * (360.0 / gons);
    // System.out.println("  angle " + angle);
    double x = cos(SloppyMath::toRadians(angle));
    double y = sin(SloppyMath::toRadians(angle));
    double factor = 2.0;
    double step = 1.0;
    int last = 0;

    // System.out.println("angle " + angle + " slope=" + slope);
    // Iterate out along one spoke until we hone in on the point that's nearly
    // exactly radiusMeters from the center:
    while (true) {

      // TODO: we could in fact cross a pole?  Just do what surpriseMePolygon
      // does?
      double lat = centerLat + y * factor;
      GeoUtils::checkLatitude(lat);
      double lon = centerLon + x * factor;
      GeoUtils::checkLongitude(lon);
      double distanceMeters =
          SloppyMath::haversinMeters(centerLat, centerLon, lat, lon);

      // System.out.println("  iter lat=" + lat + " lon=" + lon + " distance=" +
      // distanceMeters + " vs " + radiusMeters);
      if (abs(distanceMeters - radiusMeters) < 0.1) {
        // Within 10 cm: close enough!
        result[0][i] = lat;
        result[1][i] = lon;
        break;
      }

      if (distanceMeters > radiusMeters) {
        // too big
        // System.out.println("    smaller");
        factor -= step;
        if (last == 1) {
          // System.out.println("      half-step");
          step /= 2.0;
        }
        last = -1;
      } else if (distanceMeters < radiusMeters) {
        // too small
        // System.out.println("    bigger");
        factor += step;
        if (last == -1) {
          // System.out.println("      half-step");
          step /= 2.0;
        }
        last = 1;
      }
    }
  }

  // close poly
  result[0][gons] = result[0][0];
  result[1][gons] = result[1][0];

  // System.out.println("  polyLats=" + Arrays.toString(result[0]));
  // System.out.println("  polyLons=" + Arrays.toString(result[1]));

  return make_shared<Polygon>(result[0], result[1]);
}

shared_ptr<Polygon> GeoTestUtil::nextPolygon()
{
  if (random()->nextBoolean()) {
    return surpriseMePolygon();
  } else if (random()->nextInt(10) == 1) {
    // this poly is slow to create ... only do it 10% of the time:
    while (true) {
      int gons = TestUtil::nextInt(random(), 4, 500);
      // So the poly can cover at most 50% of the earth's surface:
      double radiusMeters = random()->nextDouble() *
                                GeoUtils::EARTH_MEAN_RADIUS_METERS * M_PI /
                                2.0 +
                            1.0;
      try {
        return createRegularPolygon(nextLatitude(), nextLongitude(),
                                    radiusMeters, gons);
      } catch (const invalid_argument &iae) {
        // we tried to cross dateline or pole ... try again
      }
    }
  }

  shared_ptr<Rectangle> box = nextBoxInternal(
      nextLatitude(), nextLatitude(), nextLongitude(), nextLongitude(), false);
  if (random()->nextBoolean()) {
    // box
    return boxPolygon(box);
  } else {
    // triangle
    return trianglePolygon(box);
  }
}

shared_ptr<Rectangle> GeoTestUtil::nextBoxInternal(double lat0, double lat1,
                                                   double lon0, double lon1,
                                                   bool canCrossDateLine)
{
  if (lat1 < lat0) {
    double x = lat0;
    lat0 = lat1;
    lat1 = x;
  }

  if (canCrossDateLine == false && lon1 < lon0) {
    double x = lon0;
    lon0 = lon1;
    lon1 = x;
  }

  return make_shared<Rectangle>(lat0, lat1, lon0, lon1);
}

shared_ptr<Polygon> GeoTestUtil::boxPolygon(shared_ptr<Rectangle> box)
{
  assert(box->crossesDateline() == false);
  const std::deque<double> polyLats = std::deque<double>(5);
  const std::deque<double> polyLons = std::deque<double>(5);
  polyLats[0] = box->minLat;
  polyLons[0] = box->minLon;
  polyLats[1] = box->maxLat;
  polyLons[1] = box->minLon;
  polyLats[2] = box->maxLat;
  polyLons[2] = box->maxLon;
  polyLats[3] = box->minLat;
  polyLons[3] = box->maxLon;
  polyLats[4] = box->minLat;
  polyLons[4] = box->minLon;
  return make_shared<Polygon>(polyLats, polyLons);
}

shared_ptr<Polygon> GeoTestUtil::trianglePolygon(shared_ptr<Rectangle> box)
{
  assert(box->crossesDateline() == false);
  const std::deque<double> polyLats = std::deque<double>(4);
  const std::deque<double> polyLons = std::deque<double>(4);
  polyLats[0] = box->minLat;
  polyLons[0] = box->minLon;
  polyLats[1] = box->maxLat;
  polyLons[1] = box->minLon;
  polyLats[2] = box->maxLat;
  polyLons[2] = box->maxLon;
  polyLats[3] = box->minLat;
  polyLons[3] = box->minLon;
  return make_shared<Polygon>(polyLats, polyLons);
}

shared_ptr<Polygon> GeoTestUtil::surpriseMePolygon()
{
  // repeat until we get a poly that doesn't cross dateline:
  while (true) {
    // System.out.println("\nPOLY ITER");
    double centerLat = nextLatitude();
    double centerLon = nextLongitude();
    double radius = 0.1 + 20 * random()->nextDouble();
    double radiusDelta = random()->nextDouble();

    deque<double> lats = deque<double>();
    deque<double> lons = deque<double>();
    double angle = 0.0;
    while (true) {
      angle += random()->nextDouble() * 40.0;
      // System.out.println("  angle " + angle);
      if (angle > 360) {
        break;
      }
      double len =
          radius * (1.0 - radiusDelta + radiusDelta * random()->nextDouble());
      // System.out.println("    len=" + len);
      double lat = centerLat + len * cos(SloppyMath::toRadians(angle));
      double lon = centerLon + len * sin(SloppyMath::toRadians(angle));
      if (lon <= GeoUtils::MIN_LON_INCL || lon >= GeoUtils::MAX_LON_INCL) {
        // cannot cross dateline: try again!
        goto newPolyContinue;
      }
      if (lat > 90) {
        // cross the north pole
        lat = 180 - lat;
        lon = 180 - lon;
      } else if (lat < -90) {
        // cross the south pole
        lat = -180 - lat;
        lon = 180 - lon;
      }
      if (lon <= GeoUtils::MIN_LON_INCL || lon >= GeoUtils::MAX_LON_INCL) {
        // cannot cross dateline: try again!
        goto newPolyContinue;
      }
      lats.push_back(lat);
      lons.push_back(lon);

      // System.out.println("    lat=" + lats.get(lats.size()-1) + " lon=" +
      // lons.get(lons.size()-1));
    }

    // close it
    lats.push_back(lats[0]);
    lons.push_back(lons[0]);

    std::deque<double> latsArray(lats.size());
    std::deque<double> lonsArray(lons.size());
    for (int i = 0; i < lats.size(); i++) {
      latsArray[i] = lats[i];
      lonsArray[i] = lons[i];
    }
    return make_shared<Polygon>(latsArray, lonsArray);
  newPolyContinue:;
  }
newPolyBreak:;
}

shared_ptr<Random> GeoTestUtil::random()
{
  return RandomizedContext::current().getRandom();
}

wstring GeoTestUtil::toSVG(deque<any> &objects)
{
  deque<any> flattened = deque<any>();
  for (any o : objects) {
    if (dynamic_cast<std::deque<std::shared_ptr<Polygon>>>(o) != nullptr) {
      flattened.addAll(Arrays::asList(
          static_cast<std::deque<std::shared_ptr<Polygon>>>(o)));
    } else {
      flattened.push_back(o);
    }
  }
  // first compute bounding area of all the objects
  double minLat = numeric_limits<double>::infinity();
  double maxLat = -numeric_limits<double>::infinity();
  double minLon = numeric_limits<double>::infinity();
  double maxLon = -numeric_limits<double>::infinity();
  for (auto o : flattened) {
    shared_ptr<Rectangle> *const r;
    if (std::dynamic_pointer_cast<Polygon>(o) != nullptr) {
      r = Rectangle::fromPolygon(std::deque<std::shared_ptr<Polygon>>{
          std::static_pointer_cast<Polygon>(o)});
      minLat = min(minLat, r->minLat);
      maxLat = max(maxLat, r->maxLat);
      minLon = min(minLon, r->minLon);
      maxLon = max(maxLon, r->maxLon);
    }
  }
  if (Double::isFinite(minLat) == false || Double::isFinite(maxLat) == false ||
      Double::isFinite(minLon) == false || Double::isFinite(maxLon) == false) {
    throw invalid_argument(L"you must pass at least one polygon");
  }

  // add some additional padding so we can really see what happens on the edges
  // too
  double xpadding = (maxLon - minLon) / 64;
  double ypadding = (maxLat - minLat) / 64;
  // expand points to be this large
  double pointX = xpadding * 0.1;
  double pointY = ypadding * 0.1;
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"640\" "
             L"width=\"480\" viewBox=\"");
  sb->append(minLon - xpadding)
      ->append(L" ")
      ->append(90 - maxLat - ypadding)
      ->append(L" ")
      ->append(maxLon - minLon + (2 * xpadding))
      ->append(L" ")
      ->append(maxLat - minLat + (2 * ypadding));
  sb->append(L"\">\n");

  // encode each object
  for (auto o : flattened) {
    // tostring
    if (dynamic_cast<std::deque<double>>(o) != nullptr) {
      std::deque<double> point = static_cast<std::deque<double>>(o);
      sb->append(L"<!-- point: ");
      sb->append(to_wstring(point[0]) + L"," + to_wstring(point[1]));
      sb->append(L" -->\n");
    } else {
      sb->append(L"<!-- " + o.getClass().getSimpleName() + L": \n");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      sb->append(o.toString());
      sb->append(L"\n-->\n");
    }
    shared_ptr<Polygon> *const gon;
    const wstring style;
    const wstring opacity;
    if (std::dynamic_pointer_cast<Rectangle>(o) != nullptr) {
      gon = boxPolygon(std::static_pointer_cast<Rectangle>(o));
      style = L"fill:lightskyblue;stroke:black;stroke-width:0.2%;stroke-"
              L"dasharray:0.5%,1%;";
      opacity = L"0.3";
    } else if (dynamic_cast<std::deque<double>>(o) != nullptr) {
      std::deque<double> point = static_cast<std::deque<double>>(o);
      gon = boxPolygon(make_shared<Rectangle>(
          max(-90, point[0] - pointY), min(90, point[0] + pointY),
          max(-180, point[1] - pointX), min(180, point[1] + pointX)));
      style = L"fill:red;stroke:red;stroke-width:0.1%;";
      opacity = L"0.7";
    } else {
      gon = std::static_pointer_cast<Polygon>(o);
      style = L"fill:lawngreen;stroke:black;stroke-width:0.3%;";
      opacity = L"0.5";
    }
    // polygon
    std::deque<double> polyLats = gon->getPolyLats();
    std::deque<double> polyLons = gon->getPolyLons();
    sb->append(L"<polygon fill-opacity=\"" + opacity + L"\" points=\"");
    for (int i = 0; i < polyLats.size(); i++) {
      if (i > 0) {
        sb->append(L" ");
      }
      sb->append(polyLons[i])->append(L",")->append(90 - polyLats[i]);
    }
    sb->append(L"\" style=\"" + style + L"\"/>\n");
    for (auto hole : gon->getHoles()) {
      std::deque<double> holeLats = hole->getPolyLats();
      std::deque<double> holeLons = hole->getPolyLons();
      sb->append(L"<polygon points=\"");
      for (int i = 0; i < holeLats.size(); i++) {
        if (i > 0) {
          sb->append(L" ");
        }
        sb->append(holeLons[i])->append(L",")->append(90 - holeLats[i]);
      }
      sb->append(L"\" style=\"fill:lightgray\"/>\n");
    }
  }
  sb->append(L"</svg>\n");
  return sb->toString();
}

bool GeoTestUtil::containsSlowly(shared_ptr<Polygon> polygon, double latitude,
                                 double longitude)
{
  if (polygon->getHoles().size() > 0) {
    throw make_shared<UnsupportedOperationException>(
        L"this testing method does not support holes");
  }
  std::deque<double> polyLats = polygon->getPolyLats();
  std::deque<double> polyLons = polygon->getPolyLons();
  // bounding box check required due to rounding errors (we don't solve that
  // problem)
  if (latitude < polygon->minLat || latitude > polygon->maxLat ||
      longitude < polygon->minLon || longitude > polygon->maxLon) {
    return false;
  }

  bool c = false;
  int i, j;
  int nvert = polyLats.size();
  std::deque<double> verty = polyLats;
  std::deque<double> vertx = polyLons;
  double testy = latitude;
  double testx = longitude;
  for (i = 0, j = nvert - 1; i < nvert; j = i++) {
    if (((verty[i] > testy) != (verty[j] > testy)) &&
        (testx <
         (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) +
             vertx[i])) {
      c = !c;
    }
  }
  return c;
}
} // namespace org::apache::lucene::geo