#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

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
 * Distance computation styles, supporting various ways of computing
 * distance to shapes.
 *
 * @lucene.experimental
 */
class DistanceStyle
{
  GET_CLASS_NAME(DistanceStyle)

  // convenient access to built-in styles:

  /** Arc distance calculator */
public:
  static const std::shared_ptr<ArcDistance> ARC;
  /** Linear distance calculator */
  static const std::shared_ptr<LinearDistance> LINEAR;
  /** Linear distance squared calculator */
  static const std::shared_ptr<LinearSquaredDistance> LINEAR_SQUARED;
  /** Normal distance calculator */
  static const std::shared_ptr<NormalDistance> NORMAL;
  /** Normal distance squared calculator */
  static const std::shared_ptr<NormalSquaredDistance> NORMAL_SQUARED;

  /** Compute the distance from a point to another point.
   * @param point1 Starting point
   * @param point2 Final point
   * @return the distance
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double computeDistance(final GeoPoint point1, final
  //      GeoPoint point2)
  //  {
  //    return computeDistance(point1, point2.x, point2.y, point2.z);
  //  }

  /** Compute the distance from a point to another point.
   * @param point1 Starting point
   * @param x2 Final point x
   * @param y2 Final point y
   * @param z2 Final point z
   * @return the distance
   */
  virtual double computeDistance(std::shared_ptr<GeoPoint> point1,
                                 double const x2, double const y2,
                                 double const z2) = 0;

  /** Compute the distance from a plane to a point.
   * @param planetModel The planet model
   * @param plane The plane
   * @param point The point
   * @param bounds are the plane bounds
   * @return the distance
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double computeDistance(final PlanetModel planetModel,
  //      final Plane plane, final GeoPoint point, final Membership... bounds)
  //  {
  //    return computeDistance(planetModel, plane, point.x, point.y, point.z,
  //    bounds);
  //  }

  /** Compute the distance from a plane to a point.
   * @param planetModel The planet model
   * @param plane The plane
   * @param x The point x
   * @param y The point y
   * @param z The point z
   * @param bounds are the plane bounds
   * @return the distance
   */
  virtual double computeDistance(std::shared_ptr<PlanetModel> planetModel,
                                 std::shared_ptr<Plane> plane, double const x,
                                 double const y, double const z,
                                 std::deque<Membership> &bounds) = 0;

  /** Convert a distance to a form meant for aggregation.
   * This is meant to be used in conjunction with aggregateDistances() and
   * fromAggregationForm(). Distances should be converted to aggregation form
   * before aggregation is attempted, and they should be converted back from
   * aggregation form to yield a final result.
   * @param distance is an output of computeDistance().
   * @return the distance, converted to aggregation form.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double toAggregationForm(final double distance)
  //  {
  //    return distance;
  //  }

  /** Aggregate two distances together to produce a "sum".
   * This is usually just an addition operation, but in the case of squared
   * distances it is more complex. Distances should be converted to aggregation
   * form before aggregation is attempted, and they should be converted back
   * from aggregation form to yield a final result.
   * @param distance1 is the first aggregation form distance.
   * @param distance2 is the second aggregation form distance.
   * @return the combined aggregation form distance.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double aggregateDistances(final double distance1, final
  //      double distance2)
  //  {
  //    return distance1 + distance2;
  //  }

  /** Convert an aggregation form distance value back to an actual distance.
   * This is meant to be used in conjunctiion with toAggregationForm() and
   * aggregateDistances(). Distances should be converted to aggregation form
   * before aggregation is attempted, and they should be converted back from
   * aggregation form to yield a final result.
   * @param aggregateDistance is the aggregate form of the distance.
   * @return the combined distance.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double fromAggregationForm(final double
  //      aggregateDistance)
  //  {
  //    return aggregateDistance;
  //  }

  // The following methods are used to go from a distance value back to
  // something that can be used to construct a constrained shape.

  /**  Find a GeoPoint, at a specified distance from a starting point, within
   * the specified bounds.  The GeoPoint must be in the specified plane.
   * @param planetModel is the planet model.
   * @param distanceValue is the distance to set the new point at, measured from
   * point1 and on the way to point2.
   * @param startPoint is the starting point.
   * @param plane is the plane that the point must be in.
   * @param bounds are the constraints on where the point can be found.
   * @return zero, one, or two points at the proper distance from startPoint.
   */
  virtual std::deque<std::shared_ptr<GeoPoint>> findDistancePoints(
      std::shared_ptr<PlanetModel> planetModel, double const distanceValue,
      std::shared_ptr<GeoPoint> startPoint, std::shared_ptr<Plane> plane,
      std::deque<Membership> &bounds) = 0;

  /** Given a distance metric, find the minimum arc distance represented by that
   * distance metric.
   * @param planetModel is the planet model.
   * @param distanceValue is the distance metric.
   * @return the minimum arc distance that that distance value can represent
   * given the planet model.
   */
  virtual double
  findMinimumArcDistance(std::shared_ptr<PlanetModel> planetModel,
                         double const distanceValue) = 0;

  /** Given a distance metric, find the maximum arc distance represented by the
   * distance metric.
   * @param planetModel is the planet model.
   * @param distanceValue is the distance metric.
   * @return the maximum arc distance that that distance value can represent
   * given the planet model.
   */
  virtual double
  findMaximumArcDistance(std::shared_ptr<PlanetModel> planetModel,
                         double const distanceValue) = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
