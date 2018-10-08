#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoBBox.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/LatLonBounds.h"

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
 * Factory for {@link GeoBBox}.
 *
 * @lucene.experimental
 */
class GeoBBoxFactory : public std::enable_shared_from_this<GeoBBoxFactory>
{
  GET_CLASS_NAME(GeoBBoxFactory)
private:
  GeoBBoxFactory();

  /**
   * Create a geobbox of the right kind given the specified bounds.
   *
   * @param planetModel is the planet model
   * @param topLat    is the top latitude
   * @param bottomLat is the bottom latitude
   * @param leftLon   is the left longitude
   * @param rightLon  is the right longitude
   * @return a GeoBBox corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoBBox>
  makeGeoBBox(std::shared_ptr<PlanetModel> planetModel, double topLat,
              double bottomLat, double leftLon, double rightLon);

  /**
   * Create a geobbox of the right kind given the specified {@link
   * LatLonBounds}.
   *
   * @param planetModel is the planet model
   * @param bounds    are the bounds
   * @return a GeoBBox corresponding to what was specified.
   */
  static std::shared_ptr<GeoBBox>
  makeGeoBBox(std::shared_ptr<PlanetModel> planetModel,
              std::shared_ptr<LatLonBounds> bounds);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
