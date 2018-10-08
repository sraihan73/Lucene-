#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoAreaShape;
}

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
 * GeoCompositeAreaShape is a set of GeoAreaShape's, treated as a unit.
 *
 * @lucene.experimental
 */
class GeoCompositeAreaShape
    : public GeoBaseCompositeAreaShape<std::shared_ptr<GeoAreaShape>>
{
  GET_CLASS_NAME(GeoCompositeAreaShape)

  /**
   * Constructor.
   */
public:
  GeoCompositeAreaShape(std::shared_ptr<PlanetModel> planetModel);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoCompositeAreaShape(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

  virtual bool equals(std::any o);

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoCompositeAreaShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoCompositeAreaShape>(
        GeoBaseCompositeAreaShape<GeoAreaShape>::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
