using namespace std;

#include "RandomGeo3dShapeGenerator.h"

namespace org::apache::lucene::spatial3d::geom
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble;

shared_ptr<PlanetModel> RandomGeo3dShapeGenerator::randomPlanetModel()
{
  constexpr int shapeType = random()->nextInt(2);
  switch (shapeType) {
  case 0: {
    return PlanetModel::SPHERE;
  }
  case 1: {
    return PlanetModel::WGS84;
  }
  default:
    throw make_shared<IllegalStateException>(L"Unexpected planet model");
  }
}

int RandomGeo3dShapeGenerator::randomShapeType()
{
  return random()->nextInt(12);
}

int RandomGeo3dShapeGenerator::randomGeoAreaShapeType()
{
  return random()->nextInt(12);
}

int RandomGeo3dShapeGenerator::randomConvexShapeType()
{
  int shapeType = randomShapeType();
  while (isConcave(shapeType)) {
    shapeType = randomShapeType();
  }
  return shapeType;
}

int RandomGeo3dShapeGenerator::randomConcaveShapeType()
{
  int shapeType = randomShapeType();
  while (!isConcave(shapeType)) {
    shapeType = randomShapeType();
  }
  return shapeType;
}

bool RandomGeo3dShapeGenerator::isConcave(int shapeType)
{
  return (shapeType == CONCAVE_POLYGON);
}

shared_ptr<Constraints> RandomGeo3dShapeGenerator::getEmptyConstraint()
{
  return make_shared<Constraints>(shared_from_this());
}

shared_ptr<GeoPoint>
RandomGeo3dShapeGenerator::randomGeoPoint(shared_ptr<PlanetModel> planetModel)
{
  shared_ptr<GeoPoint> point = nullptr;
  while (point == nullptr) {
    point = randomGeoPoint(planetModel, getEmptyConstraint());
  }
  return point;
}

shared_ptr<GeoPoint>
RandomGeo3dShapeGenerator::randomGeoPoint(shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_POINT_ITERATIONS) {
    double lat = randomDouble() * M_PI / 2;
    if (random()->nextBoolean()) {
      lat = (-1) * lat;
    }
    double lon = randomDouble() * M_PI;
    if (random()->nextBoolean()) {
      lon = (-1) * lon;
    }
    iterations++;
    shared_ptr<GeoPoint> point = make_shared<GeoPoint>(planetModel, lat, lon);
    if (constraints->isWithin(point)) {
      return point;
    }
  }
  return nullptr;
}

shared_ptr<GeoAreaShape> RandomGeo3dShapeGenerator::randomGeoAreaShape(
    int shapeType, shared_ptr<PlanetModel> planetModel)
{
  shared_ptr<GeoAreaShape> geoAreaShape = nullptr;
  while (geoAreaShape == nullptr) {
    geoAreaShape = randomGeoAreaShape(
        shapeType, planetModel, make_shared<Constraints>(shared_from_this()));
  }
  return geoAreaShape;
}

shared_ptr<GeoAreaShape> RandomGeo3dShapeGenerator::randomGeoAreaShape(
    int shapeType, shared_ptr<PlanetModel> planetModel,
    shared_ptr<Constraints> constraints)
{
  return std::static_pointer_cast<GeoAreaShape>(
      randomGeoShape(shapeType, planetModel, constraints));
}

shared_ptr<GeoShape>
RandomGeo3dShapeGenerator::randomGeoShape(int shapeType,
                                          shared_ptr<PlanetModel> planetModel)
{
  shared_ptr<GeoShape> geoShape = nullptr;
  while (geoShape == nullptr) {
    geoShape = randomGeoShape(shapeType, planetModel,
                              make_shared<Constraints>(shared_from_this()));
  }
  return geoShape;
}

