#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Field;
}

namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::spatial
{
class ShapeValuesSource;
}
namespace org::apache::lucene::spatial::util
{
class ShapeValuesPredicate;
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
namespace org::apache::lucene::spatial
{
class ShapeValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
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
namespace org::apache::lucene::spatial::serialized
{

using Field = org::apache::lucene::document::Field;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::io::BinaryCodec;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A SpatialStrategy based on serializing a Shape stored into BinaryDocValues.
 * This is not at all fast; it's designed to be used in conjunction with another
 * index based SpatialStrategy that is approximated (like {@link
 * org.apache.lucene.spatial.prefix.RecursivePrefixTreeStrategy}) to add
 * precision or eventually make more specific / advanced calculations on the
 * per-document geometry. The serialization uses Spatial4j's {@link
 * org.locationtech.spatial4j.io.BinaryCodec}.
 *
 * @lucene.experimental
 */
class SerializedDVStrategy : public SpatialStrategy
{
  GET_CLASS_NAME(SerializedDVStrategy)

  /**
   * A cache heuristic for the buf size based on the last shape size.
   */
  // TODO do we make this non-volatile since it's merely a heuristic?
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile int indexLastBufSize = 8 * 1024;
  int indexLastBufSize = 8 * 1024; // 8KB default on first run

  /**
   * Constructs the spatial strategy with its mandatory arguments.
   */
public:
  SerializedDVStrategy(std::shared_ptr<SpatialContext> ctx,
                       const std::wstring &fieldName);

  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) override;

private:
  class FilterOutputStreamAnonymousInnerClass : public FilterOutputStream
  {
    GET_CLASS_NAME(FilterOutputStreamAnonymousInnerClass)
  private:
    std::shared_ptr<SerializedDVStrategy> outerInstance;

    std::shared_ptr<BytesRef> bytesRef;

  public:
    FilterOutputStreamAnonymousInnerClass(
        std::shared_ptr<SerializedDVStrategy> outerInstance,
        std::shared_ptr<BytesRef> bytesRef);

    void write(std::deque<char> &b, int off,
               int len)  override;

  protected:
    std::shared_ptr<FilterOutputStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterOutputStreamAnonymousInnerClass>(
          java.io.FilterOutputStream::shared_from_this());
    }
  };

public:
  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  /**
   * Returns a Query that should be used in a random-access fashion.
   * Use in another manner will be SLOW.
   */
  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

  /**
   * Provides access to each shape per document
   */
  // TODO raise to SpatialStrategy
  virtual std::shared_ptr<ShapeValuesSource> makeShapeValueSource();

  /** Warning: don't iterate over the results of this query; it's designed for
   * use in a random-access fashion by {@link TwoPhaseIterator}.
   */
public:
  class PredicateValueSourceQuery : public Query
  {
    GET_CLASS_NAME(PredicateValueSourceQuery)
  private:
    const std::shared_ptr<ShapeValuesPredicate> predicateValueSource;

  public:
    PredicateValueSourceQuery(
        std::shared_ptr<ShapeValuesPredicate> predicateValueSource);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<PredicateValueSourceQuery> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<PredicateValueSourceQuery> outerInstance,
          float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    bool equals(std::any other) override;

    virtual int hashCode();

    std::wstring toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<PredicateValueSourceQuery> shared_from_this()
    {
      return std::static_pointer_cast<PredicateValueSourceQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  }; // PredicateValueSourceQuery

  /**
   * Implements a ShapeValueSource by deserializing a Shape from BinaryDocValues
   * using BinaryCodec.
   * @see #makeShapeValueSource()
   */
public:
  class ShapeDocValueSource : public ShapeValuesSource
  {
    GET_CLASS_NAME(ShapeDocValueSource)

  private:
    const std::wstring fieldName;
    const std::shared_ptr<BinaryCodec> binaryCodec; // spatial4j

    ShapeDocValueSource(const std::wstring &fieldName,
                        std::shared_ptr<BinaryCodec> binaryCodec);

  public:
    std::shared_ptr<ShapeValues>
    getValues(std::shared_ptr<LeafReaderContext> readerContext) throw(
        IOException) override;

  private:
    class ShapeValuesAnonymousInnerClass : public ShapeValues
    {
      GET_CLASS_NAME(ShapeValuesAnonymousInnerClass)
    private:
      std::shared_ptr<ShapeDocValueSource> outerInstance;

      std::shared_ptr<BinaryDocValues> docValues;

    public:
      ShapeValuesAnonymousInnerClass(
          std::shared_ptr<ShapeDocValueSource> outerInstance,
          std::shared_ptr<BinaryDocValues> docValues);

      bool advanceExact(int doc)  override;

      std::shared_ptr<Shape> value()  override;

    protected:
      std::shared_ptr<ShapeValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ShapeValuesAnonymousInnerClass>(
            org.apache.lucene.spatial.ShapeValues::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ShapeDocValueSource> shared_from_this()
    {
      return std::static_pointer_cast<ShapeDocValueSource>(
          org.apache.lucene.spatial.ShapeValuesSource::shared_from_this());
    }
  }; // ShapeDocValueSource

protected:
  std::shared_ptr<SerializedDVStrategy> shared_from_this()
  {
    return std::static_pointer_cast<SerializedDVStrategy>(
        org.apache.lucene.spatial.SpatialStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::serialized
