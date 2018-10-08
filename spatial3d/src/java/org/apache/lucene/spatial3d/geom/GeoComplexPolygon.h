#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Tree.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Edge.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/DistanceStyle.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/CountingEdgeIterator.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/XYZBounds.h"

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
 * GeoComplexPolygon objects are structures designed to handle very large
 * numbers of edges. They perform very well in this case compared to the
 * alternatives, which all have O(N) evaluation and O(N^2) setup times.  Complex
 * polygons have O(N) setup times and best case O(log(N)) evaluation times.
 *
 * The tradeoff is that these objects perform object creation when evaluating
 * intersects() and isWithin().
 *
 * @lucene.internal
 */
class GeoComplexPolygon : public GeoBasePolygon
{
  GET_CLASS_NAME(GeoComplexPolygon)

private:
  const std::shared_ptr<Tree> xTree;
  const std::shared_ptr<Tree> yTree;
  const std::shared_ptr<Tree> zTree;

  const std::deque<std::deque<std::shared_ptr<GeoPoint>>> pointsList;

  const bool testPoint1InSet;
  const std::shared_ptr<GeoPoint> testPoint1;

  const bool testPoint2InSet;
  const std::shared_ptr<GeoPoint> testPoint2;

  const std::shared_ptr<Plane> testPoint1FixedYPlane;
  const std::shared_ptr<Plane> testPoint1FixedYAbovePlane;
  const std::shared_ptr<Plane> testPoint1FixedYBelowPlane;
  const std::shared_ptr<Plane> testPoint1FixedXPlane;
  const std::shared_ptr<Plane> testPoint1FixedXAbovePlane;
  const std::shared_ptr<Plane> testPoint1FixedXBelowPlane;
  const std::shared_ptr<Plane> testPoint1FixedZPlane;
  const std::shared_ptr<Plane> testPoint1FixedZAbovePlane;
  const std::shared_ptr<Plane> testPoint1FixedZBelowPlane;

  const std::shared_ptr<Plane> testPoint2FixedYPlane;
  const std::shared_ptr<Plane> testPoint2FixedYAbovePlane;
  const std::shared_ptr<Plane> testPoint2FixedYBelowPlane;
  const std::shared_ptr<Plane> testPoint2FixedXPlane;
  const std::shared_ptr<Plane> testPoint2FixedXAbovePlane;
  const std::shared_ptr<Plane> testPoint2FixedXBelowPlane;
  const std::shared_ptr<Plane> testPoint2FixedZPlane;
  const std::shared_ptr<Plane> testPoint2FixedZAbovePlane;
  const std::shared_ptr<Plane> testPoint2FixedZBelowPlane;

  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;
  std::deque<std::shared_ptr<Edge>> const shapeStartEdges;

  static const double NEAR_EDGE_CUTOFF;

