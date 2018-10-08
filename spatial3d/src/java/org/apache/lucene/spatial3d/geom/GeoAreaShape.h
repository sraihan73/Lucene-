#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
 * Shape that implements GeoArea. This type of shapes are able to resolve the
 * spatial relationship of other shapes with itself.
 *
 * @lucene.experimental
 */

class GeoAreaShape : public GeoMembershipShape, public GeoArea
{
  GET_CLASS_NAME(GeoAreaShape)

  /**
   * Assess whether a shape intersects with any of the edges of this shape.
   * Note well that this method must return false if the shape contains or is
   * disjoint with the given shape.  It is permissible to return true if the
   * shape is within the specified shape, if it is difficult to compute
   * intersection with edges.
   *
   * @param geoShape is the shape to assess for intersection with this shape's
   * edges.
   *
   * @return true if there's such an intersection, false if not.
   */
public:
  virtual bool intersects(std::shared_ptr<GeoShape> geoShape) = 0;
};

} // namespace org::apache::lucene::spatial3d::geom
