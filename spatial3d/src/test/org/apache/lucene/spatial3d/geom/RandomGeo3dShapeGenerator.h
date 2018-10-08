#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Constraints.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoAreaShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPointShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoCircle.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoBBox.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPath.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoCompositeAreaShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygon.h"

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble;

/**
 * Class for generating random Geo3dShapes. They can be generated under
 * given constraints which are expressed as a shape and a relationship.
 *
 * note that convexity for polygons is defined as polygons that contains
 * antipodal points, otherwise they are convex. Internally they can be
 * created using GeoConvexPolygons and GeoConcavePolygons.
 *
 */
class RandomGeo3dShapeGenerator : public LuceneTestCase
{
  GET_CLASS_NAME(RandomGeo3dShapeGenerator)

  /* Max num of iterations to find right shape under given constrains */
private:
  static constexpr int MAX_SHAPE_ITERATIONS = 50;
  /* Max num of iterations to find right point under given constrains */
  static constexpr int MAX_POINT_ITERATIONS = 1000;

  /* Supported shapes */
protected:
  static constexpr int CONVEX_POLYGON = 0;
  static constexpr int CONVEX_POLYGON_WITH_HOLES = 1;
  static constexpr int CONCAVE_POLYGON = 2;
  static constexpr int CONCAVE_POLYGON_WITH_HOLES = 3;
  static constexpr int COMPLEX_POLYGON = 4;
  static constexpr int CIRCLE = 5;
  static constexpr int RECTANGLE = 6;
  static constexpr int PATH = 7;
  static constexpr int COLLECTION = 8;
  static constexpr int POINT = 9;
  static constexpr int LINE = 10;
  static constexpr int EXACT_CIRCLE = 11;

  /* Helper shapes for generating constraints whch are just three sided polygons
   */
  static constexpr int CONVEX_SIMPLE_POLYGON = 500;
  static constexpr int CONCAVE_SIMPLE_POLYGON = 501;

  /**
   * Method that returns a random generated Planet model from the supported
   * Planet models. currently SPHERE and WGS84
   *
   * @return a random generated Planet model
   */
public:
  virtual std::shared_ptr<PlanetModel> randomPlanetModel();

  /**
   * Method that returns a random generated a random Shape code from all
   * supported shapes.
   *
   * @return a random generated shape code
   */
  virtual int randomShapeType();

  /**
   * Method that returns a random generated GeoAreaShape code from all
   * supported GeoAreaShapes.
   *
   * We are removing Collections because it is difficult to create shapes
   * with properties in some cases.
   *
   * @return a random generated polygon code
   */
  virtual int randomGeoAreaShapeType();

  /**
   * Method that returns a random generated a random Shape code from all
   * convex supported shapes.
   *
   * @return a random generated convex shape code
   */
  virtual int randomConvexShapeType();

  /**
   * Method that returns a random generated a random Shape code from all
   * concave supported shapes.
   *
   * @return a random generated concave shape code
   */
  virtual int randomConcaveShapeType();

  /**
   * Check if a shape code represents a concave shape
   *
   * @return true if the shape represented by the code is concave
   */
  virtual bool isConcave(int shapeType);

  /**
   * Method that returns empty Constraints object..
   *
   * @return an empty Constraints object
   */
  virtual std::shared_ptr<Constraints> getEmptyConstraint();

  /**
   * Method that returns a random generated GeoPoint.
   *
   * @param planetModel The planet model.
   * @return The random generated GeoPoint.
   */
  virtual std::shared_ptr<GeoPoint>
  randomGeoPoint(std::shared_ptr<PlanetModel> planetModel);

