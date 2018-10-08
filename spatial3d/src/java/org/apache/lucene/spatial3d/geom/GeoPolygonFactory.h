#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <algorithm>
#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygon.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/BestShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoComplexPolygon.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/TileException.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SafePath.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoCompositePolygon.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/MutableBoolean.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/EdgeBuffer.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Edge.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::spatial3d::geom
{

/**
 * Class which constructs a GeoMembershipShape representing an arbitrary
 * polygon.
 *
 * @lucene.experimental
 */
class GeoPolygonFactory : public std::enable_shared_from_this<GeoPolygonFactory>
{
  GET_CLASS_NAME(GeoPolygonFactory)
private:
  GeoPolygonFactory();

  static constexpr int SMALL_POLYGON_CUTOFF_EDGES = 100;

  /** Create a GeoConcavePolygon using the specified points. The polygon must
   * have a maximum extent larger than PI. The siding of the polygon is chosen
   * so that any adjacent point to a segment provides an exterior measurement
   * and therefore, the polygon is a truly concave polygon. Note that this
   * method should only be used when there is certainty that we are dealing with
   * a concave polygon, e.g. the polygon has been serialized. If there is not
   * such certainty, please refer to @{@link
   * GeoPolygonFactory#makeGeoPolygon(PlanetModel, List)}.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.
   * @return a GeoPolygon corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoPolygon>
  makeGeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                        std::deque<std::shared_ptr<GeoPoint>> &pointList);

  /** Create a GeoConvexPolygon using the specified points. The polygon must
   * have a maximum extent no larger than PI. The siding of the polygon is
   * chosen so that any  adjacent point to a segment provides an interior
   * measurement and therefore the polygon is a truly convex polygon. Note that
   * this method should only be used when there is certainty that we are dealing
   * with a convex polygon, e.g. the polygon has been serialized. If there is
   * not such certainty, please refer to @{@link
   * GeoPolygonFactory#makeGeoPolygon(PlanetModel, List)}.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.
   * @return a GeoPolygon corresponding to what was specified.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoConvexPolygon(std::shared_ptr<PlanetModel> planetModel,
                       std::deque<std::shared_ptr<GeoPoint>> &pointList);

  /** Create a GeoConcavePolygon using the specified points and holes. The
   * polygon must have a maximum extent larger than PI. The siding of the
   * polygon is chosen so that any  adjacent point to a segment provides an
   * exterior measurement and therefore the polygon is a truly concave polygon.
   * Note that this method should only be used when there is certainty that we
   * are dealing with a concave polygon, e.g. the polygon has been serialized.
   * If there is not such certainty, please refer to {@link
   * GeoPolygonFactory#makeGeoPolygon(PlanetModel, List, List)}.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.
   * @param holes is a deque of polygons representing "holes" in the outside
   * polygon.  Holes describe the area outside each hole as being "in set". Null
   * == none.
   * @return a GeoPolygon corresponding to what was specified.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                        std::deque<std::shared_ptr<GeoPoint>> &pointList,
                        std::deque<std::shared_ptr<GeoPolygon>> &holes);

  /** Create a GeoConvexPolygon using the specified points and holes. The
   * polygon must have a maximum extent no larger than PI. The siding of the
   * polygon is chosen so that any adjacent point to a segment provides an
   * interior measurement and therefore the polygon is a truly convex polygon.
   * Note that this method should only be used when there is certainty that we
   * are dealing with a convex polygon, e.g. the polygon has been serialized. If
   * there is not such certainty, please refer to {@link
   * GeoPolygonFactory#makeGeoPolygon(PlanetModel, List, List)}.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.
   * @param holes is a deque of polygons representing "holes" in the outside
   * polygon.  Holes describe the area outside each hole as being "in set". Null
   * == none.
   * @return a GeoPolygon corresponding to what was specified.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoConvexPolygon(std::shared_ptr<PlanetModel> planetModel,
                       std::deque<std::shared_ptr<GeoPoint>> &pointList,
                       std::deque<std::shared_ptr<GeoPolygon>> &holes);

  /** Use this class to specify a polygon with associated holes.
   */
public:
  class PolygonDescription
      : public std::enable_shared_from_this<PolygonDescription>
  {
    GET_CLASS_NAME(PolygonDescription)
    /** The deque of points */
  public:
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: public final java.util.List<? extends GeoPoint> points;
    const std::deque < ? extends GeoPoint > points;
    /** The deque of holes */
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: public final java.util.List<? extends PolygonDescription>
    // holes;
    const std::deque < ? extends PolygonDescription > holes;

    /** Instantiate the polygon description.
     * @param points is the deque of points.
     */
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: public PolygonDescription(final
    // java.util.List<? extends GeoPoint> points)
    PolygonDescription(std::deque<T1> const points);

    /** Instantiate the polygon description.
     * @param points is the deque of points.
     * @param holes is the deque of holes.
     */
    template <typename T1, typename T2>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: public PolygonDescription(final
    // java.util.List<? extends GeoPoint> points, final java.util.List<? extends
    // PolygonDescription> holes)
    PolygonDescription(std::deque<T1> const points,
                       std::deque<T2> const holes);
  };

  /** Create a GeoPolygon using the specified points and holes, using order to
   * determine siding of the polygon.  Much like ESRI, this method uses
   * clockwise to indicate the space on the same side of the shape as being
   * inside, and counter-clockwise to indicate the space on the opposite side as
   * being inside.
   * @param description describes the polygon and its associated holes.  If
   * points go clockwise from a given pole, then that pole should be within the
   * polygon.  If points go counter-clockwise, then that pole should be outside
   * the polygon.
   * @return a GeoPolygon corresponding to what was specified, or null if a
   * valid polygon cannot be generated from this input.
   */
public:
  static std::shared_ptr<GeoPolygon>
  makeGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<PolygonDescription> description);

  /** Create a GeoPolygon using the specified points and holes, using order to
   * determine siding of the polygon.  Much like ESRI, this method uses
   * clockwise to indicate the space on the same side of the shape as being
   * inside, and counter-clockwise to indicate the space on the opposite side as
   * being inside.
   * @param description describes the polygon and its associated holes.  If
   * points go clockwise from a given pole, then that pole should be within the
   * polygon.  If points go counter-clockwise, then that pole should be outside
   * the polygon.
   * @param leniencyValue is the maximum distance (in units) that a point can be
   * from the plane and still be considered as belonging to the plane.  Any
   * value greater than zero may cause some of the provided points that are in
   * fact outside the strict definition of co-planarity, but are within this
   * distance, to be discarded for the purposes of creating a "safe" polygon.
   * @return a GeoPolygon corresponding to what was specified, or null if a
   * valid polygon cannot be generated from this input.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<PolygonDescription> description,
                 double const leniencyValue);

  /** Create a GeoPolygon using the specified points and holes, using order to
   * determine siding of the polygon.  Much like ESRI, this method uses
   * clockwise to indicate the space on the same side of the shape as being
   * inside, and counter-clockwise to indicate the space on the opposite side as
   * being inside.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.  If points go clockwise from a given pole, then that pole should be
   * within the polygon.  If points go counter-clockwise, then that pole should
   * be outside the polygon.
   * @return a GeoPolygon corresponding to what was specified.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::deque<std::shared_ptr<GeoPoint>> &pointList);

  /** Create a GeoPolygon using the specified points and holes, using order to
   * determine siding of the polygon.  Much like ESRI, this method uses
   * clockwise to indicate the space on the same side of the shape as being
   * inside, and counter-clockwise to indicate the space on the opposite side as
   * being inside.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.  If points go clockwise from a given pole, then that pole should be
   * within the polygon.  If points go counter-clockwise, then that pole should
   * be outside the polygon.
   * @param holes is a deque of polygons representing "holes" in the outside
   * polygon.  Holes describe the area outside each hole as being "in set". Null
   * == none.
   * @return a GeoPolygon corresponding to what was specified, or null if a
   * valid polygon cannot be generated from this input.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::deque<std::shared_ptr<GeoPoint>> &pointList,
                 std::deque<std::shared_ptr<GeoPolygon>> &holes);

  /** Create a GeoPolygon using the specified points and holes, using order to
   * determine siding of the polygon.  Much like ESRI, this method uses
   * clockwise to indicate the space on the same side of the shape as being
   * inside, and counter-clockwise to indicate the space on the opposite side as
   * being inside.
   * @param pointList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.  If points go clockwise from a given pole, then that pole should be
   * within the polygon.  If points go counter-clockwise, then that pole should
   * be outside the polygon.
   * @param holes is a deque of polygons representing "holes" in the outside
   * polygon.  Holes describe the area outside each hole as being "in set". Null
   * == none.
   * @param leniencyValue is the maximum distance (in units) that a point can be
   * from the plane and still be considered as belonging to the plane.  Any
   * value greater than zero may cause some of the provided points that are in
   * fact outside the strict definition of co-planarity, but are within this
   * distance, to be discarded for the purposes of creating a "safe" polygon.
   * @return a GeoPolygon corresponding to what was specified, or null if a
   * valid polygon cannot be generated from this input.
   */
  static std::shared_ptr<GeoPolygon>
  makeGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::deque<std::shared_ptr<GeoPoint>> &pointList,
                 std::deque<std::shared_ptr<GeoPolygon>> &holes,
                 double const leniencyValue);

  /** Generate a point at the center of mass of a deque of points.
   */
private:
  static std::shared_ptr<GeoPoint>
  getCenterOfMass(std::shared_ptr<PlanetModel> planetModel,
                  std::deque<std::shared_ptr<GeoPoint>> &points);

  /** Create a large GeoPolygon.  This is one which has more than 100 sides
   * and/or may have resolution problems with very closely spaced points, which
   * often occurs when the polygon was constructed to approximate curves.  No
   * tiling is done, and intersections and membership are optimized for having
   * large numbers of sides.
   *
   * This method does very little checking for legality.  It expects the
   * incoming shapes to not intersect each other.  The shapes can be disjoint or
   * nested.  If the shapes listed are nested, then we are describing holes.
   * There is no limit to the depth of holes.  However, if a shape is nested
   * within another it must be explicitly described as being a child of the
   * other shape.
   *
   * Membership in any given shape is described by the
   * clockwise/counterclockwise direction of the points.  The clockwise
   * direction indicates that a point inside is "in-set", while a
   * counter-clockwise direction implies that a point inside is "out-of-set".
   *
   * @param planetModel is the planet model.
   * @param shapesList is the deque of polygons we should be making.
   * @return the GeoPolygon, or null if it cannot be constructed.
   */
public:
  static std::shared_ptr<GeoPolygon> makeLargeGeoPolygon(
      std::shared_ptr<PlanetModel> planetModel,
      std::deque<std::shared_ptr<PolygonDescription>> &shapesList);

  /** Convert a polygon description to a deque of shapes.  Also locate an optimal
   * shape for evaluating a test point.
   * @param pointsList is the structure to add new polygons to.
   * @param shape is the current polygon description.
   * @param testPointShape is the current best choice for a low-level polygon to
   * evaluate.
   * @return an updated best-choice for a test point polygon, and update the
   * points deque.
   */
private:
  static std::shared_ptr<BestShape> convertPolygon(
      std::deque<std::deque<std::shared_ptr<GeoPoint>>> &pointsList,
      std::shared_ptr<PolygonDescription> shape,
      std::shared_ptr<BestShape> testPointShape, bool const mustBeInside);

  /**
   * Class for tracking the best shape for finding a pole, and whether or not
   * the pole must be inside or outside of the shape.
   */
private:
  class BestShape : public std::enable_shared_from_this<BestShape>
  {
    GET_CLASS_NAME(BestShape)
  public:
    const std::deque<std::shared_ptr<GeoPoint>> points;
    bool poleMustBeInside = false;

    BestShape(std::deque<std::shared_ptr<GeoPoint>> &points,
              bool const poleMustBeInside);

    virtual std::shared_ptr<GeoComplexPolygon> createGeoComplexPolygon(
        std::shared_ptr<PlanetModel> planetModel,
        std::deque<std::deque<std::shared_ptr<GeoPoint>>> &pointsList,
        std::shared_ptr<GeoPoint> testPoint);
  };

  /**
   * Create a GeoPolygon using the specified points and holes and a test point.
   *
   * @param filteredPointList is a filtered deque of the GeoPoints to build an
   * arbitrary polygon out of.
   * @param holes is a deque of polygons representing "holes" in the outside
   * polygon.  Null == none.
   * @param testPoint is a test point that is either known to be within the
   * polygon area, or not.
   * @param testPointInside is true if the test point is within the area, false
   * otherwise.
   * @return a GeoPolygon corresponding to what was specified, or null if what
   * was specified cannot be turned into a valid non-degenerate polygon.
   */
public:
  static std::shared_ptr<GeoPolygon>
  generateGeoPolygon(std::shared_ptr<PlanetModel> planetModel,
                     std::deque<std::shared_ptr<GeoPoint>> &filteredPointList,
                     std::deque<std::shared_ptr<GeoPolygon>> &holes,
                     std::shared_ptr<GeoPoint> testPoint,
                     bool const testPointInside) ;

  /** Filter duplicate points.
   * @param input with input deque of points
   * @return the filtered deque, or null if we can't get a legit polygon from the
   * input.
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: static java.util.List<GeoPoint>
  // filterPoints(final java.util.List<? extends GeoPoint> input)
  static std::deque<std::shared_ptr<GeoPoint>>
  filterPoints(std::deque<T1> const input);

  /** Filter coplanar points.
   * @param noIdenticalPoints with input deque of points
   * @param leniencyValue is the allowed distance of a point from the plane for
   * cleanup of overly detailed polygons
   * @return the filtered deque, or null if we can't get a legit polygon from the
   * input.
   */
  static std::deque<std::shared_ptr<GeoPoint>>
  filterEdges(std::deque<std::shared_ptr<GeoPoint>> &noIdenticalPoints,
              double const leniencyValue);

  /** Iterative path search through ordered deque of points. The method merges
   * together all consecutive coplanar points and builds the plane using the
   * first and the last point. It does not converge if the starting point is
   * coplanar with the last and next point of the path.
   *
   * @param points is the ordered raw deque of points under consideration.
   * @param startIndex is index of the point that starts the current path, so
   * that we can know when we are done.
   * @param leniencyValue is the allowed distance of a point from the plane to
   * be considered coplanar.
   * @return null if the starting point is coplanar with the last and next point
   * of the path.
   */
private:
  static std::shared_ptr<SafePath>
  findSafePath(std::deque<std::shared_ptr<GeoPoint>> &points,
               int const startIndex, double const leniencyValue);

  /** Pick a random pole that has a good chance of being inside the polygon
   * described by the points.
   * @param generator is the random number generator to use.
   * @param planetModel is the planet model to use.
   * @param points is the deque of points available.
   * @return the randomly-determined pole selection.
   */
  static std::shared_ptr<GeoPoint>
  pickPole(std::shared_ptr<Random> generator,
           std::shared_ptr<PlanetModel> planetModel,
           std::deque<std::shared_ptr<GeoPoint>> &points);

  /** For a specified point and a deque of poly points, determine based on point
   * order whether the point should be considered in or out of the polygon.
   * @param point is the point to check.
   * @param polyPoints is the deque of points comprising the polygon.
   * @return null if the point is illegal, otherwise false if the point is
   * inside and true if the point is outside of the polygon.
   */
  static std::optional<bool>
  isInsidePolygon(std::shared_ptr<GeoPoint> point,
                  std::deque<std::shared_ptr<GeoPoint>> &polyPoints);

  /** Compute the angle for a point given rotation information.
   * @param point is the point to assess
   * @param sinLatitude the sine of the latitude
   * @param cosLatitude the cosine of the latitude
   * @param sinLongitude the sine of the longitude
   * @param cosLongitude the cosine of the longitude
   * @return the angle of rotation, or null if not computable
   */
  static std::optional<double> computeAngle(std::shared_ptr<GeoPoint> point,
                                            double const sinLatitude,
                                            double const cosLatitude,
                                            double const sinLongitude,
                                            double const cosLongitude);

  /** Build a GeoPolygon out of one concave part and multiple convex parts given
   * points, starting edge, and whether starting edge is internal or not.
   * @param rval is the composite polygon to add to.
   * @param seenConcave is true if a concave polygon has been seen in this
   * generation yet.
   * @param planetModel is the planet model.
   * @param pointsList is a deque of the GeoPoints to build an arbitrary polygon
   * out of.
   * @param internalEdges specifies which edges are internal.
   * @param startPointIndex is the first of the points, constituting the
   * starting edge.
   * @param startingEdge is the plane describing the starting edge.
   * @param holes is the deque of holes in the polygon, or null if none.
   * @param testPoint is an (optional) test point, which will be used to
   * determine if we are generating a shape with the proper sidedness.  It is
   * passed in only when the test point is supposed to be outside of the
   * generated polygon.  In this case, if the generated polygon is found to
   * contain the point, the method exits early with a null return value. This
   * only makes sense in the context of evaluating both possible choices and
   * using logic to determine which result to use.  If the test point is
   * supposed to be within the shape, then it must be outside of the complement
   * shape.  If the test point is supposed to be outside the shape, then it must
   * be outside of the original shape.  Either way, we can figure out the right
   * thing to use.
   * @return false if what was specified
   *  was inconsistent with what we generated.  Specifically, if we specify an
   * exterior point that is found in the interior of the shape we create here we
   * return false, which is a signal that we chose our initial plane sidedness
   * backwards.
   */
public:
  static bool
  buildPolygonShape(std::shared_ptr<GeoCompositePolygon> rval,
                    std::shared_ptr<MutableBoolean> seenConcave,
                    std::shared_ptr<PlanetModel> planetModel,
                    std::deque<std::shared_ptr<GeoPoint>> &pointsList,
                    std::shared_ptr<BitSet> internalEdges,
                    int const startPointIndex, int const endPointIndex,
                    std::shared_ptr<SidedPlane> startingEdge,
                    std::deque<std::shared_ptr<GeoPolygon>> &holes,
                    std::shared_ptr<GeoPoint> testPoint) ;

  /** Look for a concave polygon in the remainder of the edgebuffer.
   * By this point, if there are any edges in the edgebuffer, they represent a
   * concave polygon.
   * @param planetModel is the planet model.
   * @param rval is the composite polygon we're building.
   * @param seenConcave is true if we've already seen a concave polygon.
   * @param edgeBuffer is the edge buffer.
   * @param holes is the optional deque of holes.
   * @param testPoint is the optional test point.
   * @return true unless the testPoint caused failure.
   */
private:
  static bool
  makeConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                     std::shared_ptr<GeoCompositePolygon> rval,
                     std::shared_ptr<MutableBoolean> seenConcave,
                     std::shared_ptr<EdgeBuffer> edgeBuffer,
                     std::deque<std::shared_ptr<GeoPolygon>> &holes,
                     std::shared_ptr<GeoPoint> testPoint) ;

  /** Look for a convex polygon at the specified edge.  If we find it, create
   * one and adjust the edge buffer.
   * @param planetModel is the planet model.
   * @param currentEdge is the current edge to use starting the search.
   * @param rval is the composite polygon to build.
   * @param edgeBuffer is the edge buffer.
   * @param holes is the optional deque of holes.
   * @param testPoint is the optional test point.
   * @return null if the testPoint is within any polygon detected, otherwise
   * true if a convex polygon was created.
   */
  static std::optional<bool>
  findConvexPolygon(std::shared_ptr<PlanetModel> planetModel,
                    std::shared_ptr<Edge> currentEdge,
                    std::shared_ptr<GeoCompositePolygon> rval,
                    std::shared_ptr<EdgeBuffer> edgeBuffer,
                    std::deque<std::shared_ptr<GeoPolygon>> &holes,
                    std::shared_ptr<GeoPoint> testPoint) ;

  /** Check if a point is within a set of edges.
   * @param point is the point
   * @param edgeSet is the set of edges
   * @param extension is the new edge
   * @param returnBoundary is the return edge
   * @return true if within
   */
  static bool isWithin(std::shared_ptr<GeoPoint> point,
                       std::shared_ptr<Set<std::shared_ptr<Edge>>> edgeSet,
                       std::shared_ptr<Edge> extension,
                       std::shared_ptr<SidedPlane> returnBoundary);

  /** Check if a point is within a set of edges.
   * @param point is the point
   * @param edgeSet is the set of edges
   * @return true if within
   */
  static bool isWithin(std::shared_ptr<GeoPoint> point,
                       std::shared_ptr<Set<std::shared_ptr<Edge>>> edgeSet);

  /** Convert raw point index into valid array position.
   *@param index is the array index.
   *@param size is the array size.
   *@return an updated index.
   */
  static int getLegalIndex(int index, int size);

  /** Class representing a single (unused) edge.
   */
private:
  class Edge : public std::enable_shared_from_this<Edge>
  {
    GET_CLASS_NAME(Edge)
    /** Plane */
  public:
    const std::shared_ptr<SidedPlane> plane;
    /** Start point */
    const std::shared_ptr<GeoPoint> startPoint;
    /** End point */
    const std::shared_ptr<GeoPoint> endPoint;
    /** Internal edge flag */
    const bool isInternal;

    /** Constructor.
     * @param startPoint the edge start point
     * @param endPoint the edge end point
     * @param plane the edge plane
     * @param isInternal true if internal edge
     */
    Edge(std::shared_ptr<GeoPoint> startPoint,
         std::shared_ptr<GeoPoint> endPoint, std::shared_ptr<SidedPlane> plane,
         bool const isInternal);

    virtual int hashCode();

    bool equals(std::any const o) override;
  };

  /** Class representing an iterator over an EdgeBuffer.
   */
private:
  class EdgeBufferIterator
      : public std::enable_shared_from_this<EdgeBufferIterator>,
        public Iterator<std::shared_ptr<Edge>>
  {
    GET_CLASS_NAME(EdgeBufferIterator)
    /** Edge buffer */
  protected:
    const std::shared_ptr<EdgeBuffer> edgeBuffer;
    /** First edge */
    const std::shared_ptr<Edge> firstEdge;
    /** Current edge */
    std::shared_ptr<Edge> currentEdge;

    /** Constructor.
     * @param edgeBuffer the edge buffer
     */
  public:
    EdgeBufferIterator(std::shared_ptr<EdgeBuffer> edgeBuffer);

    bool hasNext() override;

    std::shared_ptr<Edge> next() override;

    void remove() override;
  };

  /** Class representing a pool of unused edges, all linked together by
   * vertices.
   */
private:
  class EdgeBuffer : public std::enable_shared_from_this<EdgeBuffer>
  {
    GET_CLASS_NAME(EdgeBuffer)
    /** Starting edge */
  protected:
    std::shared_ptr<Edge> oneEdge;
    /** Full set of edges */
    const std::shared_ptr<Set<std::shared_ptr<Edge>>> edges =
        std::unordered_set<std::shared_ptr<Edge>>();
    /** Map to previous edge */
    const std::unordered_map<std::shared_ptr<Edge>, std::shared_ptr<Edge>>
        previousEdges =
            std::unordered_map<std::shared_ptr<Edge>, std::shared_ptr<Edge>>();
    /** Map to next edge */
    const std::unordered_map<std::shared_ptr<Edge>, std::shared_ptr<Edge>>
        nextEdges =
            std::unordered_map<std::shared_ptr<Edge>, std::shared_ptr<Edge>>();

    /** Constructor.
     * @param pointList is the deque of points.
     * @param internalEdges is the deque of edges that are internal (includes
     * return edge)
     * @param startPlaneStartIndex is the index of the startPlane's starting
     * point
     * @param startPlaneEndIndex is the index of the startPlane's ending point
     * @param startPlane is the starting plane
     */
  public:
    EdgeBuffer(std::deque<std::shared_ptr<GeoPoint>> &pointList,
               std::shared_ptr<BitSet> internalEdges,
               int const startPlaneStartIndex, int const startPlaneEndIndex,
               std::shared_ptr<SidedPlane> startPlane);

    /*
    protected void verify() {
      if (edges.size() != previousEdges.size() || edges.size() !=
    nextEdges.size()) { throw new IllegalStateException("broken structure");
      }
      // Confirm each edge
      for (final Edge e : edges) {
        final Edge previousEdge = getPrevious(e);
        final Edge nextEdge = getNext(e);
        if (e.endPoint != nextEdge.startPoint) {
          throw new IllegalStateException("broken structure");
        }
        if (e.startPoint != previousEdge.endPoint) {
          throw new IllegalStateException("broken structure");
        }
        if (getNext(previousEdge) != e) {
          throw new IllegalStateException("broken structure");
        }
        if (getPrevious(nextEdge) != e) {
          throw new IllegalStateException("broken structure");
        }
      }
      if (oneEdge != null && !edges.contains(oneEdge)) {
        throw new IllegalStateException("broken structure");
      }
      if (oneEdge == null && edges.size() > 0) {
        throw new IllegalStateException("broken structure");
      }
    }
    */

    /** Get the previous edge.
     * @param currentEdge is the current edge.
     * @return the previous edge, if found.
     */
    virtual std::shared_ptr<Edge>
    getPrevious(std::shared_ptr<Edge> currentEdge);

    /** Get the next edge.
     * @param currentEdge is the current edge.
     * @return the next edge, if found.
     */
    virtual std::shared_ptr<Edge> getNext(std::shared_ptr<Edge> currentEdge);

    /** Replace a deque of edges with a new edge.
     * @param removeList is the deque of edges to remove.
     * @param newEdge is the edge to add.
     */
    virtual void replace(std::deque<std::shared_ptr<Edge>> &removeList,
                         std::shared_ptr<Edge> newEdge);

    /** Clear all edges.
     */
    virtual void clear();

    /** Get the size of the edge buffer.
     * @return the size.
     */
    virtual int size();

    /** Get an iterator to iterate over edges.
     * @return the iterator.
     */
    virtual std::shared_ptr<Iterator<std::shared_ptr<Edge>>> iterator();

    /** Return a first edge.
     * @return the edge.
     */
    virtual std::shared_ptr<Edge> pickOne();
  };

  /** An instance of this class represents a known-good
   * path of nodes that contains no coplanar points , no matter
   * how assessed.  It's used in the depth-first search that
   * must be executed to find a valid complete polygon without
   * coplanarities.
   */
private:
  class SafePath : public std::enable_shared_from_this<SafePath>
  {
    GET_CLASS_NAME(SafePath)
  public:
    const std::shared_ptr<GeoPoint> lastPoint;
    const int lastPointIndex;
    const std::shared_ptr<Plane> lastPlane;
    const std::shared_ptr<SafePath> previous;

    /** Create a new safe end point.
     */
    SafePath(std::shared_ptr<SafePath> previous,
             std::shared_ptr<GeoPoint> lastPoint, int const lastPointIndex,
             std::shared_ptr<Plane> lastPlane);

    /** Fill in a deque, in order, of safe points.
     */
    virtual void fillInList(std::deque<std::shared_ptr<GeoPoint>> &pointList);
  };

public:
  class MutableBoolean : public std::enable_shared_from_this<MutableBoolean>
  {
    GET_CLASS_NAME(MutableBoolean)
  public:
    bool value = false;
  };

  /** Exception we throw when we can't tile a polygon due to numerical precision
   * issues.
   */
private:
  class TileException : public std::runtime_error
  {
    GET_CLASS_NAME(TileException)
  public:
    TileException(const std::wstring &msg);

  protected:
    std::shared_ptr<TileException> shared_from_this()
    {
      return std::static_pointer_cast<TileException>(
          Exception::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
