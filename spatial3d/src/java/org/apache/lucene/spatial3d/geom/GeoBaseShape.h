#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"

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
 * Base extended shape object.
 *
 * @lucene.internal
 */
class GeoBaseShape : public BasePlanetObject, public GeoShape
{
  GET_CLASS_NAME(GeoBaseShape)

  /** Constructor.
   *@param planetModel is the planet model to use.
   */
public:
  GeoBaseShape(std::shared_ptr<PlanetModel> planetModel);

  void getBounds(std::shared_ptr<Bounds> bounds) override;

protected:
  std::shared_ptr<GeoBaseShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseShape>(
        BasePlanetObject::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
