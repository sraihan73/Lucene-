#pragma once
#include "stringhelper.h"
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
 * Implemented by Geo3D shapes that can calculate if a point is within it or
 * not.
 *
 * @lucene.experimental
 */
class Membership
{
  GET_CLASS_NAME(Membership)

  /**
   * Check if a point is within this shape.
   *
   * @param point is the point to check.
   * @return true if the point is within this shape
   */
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      public default bool isWithin(final Vector point)
  //  {
  //    return isWithin(point.x, point.y, point.z);
  //  }

  /**
   * Check if a point is within this shape.
   *
   * @param x is x coordinate of point to check.
   * @param y is y coordinate of point to check.
   * @param z is z coordinate of point to check.
   * @return true if the point is within this shape
   */
public:
  virtual bool isWithin(double const x, double const y, double const z) = 0;
};

} // namespace org::apache::lucene::spatial3d::geom
