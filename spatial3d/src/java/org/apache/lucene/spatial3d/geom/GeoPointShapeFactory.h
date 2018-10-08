#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPointShape.h"

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
 *  Class which constructs a GeoPointShape.
 */
class GeoPointShapeFactory
    : public std::enable_shared_from_this<GeoPointShapeFactory>
{
  GET_CLASS_NAME(GeoPointShapeFactory)

private:
  GeoPointShapeFactory();

  /**
   * Create a GeoPointShape with the provided information.
   * @param planetModel the planet model
   * @param lat the latitude
   * @param lon the longitude
   * @return a GeoPointShape corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoPointShape>
  makeGeoPointShape(std::shared_ptr<PlanetModel> planetModel, double const lat,
                    double const lon);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
