#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoCircle;
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

using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;

/**
 * Specialization of a {@link Geo3dShape} which represents a {@link Circle}.
 *
 * @lucene.experimental
 */
class Geo3dCircleShape : public Geo3dShape<std::shared_ptr<GeoCircle>>,
                         public Circle
{
  GET_CLASS_NAME(Geo3dCircleShape)

public:
  Geo3dCircleShape(std::shared_ptr<GeoCircle> shape,
                   std::shared_ptr<SpatialContext> spatialcontext);

  void reset(double x, double y, double radiusDEG) override;

  double getRadius() override;

  std::shared_ptr<Point> getCenter() override;

protected:
  std::shared_ptr<Geo3dCircleShape> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dCircleShape>(
        Geo3dShape<
            org.apache.lucene.spatial3d.geom.GeoCircle>::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::spatial4j
