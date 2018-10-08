#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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

namespace org::apache::lucene::spatial::spatial4j
{

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;

/**
 * Geo3d implementation of {@link SpatialContextFactory}
 *
 * @lucene.experimental
 */
class Geo3dSpatialContextFactory : public SpatialContextFactory
{
  GET_CLASS_NAME(Geo3dSpatialContextFactory)

  /**
   * The default planet model
   */
private:
  static const std::shared_ptr<PlanetModel> DEFAULT_PLANET_MODEL;

  /**
   * The planet model
   */
public:
  std::shared_ptr<PlanetModel> planetModel;

  /**
   * Empty Constructor.
   */
  Geo3dSpatialContextFactory();

  std::shared_ptr<SpatialContext> newSpatialContext() override;

protected:
  void init(std::unordered_map<std::wstring, std::wstring> &args,
            std::shared_ptr<ClassLoader> classLoader) override;

  virtual void
  initPlanetModel(std::unordered_map<std::wstring, std::wstring> &args);

  void initCalculator() override;

protected:
  std::shared_ptr<Geo3dSpatialContextFactory> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dSpatialContextFactory>(
        org.locationtech.spatial4j.context
            .SpatialContextFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
