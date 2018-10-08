#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class SegmentEndpoint;
}
namespace org::apache::lucene::spatial3d::geom
{
class PathSegment;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class DistanceStyle;
}
namespace org::apache::lucene::spatial3d::geom
{
class Bounds;
}
namespace org::apache::lucene::spatial3d::geom
{
class Membership;
}
namespace org::apache::lucene::spatial3d::geom
{
class Plane;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
}
namespace org::apache::lucene::spatial3d::geom
{
class SidedPlane;
}
namespace org::apache::lucene::spatial3d::geom
{
class Vector;
}

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
 * GeoShape representing a path across the surface of the globe,
 * with a specified half-width.  Path is described by a series of points.
 * Distances are measured from the starting point along the path, and then at
 * right angles to the path.
 *
 * @lucene.internal
 */
class GeoStandardPath : public GeoBasePath
{
  GET_CLASS_NAME(GeoStandardPath)
  /** The cutoff angle (width) */
protected:
  const double cutoffAngle;

  /** Sine of cutoff angle */
  const double sinAngle;
  /** Cosine of cutoff angle */
  const double cosAngle;

  /** The original deque of path points */
  const std::deque<std::shared_ptr<GeoPoint>> points =
      std::deque<std::shared_ptr<GeoPoint>>();

  /** A deque of SegmentEndpoints */
  std::deque<std::shared_ptr<SegmentEndpoint>> endPoints;
  /** A deque of PathSegments */
  std::deque<std::shared_ptr<PathSegment>> segments;

  /** A point on the edge */
  std::deque<std::shared_ptr<GeoPoint>> edgePoints;

  /** Set to true if path has been completely constructed */
  bool isDone = false;

  /** Constructor.
   *@param planetModel is the planet model.
   *@param maxCutoffAngle is the width of the path, measured as an angle.
   *@param pathPoints are the points in the path.
   */
public:
  GeoStandardPath(std::shared_ptr<PlanetModel> planetModel,
                  double const maxCutoffAngle,
                  std::deque<std::shared_ptr<GeoPoint>> &pathPoints);

  /** Piece-wise constructor.  Use in conjunction with addPoint() and done().
   *@param planetModel is the planet model.
   *@param maxCutoffAngle is the width of the path, measured as an angle.
   */
  GeoStandardPath(std::shared_ptr<PlanetModel> planetModel,
                  double const maxCutoffAngle);

  /** Add a point to the path.
   *@param lat is the latitude of the point.
   *@param lon is the longitude of the point.
   */
  virtual void addPoint(double const lat, double const lon);

  /** Complete the path.
   */
  virtual void done();

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoStandardPath(std::shared_ptr<PlanetModel> planetModel,
                  std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  double computePathCenterDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z) override;

  double computeNearestDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y,
                                double const z) override;

protected:
  double distance(std::shared_ptr<DistanceStyle> distanceStyle, double const x,
                  double const y, double const z) override;

  double deltaDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                       double const x, double const y, double const z) override;

  void distanceBounds(std::shared_ptr<Bounds> bounds,
                      std::shared_ptr<DistanceStyle> distanceStyle,
                      double const distanceValue) override;

  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

public:
  bool isWithin(double const x, double const y, double const z) override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> plane,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

  /**
   * This is precalculated data for segment endpoint.
   * Note well: This is not necessarily a circle.  There are four cases:
   * (1) The path consists of a single endpoint.  In this case, we build a
   * simple circle with the proper cutoff offset. (2) This is the end of a path.
   * The circle plane must be constructed to go through two supplied points and
   * be perpendicular to a connecting plane. (2.5) Intersection, but the path on
   * both sides is linear.  We generate a circle, but we use the cutoff planes
   * to limit its influence in the straight line case. (3) This is an
   * intersection in a path.  We are supplied FOUR planes.  If there are
   * intersections within bounds for both upper and lower, then we generate no
   * circle at all.  If there is one intersection only, then we generate a plane
   * that includes that intersection, as well as the remaining cutoff plane/edge
   * plane points.
   */
