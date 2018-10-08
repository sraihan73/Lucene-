#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix
{
class RecursivePrefixTreeStrategy;
}

namespace org::apache::lucene::spatial::serialized
{
class SerializedDVStrategy;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::spatial::query
{
class SpatialArgs;
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
namespace org::apache::lucene::spatial::composite
{

using Field = org::apache::lucene::document::Field;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A composite {@link SpatialStrategy} based on {@link
 * RecursivePrefixTreeStrategy} (RPT) and
 * {@link SerializedDVStrategy} (SDV).
 * RPT acts as an index to the precision available in SDV, and in some
 * circumstances can avoid geometry lookups based on where a cell is in relation
 * to the query shape.  Currently the only predicate optimized like this is
 * Intersects. All predicates are supported except for the BBox* ones, and
 * Disjoint.
 *
 * @lucene.experimental
 */
class CompositeSpatialStrategy : public SpatialStrategy
{
  GET_CLASS_NAME(CompositeSpatialStrategy)

  // TODO support others? (BBox)
private:
  const std::shared_ptr<RecursivePrefixTreeStrategy> indexStrategy;

  /** Has the geometry. */
  // TODO support others?
  const std::shared_ptr<SerializedDVStrategy> geometryStrategy;
  bool optimizePredicates = true;

public:
  CompositeSpatialStrategy(
      const std::wstring &fieldName,
      std::shared_ptr<RecursivePrefixTreeStrategy> indexStrategy,
      std::shared_ptr<SerializedDVStrategy>
          geometryStrategy); // field name; unused

  virtual std::shared_ptr<RecursivePrefixTreeStrategy> getIndexStrategy();

  virtual std::shared_ptr<SerializedDVStrategy> getGeometryStrategy();

  virtual bool isOptimizePredicates();

  /** Set to false to NOT use optimized search predicates that avoid checking
   * the geometry sometimes. Only useful for benchmarking. */
  virtual void setOptimizePredicates(bool optimizePredicates);

  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) override;

  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

protected:
  std::shared_ptr<CompositeSpatialStrategy> shared_from_this()
  {
    return std::static_pointer_cast<CompositeSpatialStrategy>(
        org.apache.lucene.spatial.SpatialStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::composite
