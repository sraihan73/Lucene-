#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
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
namespace org::apache::lucene::spatial3d::geom
{

/**
 * 3D rectangle, bounded on six sides by X,Y,Z limits, degenerate in X and Y.
 * This figure, in fact, represents either zero, one, or two points, so the
 * actual data stored is minimal.
 *
 * @lucene.internal
 */
class dXdYZSolid : public BaseXYZSolid
{
  GET_CLASS_NAME(dXdYZSolid)

  /** X */
protected:
  const double X;
  /** Y */
  const double Y;
  /** Min-Z */
  const double minZ;
  /** Max-Z */
  const double maxZ;

  /** The points in this figure on the planet surface; also doubles for edge
   * points */
  std::deque<std::shared_ptr<GeoPoint>> const surfacePoints;

  /**
   * Sole constructor
   *
   *@param planetModel is the planet model.
   *@param X is the X value.
   *@param Y is the Y value.
   *@param minZ is the minimum Z value.
   *@param maxZ is the maximum Z value.
   */
public:
  dXdYZSolid(std::shared_ptr<PlanetModel> planetModel, double const X,
             double const Y, double const minZ, double const maxZ);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  dXdYZSolid(std::shared_ptr<PlanetModel> planetModel,
             std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

protected:
  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

public:
  bool isWithin(double const x, double const y, double const z) override;

  int getRelationship(std::shared_ptr<GeoShape> path) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<dXdYZSolid> shared_from_this()
  {
    return std::static_pointer_cast<dXdYZSolid>(
        BaseXYZSolid::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