  /**
   * Create a complex polygon from multiple lists of points, and a single point
   *which is known to be in or out of set.
   *@param planetModel is the planet model.
   *@param pointsList is the deque of lists of edge points.  The edge points
   *describe edges, and have an implied return boundary, so that N edges require
   *N points.  These points have furthermore been filtered so that no adjacent
   *points are identical (within the bounds of the definition used by this
   *package).  It is assumed that no edges intersect, but the structure can
   *contain both outer rings as well as holes.
   *@param testPoint is the point whose in/out of setness is known.
   *@param testPointInSet is true if the test point is considered "within" the
   *polygon.
   */
public:
  GeoComplexPolygon(
      std::shared_ptr<PlanetModel> planetModel,
      std::deque<std::deque<std::shared_ptr<GeoPoint>>> &pointsList,
      std::shared_ptr<GeoPoint> testPoint, bool const testPointInSet);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoComplexPolygon(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

private:
  static std::deque<std::deque<std::shared_ptr<GeoPoint>>>
  readPointsList(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<InputStream> inputStream) ;

public:
  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

private:
  static void
  writePointsList(std::shared_ptr<OutputStream> outputStream,
                  std::deque<std::deque<std::shared_ptr<GeoPoint>>>
                      &pointsList) ;

public:
  bool isWithin(double const x, double const y, double const z) override;

  /** Given a test point, whether it is in set, and the associated planes,
   * figure out if another point is in set or not.
   */
private:
  bool isInSet(double const x, double const y, double const z,
               std::shared_ptr<GeoPoint> testPoint, bool const testPointInSet,
               std::shared_ptr<Plane> testPointFixedXPlane,
               std::shared_ptr<Plane> testPointFixedXAbovePlane,
               std::shared_ptr<Plane> testPointFixedXBelowPlane,
               std::shared_ptr<Plane> testPointFixedYPlane,
               std::shared_ptr<Plane> testPointFixedYAbovePlane,
               std::shared_ptr<Plane> testPointFixedYBelowPlane,
               std::shared_ptr<Plane> testPointFixedZPlane,
               std::shared_ptr<Plane> testPointFixedZAbovePlane,
               std::shared_ptr<Plane> testPointFixedZBelowPlane);

public:
  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

protected:
  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

  /** Create a linear crossing edge iterator with the appropriate cutoff planes
   * given the geometry.
   */
private:
  std::shared_ptr<CountingEdgeIterator> createLinearCrossingEdgeIterator(
      std::shared_ptr<GeoPoint> testPoint, std::shared_ptr<Plane> plane,
      std::shared_ptr<Plane> abovePlane, std::shared_ptr<Plane> belowPlane,
      double const thePointX, double const thePointY, double const thePointZ);

  static std::deque<double> const halfProportions;

  /**
   * An instance of this class describes a single edge, and includes what is
   * necessary to reliably determine intersection in the context of the even/odd
   * algorithm used.
   */
private:
  class Edge : public std::enable_shared_from_this<Edge>
  {
    GET_CLASS_NAME(Edge)
  public:
    const std::shared_ptr<GeoPoint> startPoint;
    const std::shared_ptr<GeoPoint> endPoint;
    std::deque<std::shared_ptr<GeoPoint>> const notablePoints;
    const std::shared_ptr<SidedPlane> startPlane;
    const std::shared_ptr<SidedPlane> endPlane;
    const std::shared_ptr<SidedPlane> backingPlane;
    const std::shared_ptr<Plane> plane;
    const std::shared_ptr<XYZBounds> planeBounds;
    std::shared_ptr<Edge> previous = nullptr;
    std::shared_ptr<Edge> next = nullptr;

    Edge(std::shared_ptr<PlanetModel> pm, std::shared_ptr<GeoPoint> startPoint,
         std::shared_ptr<GeoPoint> endPoint);

    virtual bool isWithin(double const thePointX, double const thePointY,
                          double const thePointZ);

    // Hashcode and equals are system default!!
  };

  /** Strategy class for describing traversals.
   * Implements Comparable so that these can be ordered by Collections.sort().
   */
private:
  class TraversalStrategy
      : public std::enable_shared_from_this<TraversalStrategy>,
        public Comparable<std::shared_ptr<TraversalStrategy>>
  {
    GET_CLASS_NAME(TraversalStrategy)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    const double traversalDistance;
    const double firstLegValue;
    const double secondLegValue;
    const std::shared_ptr<Plane> firstLegPlane;
    const std::shared_ptr<Plane> firstLegAbovePlane;
    const std::shared_ptr<Plane> firstLegBelowPlane;
    const std::shared_ptr<Plane> secondLegPlane;
    const std::shared_ptr<Plane> secondLegAbovePlane;
    const std::shared_ptr<Plane> secondLegBelowPlane;
    const std::shared_ptr<Tree> firstLegTree;
    const std::shared_ptr<Tree> secondLegTree;
    const std::shared_ptr<GeoPoint> intersectionPoint;

  public:
    TraversalStrategy(std::shared_ptr<GeoComplexPolygon> outerInstance,
                      double const traversalDistance,
                      double const firstLegValue, double const secondLegValue,
                      std::shared_ptr<Plane> firstLegPlane,
                      std::shared_ptr<Plane> firstLegAbovePlane,
                      std::shared_ptr<Plane> firstLegBelowPlane,
                      std::shared_ptr<Plane> secondLegPlane,
                      std::shared_ptr<Plane> secondLegAbovePlane,
                      std::shared_ptr<Plane> secondLegBelowPlane,
                      std::shared_ptr<Tree> firstLegTree,
                      std::shared_ptr<Tree> secondLegTree,
                      std::shared_ptr<GeoPoint> intersectionPoint);

    virtual bool apply(std::shared_ptr<GeoPoint> testPoint,
                       bool const testPointInSet, double const x,
                       double const y, double const z);

    int compareTo(std::shared_ptr<TraversalStrategy> other) override;
  };

  /**
   * Iterator execution interface, for tree traversal.  Pass an object
   * implementing this interface into the traversal method of a tree, and each
   * edge that matches will cause this object to be called.
   */
private:
  class EdgeIterator
  {
    GET_CLASS_NAME(EdgeIterator)
    /**
     * @param edge is the edge that matched.
     * @return true if the iteration should continue, false otherwise.
     */
  public:
    virtual bool matches(std::shared_ptr<Edge> edge) = 0;
  };

  /**
   * Iterator execution interface, for tree traversal, plus count retrieval.
   * Pass an object implementing this interface into the traversal method of a
   * tree, and each edge that matches will cause this object to be called.
   */
private:
  class CountingEdgeIterator : public EdgeIterator
  {
    GET_CLASS_NAME(CountingEdgeIterator)
    /**
     * @return the number of edges that were crossed.
     */
  public:
    virtual int getCrossingCount() = 0;

    /**
     * @return true if the endpoint was on an edge.
     */
    virtual bool isOnEdge() = 0;
  };

  /**
   * An instance of this class represents a node in a tree.  The tree is
   * designed to be given a value and from that to iterate over a deque of edges.
   * In order to do this efficiently, each new edge is dropped into the tree
   * using its minimum and maximum value.  If the new edge's value does not
   * overlap the range, then it gets added either to the lesser side or the
   * greater side, accordingly.  If it does overlap, then the "overlapping"
   * chain is instead traversed.
   *
   * This class is generic and can be used for any definition of "value".
   *
   */
private:
  class Node : public std::enable_shared_from_this<Node>
  {
    GET_CLASS_NAME(Node)
  public:
    const std::shared_ptr<Edge> edge;
    const double low;
    const double high;
    std::shared_ptr<Node> left = nullptr;
    std::shared_ptr<Node> right = nullptr;
    double max = 0;

    Node(std::shared_ptr<Edge> edge, double const minimumValue,
         double const maximumValue);

    virtual bool traverse(std::shared_ptr<EdgeIterator> edgeIterator,
                          double const minValue, double const maxValue);
  };

  /** An interface describing a tree.
   */
private:
  class Tree : public std::enable_shared_from_this<Tree>
  {
    GET_CLASS_NAME(Tree)
  private:
    const std::shared_ptr<Node> rootNode;

  protected:
    static std::deque<std::shared_ptr<Edge>> const EMPTY_ARRAY;

    /** Constructor.
     * @param allEdges is the deque of all edges for the tree.
     */
  public:
    Tree(std::deque<std::shared_ptr<Edge>> &allEdges);

  private:
    static std::shared_ptr<Node>
    createTree(std::deque<std::shared_ptr<Node>> &edges, int const low,
               int const high);

    /** Get the minimum value from the edge.
     * @param edge is the edge.
     * @return the minimum value.
     */
  protected:
    virtual double getMinimum(std::shared_ptr<Edge> edge) = 0;

    /** Get the maximum value from the edge.
     * @param edge is the edge.
     * @return the maximum value.
     */
    virtual double getMaximum(std::shared_ptr<Edge> edge) = 0;

    /** Traverse the tree, finding all edges that intersect the provided value.
     * @param edgeIterator provides the method to call for any encountered
     * matching edge.
     * @param value is the value to match.
     * @return false if the traversal was aborted before completion.
     */
  public:
    virtual bool traverse(std::shared_ptr<EdgeIterator> edgeIterator,
                          double const value);

    /** Traverse the tree, finding all edges that intersect the provided value
     * range.
     * @param edgeIterator provides the method to call for any encountered
     * matching edge. Edges will not be invoked more than once.
     * @param minValue is the minimum value.
     * @param maxValue is the maximum value.
     * @return false if the traversal was aborted before completion.
     */
    virtual bool traverse(std::shared_ptr<EdgeIterator> edgeIterator,
                          double const minValue, double const maxValue);
  };

  /** This is the z-tree.
   */
private:
  class ZTree : public Tree
  {
    GET_CLASS_NAME(ZTree)
  public:
    std::shared_ptr<Node> rootNode = nullptr;

    ZTree(std::deque<std::shared_ptr<Edge>> &allEdges);

    /*
    @Override
    public bool traverse(final EdgeIterator edgeIterator, final double value)
    { System.err.println("Traversing in Z, value= "+value+"..."); return
    super.traverse(edgeIterator, value);
    }
    */

  protected:
    double getMinimum(std::shared_ptr<Edge> edge) override;

    double getMaximum(std::shared_ptr<Edge> edge) override;

  protected:
    std::shared_ptr<ZTree> shared_from_this()
    {
      return std::static_pointer_cast<ZTree>(Tree::shared_from_this());
    }
  };

  /** This is the y-tree.
   */
private:
  class YTree : public Tree
  {
    GET_CLASS_NAME(YTree)

  public:
    YTree(std::deque<std::shared_ptr<Edge>> &allEdges);

    /*
    @Override
    public bool traverse(final EdgeIterator edgeIterator, final double value)
    { System.err.println("Traversing in Y, value= "+value+"..."); return
    super.traverse(edgeIterator, value);
    }
    */

  protected:
    double getMinimum(std::shared_ptr<Edge> edge) override;

    double getMaximum(std::shared_ptr<Edge> edge) override;

  protected:
    std::shared_ptr<YTree> shared_from_this()
    {
      return std::static_pointer_cast<YTree>(Tree::shared_from_this());
    }
  };

  /** This is the x-tree.
   */
private:
  class XTree : public Tree
  {
    GET_CLASS_NAME(XTree)

  public:
    XTree(std::deque<std::shared_ptr<Edge>> &allEdges);

    /*
    @Override
    public bool traverse(final EdgeIterator edgeIterator, final double value)
    { System.err.println("Traversing in X, value= "+value+"..."); return
    super.traverse(edgeIterator, value);
    }
    */

  protected:
    double getMinimum(std::shared_ptr<Edge> edge) override;

    double getMaximum(std::shared_ptr<Edge> edge) override;

  protected:
    std::shared_ptr<XTree> shared_from_this()
    {
      return std::static_pointer_cast<XTree>(Tree::shared_from_this());
    }
  };

  /** Assess whether edge intersects the provided plane plus bounds.
   */
private:
  class IntersectorEdgeIterator
      : public std::enable_shared_from_this<IntersectorEdgeIterator>,
        public EdgeIterator
  {
    GET_CLASS_NAME(IntersectorEdgeIterator)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    const std::shared_ptr<Plane> plane;
    std::deque<std::shared_ptr<GeoPoint>> const notablePoints;
    std::deque<std::shared_ptr<Membership>> const bounds;

  public:
    IntersectorEdgeIterator(
        std::shared_ptr<GeoComplexPolygon> outerInstance,
        std::shared_ptr<Plane> plane,
        std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
        std::deque<Membership> &bounds);

    bool matches(std::shared_ptr<Edge> edge) override;
  };

  /** Assess whether edge intersects the provided shape.
   */
private:
  class IntersectorShapeIterator
      : public std::enable_shared_from_this<IntersectorShapeIterator>,
        public EdgeIterator
  {
    GET_CLASS_NAME(IntersectorShapeIterator)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    const std::shared_ptr<GeoShape> shape;

  public:
    IntersectorShapeIterator(std::shared_ptr<GeoComplexPolygon> outerInstance,
                             std::shared_ptr<GeoShape> shape);

    bool matches(std::shared_ptr<Edge> edge) override;
  };

  /*
  private void debugIntersectAllEdges(final Plane travelPlane, final
  Membership... bounds) { System.out.println("\n The following edges intersect
  the travel plane within the given bounds:"); for (final Edge startEdge :
  shapeStartEdges) { Edge currentEdge = startEdge; while (true) {
        System.out.println("  Edge "+currentEdge.startPoint+" -->
  "+currentEdge.endPoint+":"); final GeoPoint[] intersectionPoints =
  travelPlane.findIntersections(planetModel, currentEdge.plane, bounds, new
  Membership[]{currentEdge.startPlane, currentEdge.endPlane,
  currentEdge.backingPlane}); if (intersectionPoints == null ||
  intersectionPoints.length > 0) { System.out.println("   ... intersects!!"); }
  else { final GeoPoint[] unboundedPoints =
  travelPlane.findIntersections(planetModel, currentEdge.plane); for (final
  GeoPoint point : unboundedPoints) { for (final Membership bound : bounds) { if
  (!bound.isWithin(point)) { System.out.println("   ... intersection "+point+"
  excluded by iterator bound ("+((SidedPlane)bound).evaluate(point)+")");
              }
            }
            if (!currentEdge.startPlane.isWithin(point)) {
              System.out.println("   ... intersection "+point+" excluded by edge
  start plane ("+((SidedPlane)currentEdge.startPlane).evaluate(point)+")");
            }
            if (!currentEdge.endPlane.isWithin(point)) {
              System.out.println("   ... intersection "+point+" excluded by edge
  end plane ("+((SidedPlane)currentEdge.endPlane).evaluate(point)+")");
            }
            if (!currentEdge.backingPlane.isWithin(point)) {
              System.out.println("   ... intersection "+point+" excluded by edge
  backing plane ("+((SidedPlane)currentEdge.backingPlane).evaluate(point)+")");
            }
          }
        }
        currentEdge = currentEdge.next;
        if (currentEdge == startEdge) {
          break;
        }
      }
    }
    System.out.println(" ...done\n");
  }
  */

  /** Count the number of verifiable edge crossings for a full 1/2 a world.
   */
private:
  class FullLinearCrossingEdgeIterator
      : public std::enable_shared_from_this<FullLinearCrossingEdgeIterator>,
        public CountingEdgeIterator
  {
    GET_CLASS_NAME(FullLinearCrossingEdgeIterator)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    const std::shared_ptr<GeoPoint> testPoint;
    const std::shared_ptr<Plane> plane;
    const std::shared_ptr<Plane> abovePlane;
    const std::shared_ptr<Plane> belowPlane;
    const std::shared_ptr<Membership> bound;
    const double thePointX;
    const double thePointY;
    const double thePointZ;

    bool onEdge = false;
    int aboveCrossingCount = 0;
    int belowCrossingCount = 0;

  public:
    FullLinearCrossingEdgeIterator(
        std::shared_ptr<GeoComplexPolygon> outerInstance,
        std::shared_ptr<GeoPoint> testPoint, std::shared_ptr<Plane> plane,
        std::shared_ptr<Plane> abovePlane, std::shared_ptr<Plane> belowPlane,
        double const thePointX, double const thePointY, double const thePointZ);

    int getCrossingCount() override;

    bool isOnEdge() override;

    bool matches(std::shared_ptr<Edge> edge) override;

    /** Find the intersections with an envelope plane, and assess those
     * intersections for whether they truly describe crossings.
     */
  private:
    int countCrossings(std::shared_ptr<Edge> edge,
                       std::shared_ptr<Plane> envelopePlane,
                       std::shared_ptr<Membership> envelopeBound);

    bool edgeCrossesEnvelope(std::shared_ptr<Plane> edgePlane,
                             std::shared_ptr<GeoPoint> intersectionPoint,
                             std::shared_ptr<Plane> envelopePlane);
  };

  /** Count the number of verifiable edge crossings for less than 1/2 a world.
   */
private:
  class SectorLinearCrossingEdgeIterator
      : public std::enable_shared_from_this<SectorLinearCrossingEdgeIterator>,
        public CountingEdgeIterator
  {
    GET_CLASS_NAME(SectorLinearCrossingEdgeIterator)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    const std::shared_ptr<GeoPoint> testPoint;
    const std::shared_ptr<Plane> plane;
    const std::shared_ptr<Plane> abovePlane;
    const std::shared_ptr<Plane> belowPlane;
    const std::shared_ptr<Membership> bound1;
    const std::shared_ptr<Membership> bound2;
    const double thePointX;
    const double thePointY;
    const double thePointZ;

    bool onEdge = false;
    int aboveCrossingCount = 0;
    int belowCrossingCount = 0;

  public:
    SectorLinearCrossingEdgeIterator(
        std::shared_ptr<GeoComplexPolygon> outerInstance,
        std::shared_ptr<GeoPoint> testPoint, std::shared_ptr<Plane> plane,
        std::shared_ptr<Plane> abovePlane, std::shared_ptr<Plane> belowPlane,
        double const thePointX, double const thePointY, double const thePointZ);

    int getCrossingCount() override;

    bool isOnEdge() override;

    bool matches(std::shared_ptr<Edge> edge) override;

    /** Find the intersections with an envelope plane, and assess those
     * intersections for whether they truly describe crossings.
     */
  private:
    int countCrossings(std::shared_ptr<Edge> edge,
                       std::shared_ptr<Plane> envelopePlane,
                       std::shared_ptr<Membership> envelopeBound1,
                       std::shared_ptr<Membership> envelopeBound2);

    bool edgeCrossesEnvelope(std::shared_ptr<Plane> edgePlane,
                             std::shared_ptr<GeoPoint> intersectionPoint,
                             std::shared_ptr<Plane> envelopePlane);
  };

  /** Count the number of verifiable edge crossings for a dual-leg journey.
   */
private:
  class DualCrossingEdgeIterator
      : public std::enable_shared_from_this<DualCrossingEdgeIterator>,
        public CountingEdgeIterator
  {
    GET_CLASS_NAME(DualCrossingEdgeIterator)
  private:
    std::shared_ptr<GeoComplexPolygon> outerInstance;

    // This is a hash of which edges we've already looked at and tallied, so we
    // don't repeat ourselves. It is lazily initialized since most transitions
    // cross no edges at all.
    std::shared_ptr<Set<std::shared_ptr<Edge>>> seenEdges = nullptr;

    const std::shared_ptr<GeoPoint> testPoint;
    const std::shared_ptr<Plane> testPointPlane;
    const std::shared_ptr<Plane> testPointAbovePlane;
    const std::shared_ptr<Plane> testPointBelowPlane;
    const std::shared_ptr<Plane> travelPlane;
    const std::shared_ptr<Plane> travelAbovePlane;
    const std::shared_ptr<Plane> travelBelowPlane;
    const double thePointX;
    const double thePointY;
    const double thePointZ;

    const std::shared_ptr<GeoPoint> intersectionPoint;

    const std::shared_ptr<SidedPlane> testPointCutoffPlane;
    const std::shared_ptr<SidedPlane> checkPointCutoffPlane;
    const std::shared_ptr<SidedPlane> testPointOtherCutoffPlane;
    const std::shared_ptr<SidedPlane> checkPointOtherCutoffPlane;

    // These are computed on an as-needed basis

    bool computedInsideOutside = false;
    std::shared_ptr<Plane> testPointInsidePlane;
    std::shared_ptr<Plane> testPointOutsidePlane;
    std::shared_ptr<Plane> travelInsidePlane;
    std::shared_ptr<Plane> travelOutsidePlane;
    std::shared_ptr<SidedPlane> insideTestPointCutoffPlane;
    std::shared_ptr<SidedPlane> insideTravelCutoffPlane;
    std::shared_ptr<SidedPlane> outsideTestPointCutoffPlane;
    std::shared_ptr<SidedPlane> outsideTravelCutoffPlane;

    // The counters
    bool onEdge = false;
    int innerCrossingCount = 0;
    int outerCrossingCount = 0;

  public:
    DualCrossingEdgeIterator(std::shared_ptr<GeoComplexPolygon> outerInstance,
                             std::shared_ptr<GeoPoint> testPoint,
                             std::shared_ptr<Plane> testPointPlane,
                             std::shared_ptr<Plane> testPointAbovePlane,
                             std::shared_ptr<Plane> testPointBelowPlane,
                             std::shared_ptr<Plane> travelPlane,
                             std::shared_ptr<Plane> travelAbovePlane,
                             std::shared_ptr<Plane> travelBelowPlane,
                             double const thePointX, double const thePointY,
                             double const thePointZ,
                             std::shared_ptr<GeoPoint> intersectionPoint);

  protected:
    virtual void computeInsideOutside();

  private:
    std::shared_ptr<GeoPoint>
    pickProximate(std::deque<std::shared_ptr<GeoPoint>> &points);

  public:
    int getCrossingCount() override;

    bool isOnEdge() override;

    bool matches(std::shared_ptr<Edge> edge) override;

    /** Find the intersections with a pair of envelope planes, and assess those
     * intersections for duplication and for whether they truly describe
     * crossings.
     */
  private:
    int countCrossings(std::shared_ptr<Edge> edge,
                       std::shared_ptr<Plane> travelEnvelopePlane,
                       std::shared_ptr<Membership> travelEnvelopeBound1,
                       std::shared_ptr<Membership> travelEnvelopeBound2,
                       std::shared_ptr<Plane> testPointEnvelopePlane,
                       std::shared_ptr<Membership> testPointEnvelopeBound1,
                       std::shared_ptr<Membership> testPointEnvelopeBound2);

    /** Return true if the edge crosses the envelope plane, given the envelope
     * intersection point.
     */
    bool edgeCrossesEnvelope(std::shared_ptr<Plane> edgePlane,
                             std::shared_ptr<GeoPoint> intersectionPoint,
                             std::shared_ptr<Plane> envelopePlane);
  };

  /** This is the amount we go, roughly, in both directions, to find adjoining
   * points to test.  If we go too far, we might miss a transition, but if we go
   * too little, we might not see it either due to numerical issues.
   */
private:
  static constexpr double DELTA_DISTANCE = Vector::MINIMUM_RESOLUTION;
  /** This is the maximum number of iterations.  If we get this high,
   * effectively the planes are parallel, and we treat that as a crossing.
   */
  static constexpr int MAX_ITERATIONS = 100;
  /** This is the amount off of the envelope plane that we count as "enough" for
   * a valid crossing assessment. */
  static const double OFF_PLANE_AMOUNT;

  /** Given a point on the plane and the ellipsoid, this method looks for a pair
   * of adjoining points on either side of the plane, which are about
   * MINIMUM_RESOLUTION away from the given point.  This only works for planes
   * which go through the center of the world. Returns null if the planes are
   * effectively parallel and reasonable adjoining points cannot be determined.
   */
  std::deque<std::shared_ptr<GeoPoint>>
  findAdjoiningPoints(std::shared_ptr<Plane> plane,
                      std::shared_ptr<GeoPoint> pointOnPlane,
                      std::shared_ptr<Plane> envelopePlane);

  static double
  computeSquaredDistance(std::shared_ptr<GeoPoint> checkPoint,
                         std::shared_ptr<GeoPoint> intersectionPoint);

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

private:
  static void fillInEdgeDescription(std::shared_ptr<StringBuilder> description,
                                    std::shared_ptr<Edge> startEdge);

protected:
  std::shared_ptr<GeoComplexPolygon> shared_from_this()
  {
    return std::static_pointer_cast<GeoComplexPolygon>(
        GeoBasePolygon::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
