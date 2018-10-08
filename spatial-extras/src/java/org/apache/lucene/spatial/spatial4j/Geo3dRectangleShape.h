#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoBBox;
}

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

using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

/**
 * Specialization of a {@link Geo3dShape} which represents a {@link Rectangle}.
 *
 * @lucene.experimental
 */
class Geo3dRectangleShape : public Geo3dShape<std::shared_ptr<GeoBBox>>,
                            public Rectangle
{
  GET_CLASS_NAME(Geo3dRectangleShape)

private:
  double minX = 0;
  double maxX = 0;
  double minY = 0;
  double maxY = 0;

public:
  Geo3dRectangleShape(std::shared_ptr<GeoBBox> shape,
                      std::shared_ptr<SpatialContext> spatialcontext,
                      double minX, double maxX, double minY, double maxY);

  Geo3dRectangleShape(std::shared_ptr<GeoBBox> shape,
                      std::shared_ptr<SpatialContext> spatialcontext);

  /**
   * Set the bounds from the wrapped GeoBBox.
   */
private:
  void setBoundsFromshape();

public:
  std::shared_ptr<Point> getCenter() override;

  void reset(double minX, double maxX, double minY, double maxY) override;

  std::shared_ptr<Rectangle> getBoundingBox() override;

  double getWidth() override;

  double getHeight() override;

  double getMinX() override;

  double getMinY() override;

  double getMaxX() override;

  double getMaxY() override;

  bool getCrossesDateLine() override;

  std::shared_ptr<SpatialRelation> relateYRange(double minY,
                                                double maxY) override;

  std::shared_ptr<SpatialRelation> relateXRange(double minX,
                                                double maxX) override;

  std::shared_ptr<Shape>
  getBuffered(double distance,
              std::shared_ptr<SpatialContext> spatialContext) override;

protected:
  std::shared_ptr<Geo3dRectangleShape> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dRectangleShape>(
        Geo3dShape<
            org.apache.lucene.spatial3d.geom.GeoBBox>::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::spatial4j