shared_ptr<GeoShape>
RandomGeo3dShapeGenerator::randomGeoShape(int shapeType,
                                          shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<Constraints> constraints)
{
  switch (shapeType) {
  case CONVEX_POLYGON: {
    return convexPolygon(planetModel, constraints);
  }
  case CONVEX_POLYGON_WITH_HOLES: {
    return convexPolygonWithHoles(planetModel, constraints);
  }
  case CONCAVE_POLYGON: {
    return concavePolygon(planetModel, constraints);
  }
  case CONCAVE_POLYGON_WITH_HOLES: {
    return concavePolygonWithHoles(planetModel, constraints);
  }
  case COMPLEX_POLYGON: {
    return complexPolygon(planetModel, constraints);
  }
  case CIRCLE: {
    return circle(planetModel, constraints);
  }
  case RECTANGLE: {
    return rectangle(planetModel, constraints);
  }
  case PATH: {
    return path(planetModel, constraints);
  }
  case COLLECTION: {
    return collection(planetModel, constraints);
  }
  case POINT: {
    return point(planetModel, constraints);
  }
  case LINE: {
    return line(planetModel, constraints);
  }
  case CONVEX_SIMPLE_POLYGON: {
    return simpleConvexPolygon(planetModel, constraints);
  }
  case CONCAVE_SIMPLE_POLYGON: {
    return concaveSimplePolygon(planetModel, constraints);
  }
  case EXACT_CIRCLE: {
    return exactCircle(planetModel, constraints);
  }
  default:
    throw make_shared<IllegalStateException>(L"Unexpected shape type");
  }
}

