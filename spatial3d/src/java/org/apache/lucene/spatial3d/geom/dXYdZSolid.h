#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"

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
 * 3D rectangle, bounded on six sides by X,Y,Z limits, degenerate in X and Z.
 * This figure, in fact, represents either zero, one, or two points, so the
 * actual data stored is minimal.
 *
 * @lucene.internal
 */
class dXYdZSolid : public BaseXYZSolid
{
  GET_CLASS_NAME(dXYdZSolid)

  /** X */
protected:
  const double X;
  /** Min-Y */
  const double minY;
  /** Max-Y */
  const double maxY;
  /** Z */
  const double Z;

  /** The points in this figure on the planet surface; also doubles for edge
   * points */
  std::deque<std::shared_ptr<GeoPoint>> const surfacePoints;

  /**
   * Sole constructor
   *
   *@param planetModel is the planet model.
   *@param X is the X value.
   *@param minY is the minimum Y value.
   *@param maxY is the maximum Y value.
   *@param Z is the Z value.
   */
public:
  dXYdZSolid(std::shared_ptr<PlanetModel> planetModel, double const X,
             double const minY, double const maxY, double const Z);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  dXYdZSolid(std::shared_ptr<PlanetModel> planetModel,
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
  std::shared_ptr<dXYdZSolid> shared_from_this()
  {
    return std::static_pointer_cast<dXYdZSolid>(
        BaseXYZSolid::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
