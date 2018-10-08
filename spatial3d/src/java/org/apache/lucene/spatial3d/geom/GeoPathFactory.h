#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPath.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
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
 * Class which constructs a GeoPath representing an arbitrary path.
 *
 * @lucene.experimental
 */
class GeoPathFactory : public std::enable_shared_from_this<GeoPathFactory>
{
  GET_CLASS_NAME(GeoPathFactory)
private:
  GeoPathFactory();

  /**
   * Create a GeoPath of the right kind given the specified information.
   * @param planetModel is the planet model.
   * @param maxCutoffAngle is the width of the path, measured as an angle.
   * @param pathPoints are the points in the path.
   * @return a GeoPath corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoPath>
  makeGeoPath(std::shared_ptr<PlanetModel> planetModel,
              double const maxCutoffAngle,
              std::deque<std::shared_ptr<GeoPoint>> &pathPoints);

  /** Filter duplicate points.
   * @param pathPoints with the arras of points.
   * @return the filtered array.
   */
private:
  static std::deque<std::shared_ptr<GeoPoint>>
  filterPoints(std::deque<std::shared_ptr<GeoPoint>> &pathPoints);
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
