#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/XYZSolid.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/XYZBounds.h"

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
 * Factory for {@link XYZSolid}.
 *
 * @lucene.experimental
 */
class XYZSolidFactory : public std::enable_shared_from_this<XYZSolidFactory>
{
  GET_CLASS_NAME(XYZSolidFactory)
private:
  XYZSolidFactory();

  /**
   * Create a XYZSolid of the right kind given (x,y,z) bounds.
   * @param planetModel is the planet model
   * @param minX is the min X boundary
   * @param maxX is the max X boundary
   * @param minY is the min Y boundary
   * @param maxY is the max Y boundary
   * @param minZ is the min Z boundary
   * @param maxZ is the max Z boundary
   */
public:
  static std::shared_ptr<XYZSolid>
  makeXYZSolid(std::shared_ptr<PlanetModel> planetModel, double const minX,
               double const maxX, double const minY, double const maxY,
               double const minZ, double const maxZ);

  /**
   * Create a XYZSolid of the right kind given (x,y,z) bounds.
   * @param planetModel is the planet model
   * @param bounds is the XYZ bounds object.
   * @return the solid.
   */
  static std::shared_ptr<XYZSolid>
  makeXYZSolid(std::shared_ptr<PlanetModel> planetModel,
               std::shared_ptr<XYZBounds> bounds);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
