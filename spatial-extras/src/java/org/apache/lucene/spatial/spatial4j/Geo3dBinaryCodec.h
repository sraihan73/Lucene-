#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
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

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::io::BinaryCodec;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;

/**
 * Geo3d implementation of {@link BinaryCodec}
 *
 * @lucene.experimental
 */
class Geo3dBinaryCodec : public BinaryCodec
{
  GET_CLASS_NAME(Geo3dBinaryCodec)

private:
  std::shared_ptr<PlanetModel> planetModel;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public
  // Geo3dBinaryCodec(org.locationtech.spatial4j.context.SpatialContext ctx,
  // org.locationtech.spatial4j.context.SpatialContextFactory factory)
  Geo3dBinaryCodec(std::shared_ptr<SpatialContext> ctx,
                   std::shared_ptr<SpatialContextFactory> factory);

  std::shared_ptr<Shape>
  readShape(std::shared_ptr<DataInput> dataInput)  override;

  void writeShape(std::shared_ptr<DataOutput> dataOutput,
                  std::shared_ptr<Shape> s)  override;

  std::shared_ptr<Point>
  readPoint(std::shared_ptr<DataInput> dataInput)  override;

  void writePoint(std::shared_ptr<DataOutput> dataOutput,
                  std::shared_ptr<Point> pt)  override;

  std::shared_ptr<Rectangle>
  readRect(std::shared_ptr<DataInput> dataInput)  override;

  void writeRect(std::shared_ptr<DataOutput> dataOutput,
                 std::shared_ptr<Rectangle> r)  override;

  std::shared_ptr<Circle>
  readCircle(std::shared_ptr<DataInput> dataInput)  override;

  void writeCircle(std::shared_ptr<DataOutput> dataOutput,
                   std::shared_ptr<Circle> c)  override;

  std::shared_ptr<ShapeCollection> readCollection(
      std::shared_ptr<DataInput> dataInput)  override;

  void writeCollection(
      std::shared_ptr<DataOutput> dataOutput,
      std::shared_ptr<ShapeCollection> col)  override;

protected:
  std::shared_ptr<Geo3dBinaryCodec> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dBinaryCodec>(
        org.locationtech.spatial4j.io.BinaryCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::spatial4j