  /**
   * Method that returns a random generated GeoPoint under given constraints.
   * Returns NULL if it cannot find a point under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPoint.
   */
  virtual std::shared_ptr<GeoPoint>
  randomGeoPoint(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated GeoAreaShape.
   *
   * @param shapeType The GeoAreaShape code.
   * @param planetModel The planet model.
   * @return The random generated GeoAreaShape.
   */
  virtual std::shared_ptr<GeoAreaShape>
  randomGeoAreaShape(int shapeType, std::shared_ptr<PlanetModel> planetModel);

  /**
   * Method that returns a random generated GeoAreaShape under given
   * constraints. Returns NULL if it cannot build the GeoAreaShape under the
   * given constraints.
   *
   * @param shapeType The GeoAreaShape code.
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoAreaShape.
   */
  virtual std::shared_ptr<GeoAreaShape>
  randomGeoAreaShape(int shapeType, std::shared_ptr<PlanetModel> planetModel,
                     std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated GeoShape.
   *
   * @param shapeType The shape code.
   * @param planetModel The planet model.
   * @return The random generated GeoShape.
   */
  virtual std::shared_ptr<GeoShape>
  randomGeoShape(int shapeType, std::shared_ptr<PlanetModel> planetModel);

  /**
   * Method that returns a random generated GeoShape under given constraints.
   * Returns NULL if it cannot build the GeoShape under the given constraints.
   *
   * @param shapeType The polygon code.
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoShape.
   */
  virtual std::shared_ptr<GeoShape>
  randomGeoShape(int shapeType, std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoPointShape under given
   * constraints. Returns NULL if it cannot build the GeoCircle under the given
   * constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPointShape.
   */
private:
  std::shared_ptr<GeoPointShape>
  point(std::shared_ptr<PlanetModel> planetModel,
        std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoCircle under given constraints.
   * Returns NULL if it cannot build the GeoCircle under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoCircle.
   */
  std::shared_ptr<GeoCircle> circle(std::shared_ptr<PlanetModel> planetModel,
                                    std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoCircle under given constraints.
   * Returns NULL if it cannot build the GeoCircle under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoCircle.
   */
  std::shared_ptr<GeoCircle>
  exactCircle(std::shared_ptr<PlanetModel> planetModel,
              std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoBBox under given constraints.
   * Returns NULL if it cannot build the GeoBBox under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoBBox.
   */
  std::shared_ptr<GeoBBox> rectangle(std::shared_ptr<PlanetModel> planetModel,
                                     std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated degenerate GeoPath under given
   * constraints. Returns NULL if it cannot build the degenerate GeoPath under
   * the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated degenerated GeoPath.
   */
  std::shared_ptr<GeoPath> line(std::shared_ptr<PlanetModel> planetModel,
                                std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoPath under given constraints.
   * Returns NULL if it cannot build the GeoPath under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPath.
   */
  std::shared_ptr<GeoPath> path(std::shared_ptr<PlanetModel> planetModel,
                                std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a GeoCompositeMembershipShape under
   * given constraints. Returns NULL if it cannot build the
   * GGeoCompositeMembershipShape under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoCompositeMembershipShape.
   */
  std::shared_ptr<GeoCompositeAreaShape>
  collection(std::shared_ptr<PlanetModel> planetModel,
             std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a convex GeoPolygon under given
   * constraints. Returns NULL if it cannot build the GePolygon under the given
   * constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  convexPolygon(std::shared_ptr<PlanetModel> planetModel,
                std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a convex GeoPolygon with holes under
   * given constraints. Returns NULL if it cannot build the GeoPolygon with
   * holes under the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  convexPolygonWithHoles(std::shared_ptr<PlanetModel> planetModel,
                         std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random deque if concave GeoPolygons under given
   * constraints. Method use to generate convex holes. Note that constraints for
   * points and holes are different,
   *
   * @param planetModel The planet model.
   * @param polygon The polygon where the holes are within.
   * @param holeConstraints The given constraints that a hole must comply.
   * @param pointConstraints The given constraints that a point must comply.
   * @return The random generated GeoPolygon.
   */
  std::deque<std::shared_ptr<GeoPolygon>>
  concavePolygonHoles(std::shared_ptr<PlanetModel> planetModel,
                      std::shared_ptr<GeoPolygon> polygon,
                      std::shared_ptr<Constraints> holeConstraints,
                      std::shared_ptr<Constraints> pointConstraints);

  /**
   * Method that returns a random generated a concave GeoPolygon under given
   * constraints. Returns NULL if it cannot build the concave GeoPolygon under
   * the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  concavePolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a concave GeoPolygon with holes
   * under given constraints. Returns NULL if it cannot build the GeoPolygon
   * under the given constraints. Note that the final GeoPolygon is convex as
   * the hole wraps the convex GeoPolygon.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  concavePolygonWithHoles(std::shared_ptr<PlanetModel> planetModel,
                          std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated complex GeoPolygon under given
   * constraints. Returns NULL if it cannot build the complex GeoPolygon under
   * the given constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  complexPolygon(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a concave square GeoPolygon under
   * given constraints. Returns NULL if it cannot build the concave GeoPolygon
   * under the given constraints. This shape is an utility to build constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  simpleConvexPolygon(std::shared_ptr<PlanetModel> planetModel,
                      std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random generated a convex square GeoPolygon under
   * given constraints. Returns NULL if it cannot build the convex GeoPolygon
   * under the given constraints. This shape is an utility to build constraints.
   *
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated GeoPolygon.
   */
  std::shared_ptr<GeoPolygon>
  concaveSimplePolygon(std::shared_ptr<PlanetModel> planetModel,
                       std::shared_ptr<Constraints> constraints);

  /**
   * Method that returns a random deque of generated GeoPoints under given
   * constraints. The number of points returned might be lower than the
   * requested.
   *
   * @param count The number of points
   * @param planetModel The planet model.
   * @param constraints The given constraints.
   * @return The random generated List of GeoPoints.
   */
  std::deque<std::shared_ptr<GeoPoint>>
  points(int count, std::shared_ptr<PlanetModel> planetModel,
         std::shared_ptr<Constraints> constraints);

  /**
   * Check if a GeoPolygon is pure concave. Note that our definition for
   * concavity is that the polygon contains antipodal points.
   *
   * @param planetModel The planet model.
   * @param shape The polygon to check.
   * @return True if the polygon contains antipodal points.
   */
  bool isConcave(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<GeoPolygon> shape);

  /**
   * Check if a GeoPolygon is pure convex. Note that our definition for
   * convexity is that the polygon does not contain antipodal points.
   *
   * @param planetModel The planet model.
   * @param shape The polygon to check.
   * @return True if the polygon dies not contains antipodal points.
   */
  bool isConvex(std::shared_ptr<PlanetModel> planetModel,
                std::shared_ptr<GeoPolygon> shape);

  /**
   * Generates a random number between 0 and PI.
   *
   * @return the cutoff angle.
   */
  double randomCutoffAngle();

  /**
   * Method that orders a lit of points anti-clock-wise to prevent crossing
   * edges.
   *
   * @param points The points to order.
   * @return The deque of ordered points anti-clockwise.
   */
protected:
  virtual std::deque<std::shared_ptr<GeoPoint>>
  orderPoints(std::deque<std::shared_ptr<GeoPoint>> &points);

  /**
   * Class that holds the constraints that are given to
   * build shapes. It consists in a deque of GeoAreaShapes
   * and relationships the new shape needs to satisfy.
   */
public:
  class Constraints
      : public std::unordered_map<std::shared_ptr<GeoAreaShape>, int>
  {
    GET_CLASS_NAME(Constraints)
  private:
    std::shared_ptr<RandomGeo3dShapeGenerator> outerInstance;

  public:
    Constraints(std::shared_ptr<RandomGeo3dShapeGenerator> outerInstance);

    /**
     * Check if the shape is valid under the constraints.
     *
     * @param shape The shape to check
     * @return true if the shape satisfy the constraints, else false.
     */
    virtual bool valid(std::shared_ptr<GeoShape> shape);

    /**
     * Check if a point is Within the constraints.
     *
     * @param point The point to check
     * @return true if the point satisfy the constraints, else false.
     */
    virtual bool isWithin(std::shared_ptr<GeoPoint> point);

    /**
     * Check if a point is Within one constraint given by a shape and a
     * relationship.
     *
     * @param point The point to check
     * @param shape The shape of the constraint
     * @param relationship The relationship of the constraint.
     * @return true if the point satisfy the constraint, else false.
     */
  private:
    bool validPoint(std::shared_ptr<GeoPoint> point,
                    std::shared_ptr<GeoShape> shape, int relationship);

    /**
     * Collect the CONTAINS constraints in the object
     *
     * @return the CONTAINS constraints.
     */
  public:
    virtual std::shared_ptr<Constraints> getContains();

    /**
     * Collect the WITHIN constraints in the object
     *
     * @return the WITHIN constraints.
     */
    virtual std::shared_ptr<Constraints> getWithin();

    /**
     * Collect the OVERLAPS constraints in the object
     *
     * @return the OVERLAPS constraints.
     */
    virtual std::shared_ptr<Constraints> getOverlaps();

    /**
     * Collect the DISJOINT constraints in the object
     *
     * @return the DISJOINT constraints.
     */
    virtual std::shared_ptr<Constraints> getDisjoint();

  private:
    std::shared_ptr<Constraints> getConstraintsOfType(int type);

  protected:
    std::shared_ptr<Constraints> shared_from_this()
    {
      return std::static_pointer_cast<Constraints>(
          java.util.HashMap<GeoAreaShape, int>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<RandomGeo3dShapeGenerator> shared_from_this()
  {
    return std::static_pointer_cast<RandomGeo3dShapeGenerator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
