#pragma once
#include "stringhelper.h"
#include <memory>

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

namespace org::apache::lucene::spatial::prefix::tree
{

using com::google::common::geometry::S2CellId;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeFactory;

/**
 * Shape factory for Spatial contexts that support S2 geometry. It is an
 * extension of Spatial4j {@link ShapeFactory}.
 *
 * @lucene.experimental
 */
class S2ShapeFactory : public ShapeFactory
{
  GET_CLASS_NAME(S2ShapeFactory)

  /**
   * Factory method for S2 cell shapes.
   *
   * @param cellId The S2 cell id
   * @return the shape representing the cell.
   */
public:
  virtual std::shared_ptr<Shape>
  getS2CellShape(std::shared_ptr<S2CellId> cellId) = 0;
};

} // namespace org::apache::lucene::spatial::prefix::tree
