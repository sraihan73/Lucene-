#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::spatial::util
{

/**
 * Reusable geo-relation utility methods
 */
class GeoRelationUtils : public std::enable_shared_from_this<GeoRelationUtils>
{
  GET_CLASS_NAME(GeoRelationUtils)

  // No instance:
private:
  GeoRelationUtils();

  /**
   * Determine if a bbox (defined by minLat, maxLat, minLon, maxLon) contains
   * the provided point (defined by lat, lon) NOTE: this is a basic method that
   * does not handle dateline or pole crossing. Unwrapping must be done before
   * calling this method.
   */
public:
  static bool pointInRectPrecise(double const lat, double const lon,
                                 double const minLat, double const maxLat,
                                 double const minLon, double const maxLon);

  /////////////////////////
  // Rectangle relations
  /////////////////////////

  /**
   * Computes whether two rectangles are disjoint
   */
private:
  static bool rectDisjoint(double const aMinLat, double const aMaxLat,
                           double const aMinLon, double const aMaxLon,
                           double const bMinLat, double const bMaxLat,
                           double const bMinLon, double const bMaxLon);

  /**
   * Computes whether the first (a) rectangle is wholly within another (b)
   * rectangle (shared boundaries allowed)
   */
public:
  static bool rectWithin(double const aMinLat, double const aMaxLat,
                         double const aMinLon, double const aMaxLon,
                         double const bMinLat, double const bMaxLat,
                         double const bMinLon, double const bMaxLon);

  /**
   * Computes whether two rectangles cross
   */
  static bool rectCrosses(double const aMinLat, double const aMaxLat,
                          double const aMinLon, double const aMaxLon,
                          double const bMinLat, double const bMaxLat,
                          double const bMinLon, double const bMaxLon);

  /**
   * Computes whether a rectangle intersects another rectangle (crosses, within,
   * touching, etc)
   */
  static bool rectIntersects(double const aMinLat, double const aMaxLat,
                             double const aMinLon, double const aMaxLon,
                             double const bMinLat, double const bMaxLat,
                             double const bMinLon, double const bMaxLon);
};

} // #include  "core/src/java/org/apache/lucene/spatial/util/
