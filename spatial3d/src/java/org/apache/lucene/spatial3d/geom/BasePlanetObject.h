#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>

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
namespace org::apache::lucene::spatial3d::geom
{

/**
 * All Geo3D shapes can derive from this base class, which furnishes
 * some common code
 *
 * @lucene.internal
 */
class BasePlanetObject : public std::enable_shared_from_this<BasePlanetObject>,
                         public PlanetObject
{
  GET_CLASS_NAME(BasePlanetObject)

  /** This is the planet model embedded in all objects derived from this
   * class. */
protected:
  const std::shared_ptr<PlanetModel> planetModel;

  /** Constructor creating class instance given a planet model.
   * @param planetModel is the planet model.
   */
public:
  BasePlanetObject(std::shared_ptr<PlanetModel> planetModel);

  std::shared_ptr<PlanetModel> getPlanetModel() override;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  virtual int hashCode();

  bool equals(std::any const o) override;
};

} // namespace org::apache::lucene::spatial3d::geom
