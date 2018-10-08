#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygon.h"
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
 * Class which constructs a GeoPolygon representing S2 google pixel.
 *
 * @lucene.experimental
 */
class GeoS2ShapeFactory : public std::enable_shared_from_this<GeoS2ShapeFactory>
{
  GET_CLASS_NAME(GeoS2ShapeFactory)

private:
  GeoS2ShapeFactory();

  /**
   * Creates a convex polygon with 4 planes by providing 4 points in CCW.
   * This is a very fast shape and there are no checks that the points currently
   * define a convex shape.
   *
   * @param planetModel The planet model
   * @param point1 the first point.
   * @param point2 the second point.
   * @param point3 the third point.
   * @param point4 the four point.
   * @return the generated shape.
   */
public:
  static std::shared_ptr<GeoPolygon> makeGeoS2Shape(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<GeoPoint> point1, std::shared_ptr<GeoPoint> point2,
      std::shared_ptr<GeoPoint> point3, std::shared_ptr<GeoPoint> point4);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
