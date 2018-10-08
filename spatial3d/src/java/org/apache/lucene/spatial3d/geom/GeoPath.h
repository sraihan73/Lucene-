#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
 * Interface describing a path.
 *
 * @lucene.experimental
 */
class GeoPath : public GeoDistanceShape
{
  GET_CLASS_NAME(GeoPath)

  // The following methods compute distances along the path from the shape to a
  // point that doesn't need to be inside the shape.  The perpendicular distance
  // from the path itself to the point is not included in the calculation.

  /**
   * Compute the nearest path distance to the GeoPoint.
   * The path distance will not include the distance from the path itself to the
   * point, but just the distance along the path to the nearest point on the
   * path.
   *
   * @param distanceStyle is the distance style.
   * @param point is the point to compute the distance to.
   * @return the distance to the nearest path point.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double computeNearestDistance(final DistanceStyle
  //      distanceStyle, final GeoPoint point)
  //  {
  //    return computeNearestDistance(distanceStyle, point.x, point.y, point.z);
  //  }

  /**
   * Compute the nearest path distance to the GeoPoint.
   * The path distance will not include the distance from the path itself to the
   * point, but just the distance along the path to the nearest point on the
   * path.
   *
   * @param x is the point's unit x coordinate (using U.S. convention).
   * @param y is the point's unit y coordinate (using U.S. convention).
   * @param z is the point's unit z coordinate (using U.S. convention).
   * @return the distance to the nearest path point.
   */
public:
  virtual double
  computeNearestDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y, double const z) = 0;

  // The following methods compute the best distance from the path center to the
  // point.

  /**
   * Compute the shortest distance from the path center to the GeoPoint.
   * The distance is meant to allow comparisons between different
   * paths to find the one that goes closest to a point.
   *
   * @param distanceStyle is the distance style.
   * @param point is the point to compute the distance to.
   * @return the shortest distance from the path center to the point.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double computePathCenterDistance(final DistanceStyle
  //      distanceStyle, final GeoPoint point)
  //  {
  //    return computePathCenterDistance(distanceStyle, point.x, point.y,
  //    point.z);
  //  }

  /**
   * Compute the shortest distance from the path center to the GeoPoint.
   * The distance is meant to allow comparisons between different
   * paths to find the one that goes closest to a point.
   *
   * @param distanceStyle is the distance style.
   * @param x is the point's unit x coordinate (using U.S. convention).
   * @param y is the point's unit y coordinate (using U.S. convention).
   * @param z is the point's unit z coordinate (using U.S. convention).
   * @return the shortest distance from the path center to the point.
   */
  virtual double
  computePathCenterDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                            double const x, double const y, double const z) = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
