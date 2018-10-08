#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/spatial/ShapeValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"

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
namespace org::apache::lucene::spatial::bbox
{

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

/**
 * A SpatialStrategy for indexing and searching Rectangles by storing its
 * coordinates in numeric fields. It supports all {@link SpatialOperation}s and
 * has a custom overlap relevancy. It is based on GeoPortal's <a
 * href="http://geoportal.svn.sourceforge.net/svnroot/geoportal/Geoportal/trunk/src/com/esri/gpt/catalog/lucene/SpatialClauseAdapter.java">SpatialClauseAdapter</a>.
 * <p>
 * <b>Characteristics:</b>
 * <br>
 * <ul>
 * <li>Only indexes Rectangles; just one per field value. Other shapes can be
 * provided and the bounding box will be used.</li> <li>Can query only by a
 * Rectangle. Providing other shapes is an error.</li> <li>Supports most {@link
 * SpatialOperation}s but not Overlaps.</li> <li>Uses the DocValues API for any
 * sorting / relevancy.</li>
 * </ul>
 * <p>
 * <b>Implementation:</b>
 * <p>
 * This uses 4 double fields for minX, maxX, minY, maxY
 * and a bool to mark a dateline cross. Depending on the particular {@link
 * SpatialOperation}s, there are a variety of range queries on {@link
 * DoublePoint}s to be done. The {@link
 * #makeOverlapRatioValueSource(org.locationtech.spatial4j.shape.Rectangle,
 * double)} works by calculating the query bbox overlap percentage against the
 * indexed shape overlap percentage. The indexed shape's coordinates are
 * retrieved from
 * {@link org.apache.lucene.index.LeafReader#getNumericDocValues}.
 *
 * @lucene.experimental
 */
class BBoxStrategy : public SpatialStrategy
{
  GET_CLASS_NAME(BBoxStrategy)

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
  static BBoxStrategy::StaticConstructor staticConstructor;

public:
  static const std::wstring SUFFIX_MINX;
  static const std::wstring SUFFIX_MAXX;
  static const std::wstring SUFFIX_MINY;
  static const std::wstring SUFFIX_MAXY;
  static const std::wstring SUFFIX_XDL;

  /*
   * The Bounding Box gets stored as four fields for x/y min/max and a flag
   * that says if the box crosses the dateline (xdl).
   */
  const std::wstring field_bbox;
  const std::wstring field_minX;
  const std::wstring field_minY;
  const std::wstring field_maxX;
  const std::wstring field_maxY;
  const std::wstring field_xdl; // crosses dateline

private:
  const std::shared_ptr<FieldType>
      optionsFieldType; // from constructor; aggregate field type used to
                        // express all options
  const int fieldsLen;
  const bool hasStored;
  const bool hasDocVals;
  const bool hasPointVals;
  const std::shared_ptr<FieldType> xdlFieldType;

  /**
   * Creates a new {@link BBoxStrategy} instance that uses {@link DoublePoint}
   * and {@link DoublePoint#newRangeQuery}
   */
public:
  static std::shared_ptr<BBoxStrategy>
  newInstance(std::shared_ptr<SpatialContext> ctx,
              const std::wstring &fieldNamePrefix);

  /**
   * Creates this strategy.
   * {@code fieldType} is used to customize the indexing options of the 4 number
   * fields, and to a lesser degree the XDL field too. Search requires
   * pointValues (or legacy numerics), and relevancy requires docValues. If
   * these features aren't needed then disable them.
   */
  BBoxStrategy(std::shared_ptr<SpatialContext> ctx,
               const std::wstring &fieldNamePrefix,
               std::shared_ptr<FieldType> fieldType);

  /** Returns a field type representing the set of field options. This is
   * identical to what was passed into the constructor.  It's frozen. */
  virtual std::shared_ptr<FieldType> getFieldType();

  //---------------------------------
  // Indexing
  //---------------------------------

  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Shape> shape) override;

private:
  std::deque<std::shared_ptr<Field>>
  createIndexableFields(std::shared_ptr<Rectangle> bbox);

  //---------------------------------
  // Value Source / Relevancy
  //---------------------------------

  /**
   * Provides access to each rectangle per document as a {@link
   * ShapeValuesSource}
   */
  // TODO raise to SpatialStrategy
public:
  virtual std::shared_ptr<ShapeValuesSource> makeShapeValueSource();

