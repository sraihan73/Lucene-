#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygon.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/DistanceStyle.h"

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
 * GeoConcavePolygon objects are generic building blocks of more complex
 * structures. The only restrictions on these objects are: (1) they must be
 * concave; (2) they must have a maximum extent larger than PI.  Violating
 * either one of these limits will cause the logic to fail.
 *
 * @lucene.internal
 */
class GeoConcavePolygon : public GeoBasePolygon
{
  GET_CLASS_NAME(GeoConcavePolygon)
  /** The deque of polygon points */
protected:
  const std::deque<std::shared_ptr<GeoPoint>> points;
  /** A bitset describing, for each edge, whether it is internal or not */
  const std::shared_ptr<BitSet> isInternalEdges;
  /** The deque of holes.  If a point is in the hole, it is *not* in the polygon
   */
  const std::deque<std::shared_ptr<GeoPolygon>> holes;

  /** A deque of edges */
  std::deque<std::shared_ptr<SidedPlane>> edges;
  /** A deque of inverted edges */
  std::deque<std::shared_ptr<SidedPlane>> invertedEdges;
  /** The set of notable points for each edge */
  std::deque<std::deque<std::shared_ptr<GeoPoint>>> notableEdgePoints;
  /** A point which is on the boundary of the polygon */
  std::deque<std::shared_ptr<GeoPoint>> edgePoints;
  /** Set to true when the polygon is complete */
  bool isDone = false;
  /** A bounds object for each sided plane */
  std::unordered_map<std::shared_ptr<SidedPlane>, std::shared_ptr<Membership>>
      eitherBounds;
  /** Map from edge to its previous non-coplanar brother */
  std::unordered_map<std::shared_ptr<SidedPlane>, std::shared_ptr<SidedPlane>>
      prevBrotherMap;
  /** Map from edge to its next non-coplanar brother */
  std::unordered_map<std::shared_ptr<SidedPlane>, std::shared_ptr<SidedPlane>>
      nextBrotherMap;

  /**
   * Create a concave polygon from a deque of points.  The first point must be on
   *the external edge.
   *@param planetModel is the planet model.
   *@param pointList is the deque of points to create the polygon from.
   */
public:
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    std::deque<std::shared_ptr<GeoPoint>> &pointList);

  /**
   * Create a concave polygon from a deque of points.  The first point must be on
   *the external edge.
   *@param planetModel is the planet model.
   *@param pointList is the deque of points to create the polygon from.
   *@param holes is the deque of GeoPolygon objects that describe holes in the
   *concave polygon.  Null == no holes.
   */
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    std::deque<std::shared_ptr<GeoPoint>> &pointList,
                    std::deque<std::shared_ptr<GeoPolygon>> &holes);

  /**
   * Create a concave polygon from a deque of points, keeping track of which
   *boundaries are internal.  This is used when creating a polygon as a building
   *block for another shape.
   *@param planetModel is the planet model.
   *@param pointList is the set of points to create the polygon from.
   *@param internalEdgeFlags is a bitset describing whether each edge is
   *internal or not.
   *@param returnEdgeInternal is true when the final return edge is an internal
   *one.
   */
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    std::deque<std::shared_ptr<GeoPoint>> &pointList,
                    std::shared_ptr<BitSet> internalEdgeFlags,
                    bool const returnEdgeInternal);

  /**
   * Create a concave polygon from a deque of points, keeping track of which
   *boundaries are internal.  This is used when creating a polygon as a building
   *block for another shape.
   *@param planetModel is the planet model.
   *@param pointList is the set of points to create the polygon from.
   *@param holes is the deque of GeoPolygon objects that describe holes in the
   *concave polygon.  Null == no holes.
   *@param internalEdgeFlags is a bitset describing whether each edge is
   *internal or not.
   *@param returnEdgeInternal is true when the final return edge is an internal
   *one.
   */
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    std::deque<std::shared_ptr<GeoPoint>> &pointList,
                    std::deque<std::shared_ptr<GeoPolygon>> &holes,
                    std::shared_ptr<BitSet> internalEdgeFlags,
                    bool const returnEdgeInternal);

  /**
   * Create a concave polygon, with a starting latitude and longitude.
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   *lon: {@code -PI -> PI}
   *@param planetModel is the planet model.
   *@param startLatitude is the latitude of the first point.
   *@param startLongitude is the longitude of the first point.
   */
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    double const startLatitude, double const startLongitude);

  /**
   * Create a concave polygon, with a starting latitude and longitude.
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   *lon: {@code -PI -> PI}
   *@param planetModel is the planet model.
   *@param startLatitude is the latitude of the first point.
   *@param startLongitude is the longitude of the first point.
   *@param holes is the deque of GeoPolygon objects that describe holes in the
   *concave polygon.  Null == no holes.
   */
  GeoConcavePolygon(std::shared_ptr<PlanetModel> planetModel,
                    double const startLatitude, double const startLongitude,
                    std::deque<std::shared_ptr<GeoPolygon>> &holes);

  /**
   * Add a point to the polygon.
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   * lon: {@code -PI -> PI}
   *
   * @param latitude       is the latitude of the next point.
   * @param longitude      is the longitude of the next point.
   * @param isInternalEdge is true if the edge just added with this point should
   * be considered "internal", and not intersected as part of the intersects()
   * operation.
   */
  virtual void addPoint(double const latitude, double const longitude,
                        bool const isInternalEdge);

  /**
   * Finish the polygon, by connecting the last added point with the starting
   *point.
   *@param isInternalReturnEdge is true if the return edge (back to start) is an
   *internal one.
   */
  virtual void done(bool const isInternalReturnEdge);

  /** Check if a point is within the provided holes.
   *@param point point to check.
   *@return true if the point is within any of the holes.
   */
protected:
  virtual bool isWithinHoles(std::shared_ptr<GeoPoint> point);

  /** Compute a legal point index from a possibly illegal one, that may have
   *wrapped.
   *@param index is the index.
   *@return the normalized index.
   */
  virtual int legalIndex(int index);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
public:
  GeoConcavePolygon(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  bool isWithin(double const x, double const y, double const z) override;

protected:
  virtual bool localIsWithin(std::shared_ptr<Vector> v);

  virtual bool localIsWithin(double const x, double const y, double const z);

public:
  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  /** A membership implementation representing polygon edges that must apply.
   */
protected:
  class EitherBound : public std::enable_shared_from_this<EitherBound>,
                      public Membership
  {
    GET_CLASS_NAME(EitherBound)

  protected:
    const std::shared_ptr<SidedPlane> sideBound1;
    const std::shared_ptr<SidedPlane> sideBound2;

    /** Constructor.
     * @param sideBound1 is the first side bound.
     * @param sideBound2 is the second side bound.
     */
  public:
    EitherBound(std::shared_ptr<SidedPlane> sideBound1,
                std::shared_ptr<SidedPlane> sideBound2);

    bool isWithin(std::shared_ptr<Vector> v) override;

    bool isWithin(double const x, double const y, double const z) override;

    virtual std::wstring toString();
  };

public:
  void getBounds(std::shared_ptr<Bounds> bounds) override;

protected:
  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoConcavePolygon> shared_from_this()
  {
    return std::static_pointer_cast<GeoConcavePolygon>(
        GeoBasePolygon::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