private:
  class SegmentEndpoint : public std::enable_shared_from_this<SegmentEndpoint>
  {
    GET_CLASS_NAME(SegmentEndpoint)
    /** The center point of the endpoint */
  public:
    const std::shared_ptr<GeoPoint> point;
    /** A plane describing the circle */
    const std::shared_ptr<SidedPlane> circlePlane;
    /** Pertinent cutoff planes from adjoining segments */
    std::deque<std::shared_ptr<Membership>> const cutoffPlanes;
    /** Notable points for this segment endpoint */
    std::deque<std::shared_ptr<GeoPoint>> const notablePoints;
    /** No notable points from the circle itself */
    static std::deque<std::shared_ptr<GeoPoint>> const circlePoints;
    /** Null membership */
    static std::deque<std::shared_ptr<Membership>> const NO_MEMBERSHIP;

    /** Base case.  Does nothing at all.
     */
    SegmentEndpoint(std::shared_ptr<GeoPoint> point);

    /** Constructor for case (1).
     * Generate a simple circle cutoff plane.
     *@param point is the center point.
     *@param upperPoint is a point that must be on the circle plane.
     *@param lowerPoint is another point that must be on the circle plane.
     */
    SegmentEndpoint(std::shared_ptr<GeoPoint> point,
                    std::shared_ptr<Plane> normalPlane,
                    std::shared_ptr<GeoPoint> upperPoint,
                    std::shared_ptr<GeoPoint> lowerPoint);

    /** Constructor for case (2).
     * Generate an endpoint, given a single cutoff plane plus upper and lower
     *edge points.
     *@param point is the center point.
     *@param cutoffPlane is the plane from the adjoining path segment marking
     *the boundary between this endpoint and that segment.
     *@param topEdgePoint is a point on the cutoffPlane that should be also on
     *the circle plane.
     *@param bottomEdgePoint is another point on the cutoffPlane that should be
     *also on the circle plane.
     */
    SegmentEndpoint(std::shared_ptr<GeoPoint> point,
                    std::shared_ptr<SidedPlane> cutoffPlane,
                    std::shared_ptr<GeoPoint> topEdgePoint,
                    std::shared_ptr<GeoPoint> bottomEdgePoint);

    /** Constructor for case (2.5).
     * Generate an endpoint, given two cutoff planes plus upper and lower edge
     *points.
     *@param point is the center.
     *@param cutoffPlane1 is one adjoining path segment cutoff plane.
     *@param cutoffPlane2 is another adjoining path segment cutoff plane.
     *@param topEdgePoint is a point on the cutoffPlane that should be also on
     *the circle plane.
     *@param bottomEdgePoint is another point on the cutoffPlane that should be
     *also on the circle plane.
     */
    SegmentEndpoint(std::shared_ptr<GeoPoint> point,
                    std::shared_ptr<SidedPlane> cutoffPlane1,
                    std::shared_ptr<SidedPlane> cutoffPlane2,
                    std::shared_ptr<GeoPoint> topEdgePoint,
                    std::shared_ptr<GeoPoint> bottomEdgePoint);

    /** Constructor for case (3).
     * Generate an endpoint for an intersection, given four points.
     *@param point is the center.
     *@param prevCutoffPlane is the previous adjoining segment cutoff plane.
     *@param nextCutoffPlane is the next path segment cutoff plane.
     *@param notCand2Point is a point NOT on candidate2.
     *@param notCand1Point is a point NOT on candidate1.
     *@param notCand3Point is a point NOT on candidate3.
     *@param notCand4Point is a point NOT on candidate4.
     *@param candidate1 one of four candidate circle planes.
     *@param candidate2 one of four candidate circle planes.
     *@param candidate3 one of four candidate circle planes.
     *@param candidate4 one of four candidate circle planes.
     */
    SegmentEndpoint(std::shared_ptr<GeoPoint> point,
                    std::shared_ptr<SidedPlane> prevCutoffPlane,
                    std::shared_ptr<SidedPlane> nextCutoffPlane,
                    std::shared_ptr<GeoPoint> notCand2Point,
                    std::shared_ptr<GeoPoint> notCand1Point,
                    std::shared_ptr<GeoPoint> notCand3Point,
                    std::shared_ptr<GeoPoint> notCand4Point,
                    std::shared_ptr<SidedPlane> candidate1,
                    std::shared_ptr<SidedPlane> candidate2,
                    std::shared_ptr<SidedPlane> candidate3,
                    std::shared_ptr<SidedPlane> candidate4);

    /** Check if point is within this endpoint.
     *@param point is the point.
     *@return true of within.
     */
    virtual bool isWithin(std::shared_ptr<Vector> point);

    /** Check if point is within this endpoint.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return true of within.
     */
    virtual bool isWithin(double const x, double const y, double const z);

    /** Compute delta path distance.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, in aggregation form.
     */
    virtual double
    pathDeltaDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                      double const x, double const y, double const z);

    /** Compute interior path distance.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, in aggregation form.
     */
    virtual double pathDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y, double const z);

    /** Compute nearest path distance.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric (always value zero), in aggregation form, or
     *POSITIVE_INFINITY if the point is not within the bounds of the endpoint.
     */
    virtual double
    nearestPathDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                        double const x, double const y, double const z);

    /** Compute path center distance.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, or POSITIVE_INFINITY
     * if the point is not within the bounds of the endpoint.
     */
    virtual double
    pathCenterDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                       double const x, double const y, double const z);

    /** Compute external distance.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric.
     */
    virtual double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z);

    /** Determine if this endpoint intersects a specified plane.
     *@param planetModel is the planet model.
     *@param p is the plane.
     *@param notablePoints are the points associated with the plane.
     *@param bounds are any bounds which the intersection must lie within.
     *@return true if there is a matching intersection.
     */
    virtual bool
    intersects(std::shared_ptr<PlanetModel> planetModel,
               std::shared_ptr<Plane> p,
               std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
               std::deque<std::shared_ptr<Membership>> &bounds);

    /** Determine if this endpoint intersects a GeoShape.
     *@param geoShape is the GeoShape.
     *@return true if there is shape intersect this endpoint.
     */
    virtual bool intersects(std::shared_ptr<GeoShape> geoShape);

    /** Get the bounds for a segment endpoint.
     *@param planetModel is the planet model.
     *@param bounds are the bounds to be modified.
     */
    virtual void getBounds(std::shared_ptr<PlanetModel> planetModel,
                           std::shared_ptr<Bounds> bounds);

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();
  };

  /**
   * This is the pre-calculated data for a path segment.
   */
