using namespace std;

#include "Polygon2D.h"

namespace org::apache::lucene::geo
{
using Polygon = org::apache::lucene::geo::Polygon;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

Polygon2D::Polygon2D(shared_ptr<Polygon> polygon, shared_ptr<Polygon2D> holes)
    : minLat(polygon->minLat), maxLat(polygon->maxLat), minLon(polygon->minLon),
      maxLon(polygon->maxLon), holes(holes),
      tree(createTree(polygon->getPolyLats(), polygon->getPolyLons()))
{
  this->maxY = maxLat;
  this->maxX = maxLon;

  // create interval tree of edges
}

bool Polygon2D::contains(double latitude, double longitude)
{
  if (latitude <= maxY && longitude <= maxX) {
    if (componentContains(latitude, longitude)) {
      return true;
    }
    if (left != nullptr) {
      if (left->contains(latitude, longitude)) {
        return true;
      }
    }
    if (right != nullptr && ((splitX == false && latitude >= minLat) ||
                             (splitX && longitude >= minLon))) {
      if (right->contains(latitude, longitude)) {
        return true;
      }
    }
  }
  return false;
}

bool Polygon2D::componentContains(double latitude, double longitude)
{
  // check bounding box
  if (latitude < minLat || latitude > maxLat || longitude < minLon ||
      longitude > maxLon) {
    return false;
  }

  if (tree->contains(latitude, longitude)) {
    if (holes != nullptr && holes->contains(latitude, longitude)) {
      return false;
    }
    return true;
  }

  return false;
}

Relation Polygon2D::relate(double minLat, double maxLat, double minLon,
                           double maxLon)
{
  if (minLat <= maxY && minLon <= maxX) {
    Relation relation = componentRelate(minLat, maxLat, minLon, maxLon);
    if (relation != Relation::CELL_OUTSIDE_QUERY) {
      return relation;
    }
    if (left != nullptr) {
      relation = left->relate(minLat, maxLat, minLon, maxLon);
      if (relation != Relation::CELL_OUTSIDE_QUERY) {
        return relation;
      }
    }
    if (right != nullptr && ((splitX == false && maxLat >= this->minLat) ||
                             (splitX && maxLon >= this->minLon))) {
      relation = right->relate(minLat, maxLat, minLon, maxLon);
      if (relation != Relation::CELL_OUTSIDE_QUERY) {
        return relation;
      }
    }
  }
  return Relation::CELL_OUTSIDE_QUERY;
}

Relation Polygon2D::componentRelate(double minLat, double maxLat, double minLon,
                                    double maxLon)
{
  // if the bounding boxes are disjoint then the shape does not cross
  if (maxLon < this->minLon || minLon > this->maxLon || maxLat < this->minLat ||
      minLat > this->maxLat) {
    return Relation::CELL_OUTSIDE_QUERY;
  }
  // if the rectangle fully encloses us, we cross.
  if (minLat <= this->minLat && maxLat >= this->maxLat &&
      minLon <= this->minLon && maxLon >= this->maxLon) {
    return Relation::CELL_CROSSES_QUERY;
  }
  // check any holes
  if (holes != nullptr) {
    Relation holeRelation = holes->relate(minLat, maxLat, minLon, maxLon);
    if (holeRelation == Relation::CELL_CROSSES_QUERY) {
      return Relation::CELL_CROSSES_QUERY;
    } else if (holeRelation == Relation::CELL_INSIDE_QUERY) {
      return Relation::CELL_OUTSIDE_QUERY;
    }
  }
  // check each corner: if < 4 are present, its cheaper than crossesSlowly
  int numCorners = numberOfCorners(minLat, maxLat, minLon, maxLon);
  if (numCorners == 4) {
    if (tree->crosses(minLat, maxLat, minLon, maxLon)) {
      return Relation::CELL_CROSSES_QUERY;
    }
    return Relation::CELL_INSIDE_QUERY;
  } else if (numCorners > 0) {
    return Relation::CELL_CROSSES_QUERY;
  }

  // we cross
  if (tree->crosses(minLat, maxLat, minLon, maxLon)) {
    return Relation::CELL_CROSSES_QUERY;
  }

  return Relation::CELL_OUTSIDE_QUERY;
}

int Polygon2D::numberOfCorners(double minLat, double maxLat, double minLon,
                               double maxLon)
{
  int containsCount = 0;
  if (componentContains(minLat, minLon)) {
    containsCount++;
  }
  if (componentContains(minLat, maxLon)) {
    containsCount++;
  }
  if (containsCount == 1) {
    return containsCount;
  }
  if (componentContains(maxLat, maxLon)) {
    containsCount++;
  }
  if (containsCount == 2) {
    return containsCount;
  }
  if (componentContains(maxLat, minLon)) {
    containsCount++;
  }
  return containsCount;
}

shared_ptr<Polygon2D>
Polygon2D::createTree(std::deque<std::shared_ptr<Polygon2D>> &components,
                      int low, int high, bool splitX)
{
  if (low > high) {
    return nullptr;
  }
  constexpr int mid =
      static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
  if (low < high) {
    shared_ptr<Comparator<std::shared_ptr<Polygon2D>>> comparator;
    if (splitX) {
      comparator = [&](left, right) {
        int ret = Double::compare(left->minLon, right->minLon);
        if (ret == 0) {
          ret = Double::compare(left->maxX, right->maxX);
        }
        return ret;
      };
    } else {
      comparator = [&](left, right) {
        int ret = Double::compare(left->minLat, right->minLat);
        if (ret == 0) {
          ret = Double::compare(left->maxY, right->maxY);
        }
        return ret;
      };
    }
    ArrayUtil::select(components, low, high + 1, mid, comparator);
  }
  // add midpoint
  shared_ptr<Polygon2D> newNode = components[mid];
  newNode->splitX = splitX;
  // add children
  newNode->left = createTree(components, low, mid - 1, !splitX);
  newNode->right = createTree(components, mid + 1, high, !splitX);
  // pull up max values to this node
  if (newNode->left != nullptr) {
    newNode->maxX = max(newNode->maxX, newNode->left->maxX);
    newNode->maxY = max(newNode->maxY, newNode->left->maxY);
  }
  if (newNode->right != nullptr) {
    newNode->maxX = max(newNode->maxX, newNode->right->maxX);
    newNode->maxY = max(newNode->maxY, newNode->right->maxY);
  }
  return newNode;
}

shared_ptr<Polygon2D> Polygon2D::create(deque<Polygon> &polygons)
{
  std::deque<std::shared_ptr<Polygon2D>> components(polygons->length);
  for (int i = 0; i < components.size(); i++) {
    shared_ptr<Polygon> gon = polygons[i];
    std::deque<std::shared_ptr<Polygon>> gonHoles = gon->getHoles();
    shared_ptr<Polygon2D> holes = nullptr;
    if (gonHoles.size() > 0) {
      holes = create(gonHoles);
    }
    components[i] = make_shared<Polygon2D>(gon, holes);
  }
  return createTree(components, 0, components.size() - 1, false);
}

Polygon2D::Edge::Edge(double lat1, double lon1, double lat2, double lon2,
                      double low, double max)
    : lat1(lat1), lat2(lat2), lon1(lon1), lon2(lon2), low(low)
{
  this->max = max;
}

bool Polygon2D::Edge::contains(double latitude, double longitude)
{
  // crossings algorithm is an odd-even algorithm, so we descend the tree
  // xor'ing results along our path
  bool res = false;
  if (latitude <= max) {
    if (lat1 > latitude != lat2 > latitude) {
      if (longitude <
          (lon1 - lon2) * (latitude - lat2) / (lat1 - lat2) + lon2) {
        res = true;
      }
    }
    if (left != nullptr) {
      res ^= left->contains(latitude, longitude);
    }
    if (right != nullptr && latitude >= low) {
      res ^= right->contains(latitude, longitude);
    }
  }
  return res;
}

bool Polygon2D::Edge::crosses(double minLat, double maxLat, double minLon,
                              double maxLon)
{
  // we just have to cross one edge to answer the question, so we descend the
  // tree and return when we do.
  if (minLat <= max) {
    // we compute line intersections of every polygon edge with every box line.
    // if we find one, return true.
    // for each box line (AB):
    //   for each poly line (CD):
    //     intersects = orient(C,D,A) * orient(C,D,B) <= 0 && orient(A,B,C) *
    //     orient(A,B,D) <= 0
    double cy = lat1;
    double dy = lat2;
    double cx = lon1;
    double dx = lon2;

    // optimization: see if the rectangle is outside of the "bounding box" of
    // the polyline at all if not, don't waste our time trying more complicated
    // stuff
    bool outside = (cy < minLat && dy < minLat) ||
                   (cy > maxLat && dy > maxLat) ||
                   (cx < minLon && dx < minLon) || (cx > maxLon && dx > maxLon);
    if (outside == false) {
      // does box's top edge intersect polyline?
      // ax = minLon, bx = maxLon, ay = maxLat, by = maxLat
      if (orient(cx, cy, dx, dy, minLon, maxLat) *
                  orient(cx, cy, dx, dy, maxLon, maxLat) <=
              0 &&
          orient(minLon, maxLat, maxLon, maxLat, cx, cy) *
                  orient(minLon, maxLat, maxLon, maxLat, dx, dy) <=
              0) {
        return true;
      }

      // does box's right edge intersect polyline?
      // ax = maxLon, bx = maxLon, ay = maxLat, by = minLat
      if (orient(cx, cy, dx, dy, maxLon, maxLat) *
                  orient(cx, cy, dx, dy, maxLon, minLat) <=
              0 &&
          orient(maxLon, maxLat, maxLon, minLat, cx, cy) *
                  orient(maxLon, maxLat, maxLon, minLat, dx, dy) <=
              0) {
        return true;
      }

      // does box's bottom edge intersect polyline?
      // ax = maxLon, bx = minLon, ay = minLat, by = minLat
      if (orient(cx, cy, dx, dy, maxLon, minLat) *
                  orient(cx, cy, dx, dy, minLon, minLat) <=
              0 &&
          orient(maxLon, minLat, minLon, minLat, cx, cy) *
                  orient(maxLon, minLat, minLon, minLat, dx, dy) <=
              0) {
        return true;
      }

      // does box's left edge intersect polyline?
      // ax = minLon, bx = minLon, ay = minLat, by = maxLat
      if (orient(cx, cy, dx, dy, minLon, minLat) *
                  orient(cx, cy, dx, dy, minLon, maxLat) <=
              0 &&
          orient(minLon, minLat, minLon, maxLat, cx, cy) *
                  orient(minLon, minLat, minLon, maxLat, dx, dy) <=
              0) {
        return true;
      }
    }

    if (left != nullptr) {
      if (left->crosses(minLat, maxLat, minLon, maxLon)) {
        return true;
      }
    }

    if (right != nullptr && maxLat >= low) {
      if (right->crosses(minLat, maxLat, minLon, maxLon)) {
        return true;
      }
    }
  }
  return false;
}

shared_ptr<Edge> Polygon2D::createTree(std::deque<double> &polyLats,
                                       std::deque<double> &polyLons)
{
  std::deque<std::shared_ptr<Edge>> edges(polyLats.size() - 1);
  for (int i = 1; i < polyLats.size(); i++) {
    double lat1 = polyLats[i - 1];
    double lon1 = polyLons[i - 1];
    double lat2 = polyLats[i];
    double lon2 = polyLons[i];
    edges[i - 1] = make_shared<Edge>(lat1, lon1, lat2, lon2, min(lat1, lat2),
                                     max(lat1, lat2));
  }
  // sort the edges then build a balanced tree from them
  Arrays::sort(edges, [&](left, right) {
    int ret = Double::compare(left->low, right->low);
    if (ret == 0) {
      ret = Double::compare(left->max, right->max);
    }
    return ret;
  });
  return createTree(edges, 0, edges.size() - 1);
}

shared_ptr<Edge>
Polygon2D::createTree(std::deque<std::shared_ptr<Edge>> &edges, int low,
                      int high)
{
  if (low > high) {
    return nullptr;
  }
  // add midpoint
  int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
  shared_ptr<Edge> newNode = edges[mid];
  // add children
  newNode->left = createTree(edges, low, mid - 1);
  newNode->right = createTree(edges, mid + 1, high);
  // pull up max values to this node
  if (newNode->left != nullptr) {
    newNode->max = max(newNode->max, newNode->left->max);
  }
  if (newNode->right != nullptr) {
    newNode->max = max(newNode->max, newNode->right->max);
  }
  return newNode;
}

int Polygon2D::orient(double ax, double ay, double bx, double by, double cx,
                      double cy)
{
  double v1 = (bx - ax) * (cy - ay);
  double v2 = (cx - ax) * (by - ay);
  if (v1 > v2) {
    return 1;
  } else if (v1 < v2) {
    return -1;
  } else {
    return 0;
  }
}
} // namespace org::apache::lucene::geo