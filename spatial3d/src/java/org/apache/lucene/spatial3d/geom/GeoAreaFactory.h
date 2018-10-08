#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoArea.h"

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
 * Factory for {@link GeoArea}.
 *
 * @lucene.experimental
 */
class GeoAreaFactory : public std::enable_shared_from_this<GeoAreaFactory>
{
  GET_CLASS_NAME(GeoAreaFactory)
private:
  GeoAreaFactory();

  /**
   * Create a GeoArea of the right kind given the specified bounds.
   * @param planetModel is the planet model
   * @param topLat    is the top latitude
   * @param bottomLat is the bottom latitude
   * @param leftLon   is the left longitude
   * @param rightLon  is the right longitude
   * @return a GeoArea corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoArea>
  makeGeoArea(std::shared_ptr<PlanetModel> planetModel, double const topLat,
              double const bottomLat, double const leftLon,
              double const rightLon);

  /**
   * Create a GeoArea of the right kind given (x,y,z) bounds.
   * @param planetModel is the planet model
   * @param minX is the min X boundary
   * @param maxX is the max X boundary
   * @param minY is the min Y boundary
   * @param maxY is the max Y boundary
   * @param minZ is the min Z boundary
   * @param maxZ is the max Z boundary
   */
  static std::shared_ptr<GeoArea>
  makeGeoArea(std::shared_ptr<PlanetModel> planetModel, double const minX,
              double const maxX, double const minY, double const maxY,
              double const minZ, double const maxZ);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
