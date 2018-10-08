#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
 * GeoCircles have all the characteristics of GeoBaseDistanceShapes, plus
 * GeoSizeable.
 *
 * @lucene.internal
 */
class GeoBaseCircle : public GeoBaseDistanceShape, public GeoCircle
{
  GET_CLASS_NAME(GeoBaseCircle)

  /** Constructor.
   *@param planetModel is the planet model to use.
   */
public:
  GeoBaseCircle(std::shared_ptr<PlanetModel> planetModel);

protected:
  std::shared_ptr<GeoBaseCircle> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseCircle>(
        GeoBaseDistanceShape::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
