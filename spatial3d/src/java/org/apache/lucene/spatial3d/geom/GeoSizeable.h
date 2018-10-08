#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
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
 * Some shapes can compute radii of a geocircle in which they are inscribed.
 *
 * @lucene.experimental
 */
class GeoSizeable
{
  GET_CLASS_NAME(GeoSizeable)
  /**
   * Returns the radius of a circle into which the GeoSizeable area can
   * be inscribed.
   *
   * @return the radius.
   */
public:
  virtual double getRadius() = 0;

  /**
   * Returns the center of a circle into which the area will be inscribed.
   *
   * @return the center.
   */
  virtual std::shared_ptr<GeoPoint> getCenter() = 0;
};

} // namespace org::apache::lucene::spatial3d::geom