  std::shared_ptr<DoubleValuesSource>
  makeDistanceValueSource(std::shared_ptr<Point> queryPoint,
                          double multiplier) override;

  /** Returns a similarity based on {@link BBoxOverlapRatioValueSource}. This is
   * just a convenience method. */
  virtual std::shared_ptr<DoubleValuesSource>
  makeOverlapRatioValueSource(std::shared_ptr<Rectangle> queryBox,
                              double queryTargetProportion);

  //---------------------------------
  // Query Building
  //---------------------------------

  //  Utility on SpatialStrategy?
  //  public Query makeQueryWithValueSource(SpatialArgs args, ValueSource
  //  valueSource) {
  //    return new CustomScoreQuery(makeQuery(args), new
  //    FunctionQuery(valueSource));
  // or...
  //  return new BooleanQuery.Builder()
  //      .add(new FunctionQuery(valueSource),
  //      BooleanClause.Occur.MUST)//matches everything and provides score
  //      .add(filterQuery, BooleanClause.Occur.FILTER)//filters (score isn't
  //      used)
  //  .build();
  //  }

  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

  /**
   * Constructs a query to retrieve documents that fully contain the input
   * envelope.
   *
   * @return the spatial query
   */
  virtual std::shared_ptr<Query> makeContains(std::shared_ptr<Rectangle> bbox);

  /**
   * Constructs a query to retrieve documents that are disjoint to the input
   * envelope.
   *
   * @return the spatial query
   */
  virtual std::shared_ptr<Query> makeDisjoint(std::shared_ptr<Rectangle> bbox);

  /**
   * Constructs a query to retrieve documents that equal the input envelope.
   *
   * @return the spatial query
   */
  virtual std::shared_ptr<Query> makeEquals(std::shared_ptr<Rectangle> bbox);

  /**
   * Constructs a query to retrieve documents that intersect the input envelope.
   *
   * @return the spatial query
   */
  virtual std::shared_ptr<Query>
  makeIntersects(std::shared_ptr<Rectangle> bbox);

  /**
   * Makes a bool query based upon a collection of queries and a logical
   * operator.
   *
   * @param occur the logical operator
   * @param queries the query collection
   * @return the query
   */
  virtual std::shared_ptr<BooleanQuery> makeQuery(BooleanClause::Occur occur,
                                                  std::deque<Query> &queries);

  /**
   * Constructs a query to retrieve documents are fully within the input
   * envelope.
   *
   * @return the spatial query
   */
  virtual std::shared_ptr<Query> makeWithin(std::shared_ptr<Rectangle> bbox);

  /**
   * Constructs a query to retrieve documents that do or do not cross the date
   * line.
   *
   * @param crossedDateLine <code>true</true> for documents that cross the date
   * line
   * @return the query
   */
private:
  std::shared_ptr<Query> makeXDL(bool crossedDateLine);

  /**
   * Constructs a query to retrieve documents that do or do not cross the date
   * line and match the supplied spatial query.
   *
   * @param crossedDateLine <code>true</true> for documents that cross the date
   * line
   * @param query the spatial query
   * @return the query
   */
  std::shared_ptr<Query> makeXDL(bool crossedDateLine,
                                 std::shared_ptr<Query> query);

  std::shared_ptr<Query> makeNumberTermQuery(const std::wstring &field,
                                             double number);

  /**
   * Returns a numeric range query based on FieldType
   * {@link DoublePoint#newRangeQuery} is used for indexes created using {@link
   * DoublePoint} fields
   *
   * @param fieldname field name. must not be <code>null</code>.
   * @param min minimum value of the range.
   * @param max maximum value of the range.
   * @param minInclusive include the minimum value if <code>true</code>.
   * @param maxInclusive include the maximum value if <code>true</code>
   */
  std::shared_ptr<Query> makeNumericRangeQuery(const std::wstring &fieldname,
                                               std::optional<double> &min,
                                               std::optional<double> &max,
                                               bool minInclusive,
                                               bool maxInclusive);

protected:
  std::shared_ptr<BBoxStrategy> shared_from_this()
  {
    return std::static_pointer_cast<BBoxStrategy>(
        org.apache.lucene.spatial.SpatialStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/bbox/
