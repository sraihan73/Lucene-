#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPointShape.h"

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

namespace org::apache::lucene::spatial::spatial4j
{

using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Specialization of a {@link Geo3dShape} which represents a {@link Point}.
 *
 * @lucene.experimental
 */
class Geo3dPointShape : public Geo3dShape<std::shared_ptr<GeoPointShape>>,
                        public Point
{
  GET_CLASS_NAME(Geo3dPointShape)

public:
  Geo3dPointShape(std::shared_ptr<GeoPointShape> shape,
                  std::shared_ptr<SpatialContext> spatialcontext);

  void reset(double x, double y) override;

  double getX() override;

  double getY() override;

  std::shared_ptr<Rectangle> getBoundingBox() override;

  std::shared_ptr<Shape>
  getBuffered(double distance,
              std::shared_ptr<SpatialContext> spatialContext) override;

  bool hasArea() override;

protected:
  std::shared_ptr<Geo3dPointShape> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dPointShape>(
        Geo3dShape<org.apache.lucene.spatial3d.geom
                       .GeoPointShape>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