private:
  class PathSegment : public std::enable_shared_from_this<PathSegment>
  {
    GET_CLASS_NAME(PathSegment)
    /** Starting point of the segment */
  public:
    const std::shared_ptr<GeoPoint> start;
    /** End point of the segment */
    const std::shared_ptr<GeoPoint> end;
    /** Place to keep any complete segment distances we've calculated so far */
    const std::unordered_map<std::shared_ptr<DistanceStyle>, double>
        fullDistanceCache =
            std::unordered_map<std::shared_ptr<DistanceStyle>, double>();
    /** Normalized plane connecting the two points and going through world
     * center */
    const std::shared_ptr<Plane> normalizedConnectingPlane;
    /** Cutoff plane parallel to connecting plane representing one side of the
     * path segment */
    const std::shared_ptr<SidedPlane> upperConnectingPlane;
    /** Cutoff plane parallel to connecting plane representing the other side of
     * the path segment */
    const std::shared_ptr<SidedPlane> lowerConnectingPlane;
    /** Plane going through the center and start point, marking the start edge
     * of the segment */
    const std::shared_ptr<SidedPlane> startCutoffPlane;
    /** Plane going through the center and end point, marking the end edge of
     * the segment */
    const std::shared_ptr<SidedPlane> endCutoffPlane;
    /** Upper right hand corner of segment */
    const std::shared_ptr<GeoPoint> URHC;
    /** Lower right hand corner of segment */
    const std::shared_ptr<GeoPoint> LRHC;
    /** Upper left hand corner of segment */
    const std::shared_ptr<GeoPoint> ULHC;
    /** Lower left hand corner of segment */
    const std::shared_ptr<GeoPoint> LLHC;
    /** Notable points for the upper connecting plane */
    std::deque<std::shared_ptr<GeoPoint>> const upperConnectingPlanePoints;
    /** Notable points for the lower connecting plane */
    std::deque<std::shared_ptr<GeoPoint>> const lowerConnectingPlanePoints;
    /** Notable points for the start cutoff plane */
    std::deque<std::shared_ptr<GeoPoint>> const startCutoffPlanePoints;
    /** Notable points for the end cutoff plane */
    std::deque<std::shared_ptr<GeoPoint>> const endCutoffPlanePoints;

    /** Construct a path segment.
     *@param planetModel is the planet model.
     *@param start is the starting point.
     *@param end is the ending point.
     *@param normalizedConnectingPlane is the connecting plane.
     *@param planeBoundingOffset is the linear offset from the connecting plane
     *to either side.
     */
    PathSegment(std::shared_ptr<PlanetModel> planetModel,
                std::shared_ptr<GeoPoint> start, std::shared_ptr<GeoPoint> end,
                std::shared_ptr<Plane> normalizedConnectingPlane,
                double const planeBoundingOffset);

    /** Compute the full distance along this path segment.
     *@param distanceStyle is the distance style.
     *@return the distance metric, in aggregation form.
     */
    virtual double
    fullPathDistance(std::shared_ptr<DistanceStyle> distanceStyle);

    /** Check if point is within this segment.
     *@param point is the point.
     *@return true of within.
     */
    virtual bool isWithin(std::shared_ptr<Vector> point);

    /** Check if point is within this segment.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return true of within.
     */
    virtual bool isWithin(double const x, double const y, double const z);

    /** Compute path center distance.
     *@param planetModel is the planet model.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, or Double.POSITIVE_INFINITY if outside this
     *segment
     */
    virtual double
    pathCenterDistance(std::shared_ptr<PlanetModel> planetModel,
                       std::shared_ptr<DistanceStyle> distanceStyle,
                       double const x, double const y, double const z);

    /** Compute nearest path distance.
     *@param planetModel is the planet model.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, in aggregation form, or
     *Double.POSITIVE_INFINITY if outside this segment
     */
    virtual double
    nearestPathDistance(std::shared_ptr<PlanetModel> planetModel,
                        std::shared_ptr<DistanceStyle> distanceStyle,
                        double const x, double const y, double const z);

    /** Compute delta path distance.
     *@param planetModel is the planet model.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, in aggregation form, or
     *Double.POSITIVE_INFINITY if outside the segment.
     */
    virtual double
    pathDeltaDistance(std::shared_ptr<PlanetModel> planetModel,
                      std::shared_ptr<DistanceStyle> distanceStyle,
                      double const x, double const y, double const z);

    /** Compute interior path distance.
     *@param planetModel is the planet model.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric, in aggregation form.
     */
    virtual double pathDistance(std::shared_ptr<PlanetModel> planetModel,
                                std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y, double const z);

    /** Compute external distance.
     *@param planetModel is the planet model.
     *@param distanceStyle is the distance style.
     *@param x is the point x.
     *@param y is the point y.
     *@param z is the point z.
     *@return the distance metric.
     */
    virtual double outsideDistance(std::shared_ptr<PlanetModel> planetModel,
                                   std::shared_ptr<DistanceStyle> distanceStyle,
                                   double const x, double const y,
                                   double const z);

    /** Determine if this endpoint intersects a specified plane.
     *@param planetModel is the planet model.
     *@param p is the plane.
     *@param notablePoints are the points associated with the plane.
     *@param bounds are any bounds which the intersection must lie within.
     *@return true if there is a matching intersection.
     */
    virtual bool
    intersects(std::shared_ptr<PlanetModel> planetModel,
               std::shared_ptr<Plane> p,
               std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
               std::deque<std::shared_ptr<Membership>> &bounds);

    /** Determine if this endpoint intersects a specified GeoShape.
     *@param geoShape is the GeoShape.
     *@return true if there GeoShape intersects this endpoint.
     */
    virtual bool intersects(std::shared_ptr<GeoShape> geoShape);

    /** Get the bounds for a segment endpoint.
     *@param planetModel is the planet model.
     *@param bounds are the bounds to be modified.
     */
    virtual void getBounds(std::shared_ptr<PlanetModel> planetModel,
                           std::shared_ptr<Bounds> bounds);
  };

protected:
  std::shared_ptr<GeoStandardPath> shared_from_this()
  {
    return std::static_pointer_cast<GeoStandardPath>(
        GeoBasePath::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
