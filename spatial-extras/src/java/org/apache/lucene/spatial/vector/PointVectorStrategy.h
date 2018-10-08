#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
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
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class DoubleValues;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::spatial::deque
{

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Simple {@link SpatialStrategy} which represents Points in two numeric fields.
 * The Strategy's best feature is decent distance sort.
 *
 * <p>
 * <b>Characteristics:</b>
 * <br>
 * <ul>
 * <li>Only indexes points; just one per field value.</li>
 * <li>Can query by a rectangle or circle.</li>
 * <li>{@link
 * org.apache.lucene.spatial.query.SpatialOperation#Intersects} and {@link
 * SpatialOperation#IsWithin} is supported.</li>
 * <li>Requires DocValues for
 * {@link #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point)} and
 * for searching with a Circle.</li>
 * </ul>
 *
 * <p>
 * <b>Implementation:</b>
 * <p>
 * This is a simple Strategy.  Search works with a pair of range queries on two
 * {@link DoublePoint}s representing x &amp; y fields.  A Circle query does the
 * same bbox query but adds a ValueSource filter on
 * {@link #makeDistanceValueSource(org.locationtech.spatial4j.shape.Point)}.
 * <p>
 * One performance shortcoming with this strategy is that a scenario involving
 * both a search using a Circle and sort will result in calculations for the
 * spatial distance being done twice -- once for the filter and second for the
 * sort.
 *
 * @lucene.experimental
 */
class PointVectorStrategy : public SpatialStrategy
{
  GET_CLASS_NAME(PointVectorStrategy)

  // note: we use a FieldType to articulate the options we want on the field. We
  // don't use it as-is with a Field, we
  //  create more than one Field.

  /**
   * pointValues, docValues, and nothing else.
   */
public:
  static std::shared_ptr<FieldType> DEFAULT_FIELDTYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static PointVectorStrategy::StaticConstructor staticConstructor;

public:
  static const std::wstring SUFFIX_X;
  static const std::wstring SUFFIX_Y;

private:
  const std::wstring fieldNameX;
  const std::wstring fieldNameY;

  const int fieldsLen;
  const bool hasStored;
  const bool hasDocVals;
  const bool hasPointVals;

  /**
   * Create a new {@link PointVectorStrategy} instance that uses {@link
   * DoublePoint} and {@link DoublePoint#newRangeQuery}
   */
public:
  static std::shared_ptr<PointVectorStrategy>
  newInstance(std::shared_ptr<SpatialContext> ctx,
              const std::wstring &fieldNamePrefix);

  /**
   * Create a new instance configured with the provided FieldType options. See
   * {@link #DEFAULT_FIELDTYPE}. a field type is used to articulate the desired
   * options (namely pointValues, docValues, stored).  Legacy numerics is
   * configurable this way too.
   */
  PointVectorStrategy(std::shared_ptr<SpatialContext> ctx,
                      const std::wstring &fieldNamePrefix,
                      std::shared_ptr<FieldType> fieldType);

  virtual std::wstring getFieldNameX();

  virtual std::wstring getFieldNameY();

  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) override;

  /** @see #createIndexableFields(org.locationtech.spatial4j.shape.Shape) */
  virtual std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Point> point);

  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

  /**
   * Constructs a query to retrieve documents that fully contain the input
   * envelope.
   */
private:
  std::shared_ptr<Query> makeWithin(std::shared_ptr<Rectangle> bbox);

  /**
   * Returns a numeric range query based on FieldType
   * {@link DoublePoint#newRangeQuery} is used for indexes created using {@link
   * DoublePoint} fields
   */
  std::shared_ptr<Query> rangeQuery(const std::wstring &fieldName,
                                    std::optional<double> &min,
                                    std::optional<double> &max);

private:
  class DistanceRangeQuery : public Query
  {
    GET_CLASS_NAME(DistanceRangeQuery)

  public:
    const std::shared_ptr<Query> inner;
    const std::shared_ptr<DoubleValuesSource> distanceSource;
    const double limit;

  private:
    DistanceRangeQuery(std::shared_ptr<Query> inner,
                       std::shared_ptr<DoubleValuesSource> distanceSource,
                       double limit);

  public:
    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<DistanceRangeQuery> outerInstance;

      std::shared_ptr<Weight> w;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<DistanceRangeQuery> outerInstance, float boost,
          std::shared_ptr<Weight> w);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
      {
        GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
      private:
        std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<DoubleValues> v;
        std::shared_ptr<DocIdSetIterator> approximation;

      public:
        TwoPhaseIteratorAnonymousInnerClass(
            std::shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<DocIdSetIterator> approximation,
            std::shared_ptr<DoubleValues> v);

        bool matches()  override;

        float matchCost() override;

      protected:
        std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
              org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
        }
      };

    public:
      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<DistanceRangeQuery> shared_from_this()
    {
      return std::static_pointer_cast<DistanceRangeQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PointVectorStrategy> shared_from_this()
  {
    return std::static_pointer_cast<PointVectorStrategy>(
        org.apache.lucene.spatial.SpatialStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::deque