shared_ptr<GeoPointShape>
RandomGeo3dShapeGenerator::point(shared_ptr<PlanetModel> planetModel,
                                 shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    shared_ptr<GeoPoint> *const point =
        randomGeoPoint(planetModel, constraints);
    if (point == nullptr) {
      continue;
    }
    try {

      shared_ptr<GeoPointShape> pointShape =
          GeoPointShapeFactory::makeGeoPointShape(
              planetModel, point->getLatitude(), point->getLongitude());
      if (!constraints->valid(pointShape)) {
        continue;
      }
      return pointShape;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoCircle>
RandomGeo3dShapeGenerator::circle(shared_ptr<PlanetModel> planetModel,
                                  shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    shared_ptr<GeoPoint> *const center =
        randomGeoPoint(planetModel, constraints);
    if (center == nullptr) {
      continue;
    }
    constexpr double radius = randomCutoffAngle();
    try {

      shared_ptr<GeoCircle> circle = GeoCircleFactory::makeGeoCircle(
          planetModel, center->getLatitude(), center->getLongitude(), radius);
      if (!constraints->valid(circle)) {
        continue;
      }
      return circle;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoCircle>
RandomGeo3dShapeGenerator::exactCircle(shared_ptr<PlanetModel> planetModel,
                                       shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    shared_ptr<GeoPoint> *const center =
        randomGeoPoint(planetModel, constraints);
    if (center == nullptr) {
      continue;
    }
    constexpr double radius = randomCutoffAngle();
    constexpr int pow = random()->nextInt(10) + 3;
    constexpr double accuracy = random()->nextDouble() * pow(10, (-1) * pow);
    try {
      shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
          planetModel, center->getLatitude(), center->getLongitude(), radius,
          accuracy);
      if (!constraints->valid(circle)) {
        continue;
      }
      return circle;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoBBox>
RandomGeo3dShapeGenerator::rectangle(shared_ptr<PlanetModel> planetModel,
                                     shared_ptr<Constraints> constraints)
{

  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    shared_ptr<GeoPoint> *const point1 =
        randomGeoPoint(planetModel, constraints);
    if (point1 == nullptr) {
      continue;
    }
    shared_ptr<GeoPoint> *const point2 =
        randomGeoPoint(planetModel, constraints);
    if (point2 == nullptr) {
      continue;
    }

    double minLat = min(point1->getLatitude(), point2->getLatitude());
    double maxLat = max(point1->getLatitude(), point2->getLatitude());
    double minLon = min(point1->getLongitude(), point2->getLongitude());
    double maxLon = max(point1->getLongitude(), point2->getLongitude());

    try {
      shared_ptr<GeoBBox> bbox = GeoBBoxFactory::makeGeoBBox(
          planetModel, maxLat, minLat, minLon, maxLon);
      if (!constraints->valid(bbox)) {
        continue;
      }
      return bbox;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPath>
RandomGeo3dShapeGenerator::line(shared_ptr<PlanetModel> planetModel,
                                shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    int vertexCount = random()->nextInt(2) + 2;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, constraints);
    if (geoPoints.size() < 2) {
      continue;
    }
    try {
      shared_ptr<GeoPath> path = GeoPathFactory::makeGeoPath(
          planetModel, 0,
          geoPoints.toArray(
              std::deque<std::shared_ptr<GeoPoint>>(geoPoints.size())));
      if (!constraints->valid(path)) {
        continue;
      }
      return path;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPath>
RandomGeo3dShapeGenerator::path(shared_ptr<PlanetModel> planetModel,
                                shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    int vertexCount = random()->nextInt(2) + 2;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, constraints);
    if (geoPoints.size() < 2) {
      continue;
    }
    double width = randomCutoffAngle();
    try {
      shared_ptr<GeoPath> path = GeoPathFactory::makeGeoPath(
          planetModel, width,
          geoPoints.toArray(
              std::deque<std::shared_ptr<GeoPoint>>(geoPoints.size())));
      if (!constraints->valid(path)) {
        continue;
      }
      return path;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoCompositeAreaShape>
RandomGeo3dShapeGenerator::collection(shared_ptr<PlanetModel> planetModel,
                                      shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    int numberShapes = random()->nextInt(3) + 2;
    shared_ptr<GeoCompositeAreaShape> collection =
        make_shared<GeoCompositeAreaShape>(planetModel);
    for (int i = 0; i < numberShapes; i++) {
      shared_ptr<GeoPolygon> member = convexPolygon(planetModel, constraints);
      if (member != nullptr) {
        collection->addShape(member);
      }
    }
    if (collection->shapes->size() == 0) {
      continue;
    }
    return collection;
  }
  return nullptr;
}

shared_ptr<GeoPolygon>
RandomGeo3dShapeGenerator::convexPolygon(shared_ptr<PlanetModel> planetModel,
                                         shared_ptr<Constraints> constraints)
{
  int vertexCount = random()->nextInt(4) + 3;
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, constraints);
    if (geoPoints.size() < 3) {
      continue;
    }
    deque<std::shared_ptr<GeoPoint>> orderedGeoPoints = orderPoints(geoPoints);
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeGeoPolygon(planetModel, orderedGeoPoints);
      if (!constraints->valid(polygon) || isConcave(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPolygon> RandomGeo3dShapeGenerator::convexPolygonWithHoles(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Constraints> constraints)
{
  int vertexCount = random()->nextInt(4) + 3;
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, constraints);
    if (geoPoints.size() < 3) {
      continue;
    }
    deque<std::shared_ptr<GeoPoint>> orderedGeoPoints = orderPoints(geoPoints);
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeGeoPolygon(planetModel, orderedGeoPoints);
      // polygon should comply with all constraints except disjoint as we have
      // holes
      shared_ptr<Constraints> polygonConstraints =
          make_shared<Constraints>(shared_from_this());
      polygonConstraints->putAll(constraints->getContains());
      polygonConstraints->putAll(constraints->getWithin());
      polygonConstraints->putAll(constraints->getDisjoint());
      if (!polygonConstraints->valid(polygon) ||
          isConcave(planetModel, polygon)) {
        continue;
      }
      // hole must overlap with polygon and comply with any CONTAINS constraint.
      shared_ptr<Constraints> holeConstraints =
          make_shared<Constraints>(shared_from_this());
      holeConstraints->putAll(constraints->getContains());
      holeConstraints->emplace(polygon, GeoArea::OVERLAPS);
      // Points must be with in the polygon and must comply
      // CONTAINS and DISJOINT constraints
      shared_ptr<Constraints> pointsConstraints =
          make_shared<Constraints>(shared_from_this());
      pointsConstraints->emplace(polygon, GeoArea::WITHIN);
      pointsConstraints->putAll(constraints->getContains());
      pointsConstraints->putAll(constraints->getDisjoint());
      deque<std::shared_ptr<GeoPolygon>> holes = concavePolygonHoles(
          planetModel, polygon, holeConstraints, pointsConstraints);
      // we should have at least one hole
      if (holes.empty()) {
        continue;
      }
      polygon = GeoPolygonFactory::makeGeoPolygon(planetModel, orderedGeoPoints,
                                                  holes);
      if (!constraints->valid(polygon) || isConcave(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

deque<std::shared_ptr<GeoPolygon>>
RandomGeo3dShapeGenerator::concavePolygonHoles(
    shared_ptr<PlanetModel> planetModel, shared_ptr<GeoPolygon> polygon,
    shared_ptr<Constraints> holeConstraints,
    shared_ptr<Constraints> pointConstraints)
{
  int iterations = 0;
  int holesCount = random()->nextInt(3) + 1;
  deque<std::shared_ptr<GeoPolygon>> holes =
      deque<std::shared_ptr<GeoPolygon>>();
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    int vertexCount = random()->nextInt(3) + 3;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, pointConstraints);
    if (geoPoints.size() < 3) {
      continue;
    }
    geoPoints = orderPoints(geoPoints);
    shared_ptr<GeoPolygon> inversePolygon =
        GeoPolygonFactory::makeGeoPolygon(planetModel, geoPoints);
    // The convex polygon must be within
    if (inversePolygon == nullptr ||
        polygon->getRelationship(inversePolygon) != GeoArea::WITHIN) {
      continue;
    }
    // make it concave
    reverse(geoPoints.begin(), geoPoints.end());
    try {
      shared_ptr<GeoPolygon> hole =
          GeoPolygonFactory::makeGeoPolygon(planetModel, geoPoints);
      if (!holeConstraints->valid(hole) || isConvex(planetModel, hole)) {
        continue;
      }
      holes.push_back(hole);
      if (holes.size() == holesCount) {
        return holes;
      }
      pointConstraints->emplace(hole, GeoArea::DISJOINT);
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return holes;
}

shared_ptr<GeoPolygon>
RandomGeo3dShapeGenerator::concavePolygon(shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<Constraints> constraints)
{

  int vertexCount = random()->nextInt(4) + 3;
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, constraints);
    if (geoPoints.size() < 3) {
      continue;
    }
    deque<std::shared_ptr<GeoPoint>> orderedGeoPoints = orderPoints(geoPoints);
    reverse(orderedGeoPoints.begin(), orderedGeoPoints.end());
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeGeoPolygon(planetModel, orderedGeoPoints);
      if (!constraints->valid(polygon) || isConvex(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPolygon> RandomGeo3dShapeGenerator::concavePolygonWithHoles(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Constraints> constraints)
{
  int vertexCount = random()->nextInt(4) + 3;
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    // we first build the hole. We consider all constraints except
    // disjoint as we have a hole
    shared_ptr<Constraints> holeConstraints =
        make_shared<Constraints>(shared_from_this());
    holeConstraints->putAll(constraints->getContains());
    holeConstraints->putAll(constraints->getWithin());
    holeConstraints->putAll(constraints->getOverlaps());
    shared_ptr<GeoPolygon> hole = convexPolygon(planetModel, holeConstraints);
    if (hole == nullptr) {
      continue;
    }
    // Now we get points for polygon. Must we within the hole
    // and we add contain constraints
    shared_ptr<Constraints> pointConstraints =
        make_shared<Constraints>(shared_from_this());
    pointConstraints->emplace(hole, GeoArea::WITHIN);
    pointConstraints->putAll(constraints->getContains());
    deque<std::shared_ptr<GeoPoint>> geoPoints =
        points(vertexCount, planetModel, pointConstraints);
    if (geoPoints.size() < 3) {
      continue;
    }
    try {
      deque<std::shared_ptr<GeoPoint>> orderedGeoPoints =
          orderPoints(geoPoints);
      shared_ptr<GeoPolygon> inversePolygon =
          GeoPolygonFactory::makeGeoPolygon(planetModel, geoPoints);
      // The convex polygon must be within the hole
      if (inversePolygon == nullptr ||
          hole->getRelationship(inversePolygon) != GeoArea::WITHIN) {
        continue;
      }
      reverse(orderedGeoPoints.begin(), orderedGeoPoints.end());
      shared_ptr<GeoPolygon> polygon = GeoPolygonFactory::makeGeoPolygon(
          planetModel, orderedGeoPoints, Collections::singletonList(hole));
      // final polygon must be convex
      if (!constraints->valid(polygon) || isConcave(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPolygon>
RandomGeo3dShapeGenerator::complexPolygon(shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<Constraints> constraints)
{
  int polygonsCount = random()->nextInt(2) + 1;
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>
        polDescription =
            deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>();
    while (polDescription.size() < polygonsCount) {
      int vertexCount = random()->nextInt(14) + 3;
      deque<std::shared_ptr<GeoPoint>> geoPoints =
          points(vertexCount, planetModel, constraints);
      if (geoPoints.size() < 3) {
        break;
      }
      orderPoints(geoPoints);
      polDescription.push_back(
          make_shared<GeoPolygonFactory::PolygonDescription>(geoPoints));
    }
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeLargeGeoPolygon(planetModel, polDescription);
      if (!constraints->valid(polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPolygon> RandomGeo3dShapeGenerator::simpleConvexPolygon(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPoint>> points =
        this->points(3, planetModel, constraints);
    if (points.size() < 3) {
      continue;
    }
    points = orderPoints(points);
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeGeoConvexPolygon(planetModel, points);
      if (!constraints->valid(polygon) || isConcave(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

shared_ptr<GeoPolygon> RandomGeo3dShapeGenerator::concaveSimplePolygon(
    shared_ptr<PlanetModel> planetModel, shared_ptr<Constraints> constraints)
{
  int iterations = 0;
  while (iterations < MAX_SHAPE_ITERATIONS) {
    iterations++;
    deque<std::shared_ptr<GeoPoint>> points =
        this->points(3, planetModel, constraints);
    if (points.size() < 3) {
      continue;
    }
    points = orderPoints(points);
    reverse(points.begin(), points.end());
    try {
      shared_ptr<GeoPolygon> polygon =
          GeoPolygonFactory::makeGeoConcavePolygon(planetModel, points);
      if (!constraints->valid(polygon) || isConvex(planetModel, polygon)) {
        continue;
      }
      return polygon;
    } catch (const invalid_argument &e) {
      continue;
    }
  }
  return nullptr;
}

deque<std::shared_ptr<GeoPoint>>
RandomGeo3dShapeGenerator::points(int count,
                                  shared_ptr<PlanetModel> planetModel,
                                  shared_ptr<Constraints> constraints)
{
  deque<std::shared_ptr<GeoPoint>> geoPoints =
      deque<std::shared_ptr<GeoPoint>>(count);
  for (int i = 0; i < count; i++) {
    shared_ptr<GeoPoint> point = randomGeoPoint(planetModel, constraints);
    if (point != nullptr) {
      geoPoints.push_back(point);
    }
  }
  return geoPoints;
}

bool RandomGeo3dShapeGenerator::isConcave(shared_ptr<PlanetModel> planetModel,
                                          shared_ptr<GeoPolygon> shape)
{
  return (shape->isWithin(planetModel->NORTH_POLE) &&
          shape->isWithin(planetModel->SOUTH_POLE)) ||
         (shape->isWithin(planetModel->MAX_X_POLE) &&
          shape->isWithin(planetModel->MIN_X_POLE)) ||
         (shape->isWithin(planetModel->MAX_Y_POLE) &&
          shape->isWithin(planetModel->MIN_Y_POLE));
}

bool RandomGeo3dShapeGenerator::isConvex(shared_ptr<PlanetModel> planetModel,
                                         shared_ptr<GeoPolygon> shape)
{
  return !isConcave(planetModel, shape);
}

double RandomGeo3dShapeGenerator::randomCutoffAngle()
{
  return randomDouble() * M_PI;
}

deque<std::shared_ptr<GeoPoint>> RandomGeo3dShapeGenerator::orderPoints(
    deque<std::shared_ptr<GeoPoint>> &points)
{
  double x = 0;
  double y = 0;
  double z = 0;
  // get center of mass
  for (auto point : points) {
    x += point->x;
    y += point->y;
    z += point->z;
  }
  unordered_map<double, std::shared_ptr<GeoPoint>> pointWithAngle =
      unordered_map<double, std::shared_ptr<GeoPoint>>();
  // get angle respect center of mass
  for (auto point : points) {
    shared_ptr<GeoPoint> center = make_shared<GeoPoint>(
        x / points.size(), y / points.size(), z / points.size());
    double cs = sin(center->getLatitude()) * sin(point->getLatitude()) +
                cos(center->getLatitude()) * cos(point->getLatitude()) *
                    cos(point->getLongitude() - center->getLongitude());
    double posAng =
        atan2(cos(center->getLatitude()) * cos(point->getLatitude()) *
                  sin(point->getLongitude() - center->getLongitude()),
              sin(point->getLatitude()) - sin(center->getLatitude()) * cs);
    pointWithAngle.emplace(posAng, point);
  }
  // order points
  deque<double> angles = deque<double>(pointWithAngle.keySet());
  sort(angles.begin(), angles.end());
  reverse(angles.begin(), angles.end());
  deque<std::shared_ptr<GeoPoint>> orderedPoints =
      deque<std::shared_ptr<GeoPoint>>();
  for (shared_ptr<> : : optional<double> d : angles) {
    orderedPoints.push_back(pointWithAngle[d]);
  }
  return orderedPoints;
}

RandomGeo3dShapeGenerator::Constraints::Constraints(
    shared_ptr<RandomGeo3dShapeGenerator> outerInstance)
    : outerInstance(outerInstance)
{
}

bool RandomGeo3dShapeGenerator::Constraints::valid(shared_ptr<GeoShape> shape)
{
  if (shape == nullptr) {
    return false;
  }
  for (shared_ptr<GeoAreaShape> constraint : keySet()) {
    if (constraint->getRelationship(shape) != shared_from_this()[constraint]) {
      return false;
    }
  }
  return true;
}

bool RandomGeo3dShapeGenerator::Constraints::isWithin(
    shared_ptr<GeoPoint> point)
{
  for (shared_ptr<GeoShape> constraint : keySet()) {
    if (!(validPoint(point, constraint, shared_from_this()[constraint]))) {
      return false;
    }
  }
  return true;
}

bool RandomGeo3dShapeGenerator::Constraints::validPoint(
    shared_ptr<GeoPoint> point, shared_ptr<GeoShape> shape, int relationship)
{
  // For GeoCompositeMembershipShape we only consider the first shape to help
  // converging
  if (relationship == GeoArea::WITHIN &&
      std::dynamic_pointer_cast<GeoCompositeMembershipShape>(shape) !=
          nullptr) {
    shape = ((std::static_pointer_cast<GeoCompositeMembershipShape>(shape))
                 ->shapes->get(0));
  }
  switch (relationship) {
  case GeoArea::DISJOINT:
    return !shape->isWithin(point);
  case GeoArea::OVERLAPS:
    return true;
  case GeoArea::CONTAINS:
    return !shape->isWithin(point);
  case GeoArea::WITHIN:
    return shape->isWithin(point);
  default:
    return true;
  }
}

shared_ptr<Constraints> RandomGeo3dShapeGenerator::Constraints::getContains()
{
  return getConstraintsOfType(GeoArea::CONTAINS);
}

shared_ptr<Constraints> RandomGeo3dShapeGenerator::Constraints::getWithin()
{
  return getConstraintsOfType(GeoArea::WITHIN);
}

shared_ptr<Constraints> RandomGeo3dShapeGenerator::Constraints::getOverlaps()
{
  return getConstraintsOfType(GeoArea::OVERLAPS);
}

shared_ptr<Constraints> RandomGeo3dShapeGenerator::Constraints::getDisjoint()
{
  return getConstraintsOfType(GeoArea::DISJOINT);
}

shared_ptr<Constraints>
RandomGeo3dShapeGenerator::Constraints::getConstraintsOfType(int type)
{
  shared_ptr<Constraints> constraints = make_shared<Constraints>(outerInstance);
  for (shared_ptr<GeoAreaShape> constraint : keySet()) {
    if (type == shared_from_this()[constraint]) {
      constraints->emplace(constraint, type);
    }
  }
  return constraints;
}
} // namespace org::apache::lucene::spatial3d::geom