#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
 * Distance shapes have capabilities of both geohashing and distance
 * computation (which also includes point membership determination).
 *
 * @lucene.experimental
 */
class GeoDistanceShape : public GeoAreaShape, public GeoDistance
{
  GET_CLASS_NAME(GeoDistanceShape)

  /**
   * Compute a bound based on a provided distance measure.
   * This method takes an input distance and distance metric and provides bounds
   * on the shape if reduced to match that distance.  The method is allowed to
   * return bounds that are larger than the distance would indicate, but never
   * smaller.
   * @param bounds is the bounds object to update.
   * @param distanceStyle describes the type of distance metric provided.
   * @param distanceValue is the distance metric to use.  It is presumed that
   * the distance metric was produced with the same distance style as is
   * provided to this method.
   */
public:
  virtual void getDistanceBounds(std::shared_ptr<Bounds> bounds,
                                 std::shared_ptr<DistanceStyle> distanceStyle,
                                 double const distanceValue) = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
