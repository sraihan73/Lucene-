using namespace std;

#include "Geo3dShape.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoArea = org::apache::lucene::spatial3d::geom::GeoArea;
using GeoAreaFactory = org::apache::lucene::spatial3d::geom::GeoAreaFactory;
using GeoAreaShape = org::apache::lucene::spatial3d::geom::GeoAreaShape;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using LatLonBounds = org::apache::lucene::spatial3d::geom::LatLonBounds;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
} // namespace org::apache::lucene::spatial::spatial4j