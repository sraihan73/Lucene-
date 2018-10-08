#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"

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
namespace org::apache::lucene::spatial::query
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Principally holds the query {@link Shape} and the {@link SpatialOperation}.
 * It's used as an argument to some methods on {@link
 * org.apache.lucene.spatial.SpatialStrategy}.
 *
 * @lucene.experimental
 */
class SpatialArgs : public std::enable_shared_from_this<SpatialArgs>
{
  GET_CLASS_NAME(SpatialArgs)

public:
  static constexpr double DEFAULT_DISTERRPCT = 0.025;

private:
  std::shared_ptr<SpatialOperation> operation;
  std::shared_ptr<Shape> shape;
  std::optional<double> distErrPct;
  std::optional<double> distErr;

public:
  SpatialArgs(std::shared_ptr<SpatialOperation> operation,
              std::shared_ptr<Shape> shape);

  /**
   * Computes the distance given a shape and the {@code distErrPct}.  The
   * algorithm is the fraction of the distance from the center of the query
   * shape to its closest bounding box corner.
   *
   * @param shape Mandatory.
   * @param distErrPct 0 to 0.5
   * @param ctx Mandatory
   * @return A distance (in degrees).
   */
  static double calcDistanceFromErrPct(std::shared_ptr<Shape> shape,
                                       double distErrPct,
                                       std::shared_ptr<SpatialContext> ctx);

  /**
   * Gets the error distance that specifies how precise the query shape is. This
   * looks at {@link #getDistErr()}, {@link #getDistErrPct()}, and {@code
   * defaultDistErrPct}.
   * @param defaultDistErrPct 0 to 0.5
   * @return {@code >= 0}
   */
  virtual double resolveDistErr(std::shared_ptr<SpatialContext> ctx,
                                double defaultDistErrPct);

  /** Check if the arguments make sense -- throw an exception if not */
  virtual void validate() ;

  virtual std::wstring toString();

  //------------------------------------------------
  // Getters & Setters
  //------------------------------------------------

  virtual std::shared_ptr<SpatialOperation> getOperation();

  virtual void setOperation(std::shared_ptr<SpatialOperation> operation);

  virtual std::shared_ptr<Shape> getShape();

  virtual void setShape(std::shared_ptr<Shape> shape);

  /**
   * A measure of acceptable error of the shape as a fraction.  This effectively
   * inflates the size of the shape but should not shrink it.
   *
   * @return 0 to 0.5
   * @see #calcDistanceFromErrPct(org.locationtech.spatial4j.shape.Shape,
   * double, org.locationtech.spatial4j.context.SpatialContext)
   */
  virtual std::optional<double> getDistErrPct();

  virtual void setDistErrPct(std::optional<double> &distErrPct);

  /**
   * The acceptable error of the shape.  This effectively inflates the
   * size of the shape but should not shrink it.
   *
   * @return {@code >= 0}
   */
  virtual std::optional<double> getDistErr();

  virtual void setDistErr(std::optional<double> &distErr);
};

} // #include  "core/src/java/org/apache/lucene/spatial/query/
