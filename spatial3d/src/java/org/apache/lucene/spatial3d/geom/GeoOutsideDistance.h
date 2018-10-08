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
 * Implemented by Geo3D shapes that can compute the distance from a point to the
 * closest outside edge.
 *
 * @lucene.experimental
 */
class GeoOutsideDistance : public Membership
{
  GET_CLASS_NAME(GeoOutsideDistance)

  // The following methods compute distances from the shape to a point
  // expected to be OUTSIDE the shape.  Typically a value of 0.0
  // is returned for points that happen to be within the shape.

  /**
   * Compute this shape's distance to the GeoPoint.
   * A return value of 0.0 should be returned for
   * points inside of the shape.
   * @param distanceStyle is the distance style.
   * @param point is the point to compute the distance to.
   * @return the distance.
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default double computeOutsideDistance(final DistanceStyle
  //      distanceStyle, final GeoPoint point)
  //  {
  //    return computeOutsideDistance(distanceStyle, point.x, point.y, point.z);
  //  }

  /**
   * Compute this shape's distance to the GeoPoint.
   * A return value of 0.0 should be returned for
   * points inside of the shape.
   * @param distanceStyle is the distance style.
   * @param x is the point's unit x coordinate (using U.S. convention).
   * @param y is the point's unit y coordinate (using U.S. convention).
   * @param z is the point's unit z coordinate (using U.S. convention).
   * @return the distance.
   */
public:
  virtual double
  computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y, double const z) = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
