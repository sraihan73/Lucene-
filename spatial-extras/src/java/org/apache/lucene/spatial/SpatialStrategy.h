#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Field.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"

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
namespace org::apache::lucene::spatial
{

using Field = org::apache::lucene::document::Field;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

/**
 * The SpatialStrategy encapsulates an approach to indexing and searching based
 * on shapes.
 * <p>
 * Different implementations will support different features. A strategy should
 * document these common elements:
 * <ul>
 *   <li>Can it index more than one shape per field?</li>
 *   <li>What types of shapes can be indexed?</li>
 *   <li>What types of query shapes can be used?</li>
 *   <li>What types of query operations are supported?
 *   This might vary per shape.</li>
 *   <li>Does it use some type of cache?  When?
 * </ul>
 * If a strategy only supports certain shapes at index or query time, then in
 * general it will throw an exception if given an incompatible one.  It will not
 * be coerced into compatibility.
 * <p>
 * Note that a SpatialStrategy is not involved with the Lucene stored field
 * values of shapes, which is immaterial to indexing and search.
 * <p>
 * Thread-safe.
 * <p>
 * This API is marked as experimental, however it is quite stable.
 *
 * @lucene.experimental
 */
class SpatialStrategy : public std::enable_shared_from_this<SpatialStrategy>
{
  GET_CLASS_NAME(SpatialStrategy)

protected:
  const std::shared_ptr<SpatialContext> ctx;

private:
  const std::wstring fieldName;

  /**
   * Constructs the spatial strategy with its mandatory arguments.
   */
public:
  SpatialStrategy(std::shared_ptr<SpatialContext> ctx,
                  const std::wstring &fieldName);

  virtual std::shared_ptr<SpatialContext> getSpatialContext();

  /**
   * The name of the field or the prefix of them if there are multiple
   * fields needed internally.
   * @return Not null.
   */
  virtual std::wstring getFieldName();

  /**
   * Returns the IndexableField(s) from the {@code shape} that are to be
   * added to the {@link org.apache.lucene.document.Document}.  These fields
   * are expected to be marked as indexed and not stored.
   * <p>
   * Note: If you want to <i>store</i> the shape as a string for retrieval in
   * search results, you could add it like this:
   * <pre>document.add(new StoredField(fieldName,ctx.toString(shape)));</pre>
   * The particular string representation used doesn't matter to the Strategy
   * since it doesn't use it.
   *
   * @return Not null nor will it have null elements.
   * @throws UnsupportedOperationException if given a shape incompatible with
   * the strategy
   */
  virtual std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) = 0;

  /**
   * See {@link #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point,
   * double)} called with a multiplier of 1.0 (i.e. units of degrees).
   */
  virtual std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint);

  /**
   * Make a ValueSource returning the distance between the center of the
   * indexed shape and {@code queryPoint}.  If there are multiple indexed shapes
   * then the closest one is chosen. The result is multiplied by {@code
   * multiplier}, which conveniently is used to get the desired units.
   */
  virtual std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) = 0;

  /**
   * Make a Query based principally on {@link
   * org.apache.lucene.spatial.query.SpatialOperation} and {@link Shape} from
   * the supplied {@code args}.  It should be constant scoring of 1.
   *
   * @throws UnsupportedOperationException If the strategy does not support the
   * shape in {@code args}
   * @throws org.apache.lucene.spatial.query.UnsupportedSpatialOperation If the
   * strategy does not support the {@link
   * org.apache.lucene.spatial.query.SpatialOperation} in {@code args}.
   */
  virtual std::shared_ptr<Query>
  makeQuery(std::shared_ptr<SpatialArgs> args) = 0;

  /**
   * Returns a ValueSource with values ranging from 1 to 0, depending inversely
   * on the distance from {@link
   * #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point,double)}.
   * The formula is {@code c/(d + c)} where 'd' is the distance and 'c' is
   * one tenth the distance to the farthest edge from the center. Thus the
   * scores will be 1 for indexed points at the center of the query shape and as
   * low as ~0.1 at its furthest edges.
   */
  std::shared_ptr<DoubleValuesSource>
  makeRecipDistanceValueSource(std::shared_ptr<Shape> queryShape);

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/spatial/
