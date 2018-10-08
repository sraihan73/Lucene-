#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/ShapeValuesSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/spatial/ShapeValues.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
namespace org::apache::lucene::spatial::util
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::shape::Point;

/**
 * The distance from a provided Point to a Point retrieved from an
 * ShapeValuesSource. The distance is calculated via a {@link
 * org.locationtech.spatial4j.distance.DistanceCalculator}.
 *
 * @lucene.experimental
 */
class DistanceToShapeValueSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DistanceToShapeValueSource)

private:
  const std::shared_ptr<ShapeValuesSource> shapeValueSource;
  const std::shared_ptr<Point> queryPoint;
  const double multiplier;
  const std::shared_ptr<DistanceCalculator> distCalc;

  // TODO if DoubleValues returns NaN; will things be ok?
  const double nullValue;

public:
  DistanceToShapeValueSource(
      std::shared_ptr<ShapeValuesSource> shapeValueSource,
      std::shared_ptr<Point> queryPoint, double multiplier,
      std::shared_ptr<SpatialContext> ctx);

  virtual std::wstring toString();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DistanceToShapeValueSource> outerInstance;

    std::shared_ptr<ShapeValues> shapeValues;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<DistanceToShapeValueSource> outerInstance,
        std::shared_ptr<ShapeValues> shapeValues);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

public:
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<DistanceToShapeValueSource> shared_from_this()
  {
    return std::static_pointer_cast<DistanceToShapeValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/util/
