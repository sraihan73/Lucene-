#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::geo
{
class Polygon;
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
namespace org::apache::lucene::spatial3d
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using SortField = org::apache::lucene::search::SortField;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using Polygon = org::apache::lucene::geo::Polygon;

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;

/**
 * An per-document 3D location field.
 * <p>
 * Sorting by distance is efficient. Multiple values for the same field in one
 * document is allowed. <p> This field defines static factory methods for common
 * operations: <ul> <li>TBD
 * </ul>
 * <p>
 * If you also need query operations, you should add a separate {@link
 * Geo3DPoint} instance. <p> <b>WARNING</b>: Values are indexed with some loss
 * of precision from the original {@code double} values (4.190951585769653E-8
 * for the latitude component and 8.381903171539307E-8 for longitude).
 * @see Geo3DPoint
 */
class Geo3DDocValuesField : public Field
{
  GET_CLASS_NAME(Geo3DDocValuesField)

  // These are the multiplicative constants we need to use to arrive at values
  // that fit in 21 bits. The formula we use to go from double to encoded value
  // is:  Math.floor((value - minimum) * factor + 0.5) If we plug in maximum for
  // value, we should get 0x1FFFFF. So, 0x1FFFFF = Math.floor((maximum -
  // minimum) * factor + 0.5) We factor out the 0.5 and Math.floor by stating
  // instead: 0x1FFFFF = (maximum - minimum) * factor So, factor = 0x1FFFFF /
  // (maximum - minimum)

private:
  static const double inverseMaximumValue;

  static const double inverseXFactor;
  static const double inverseYFactor;
  static const double inverseZFactor;

  static const double xFactor;
  static const double yFactor;
  static const double zFactor;

  // Fudge factor for step adjustments.  This is here solely to handle
  // inaccuracies in bounding boxes that occur because of quantization.  For
  // unknown reasons, the fudge factor needs to be 10.0 rather than 1.0.  See
  // LUCENE-7430.

  static constexpr double STEP_FUDGE = 10.0;

  // These values are the delta between a value and the next value in each
  // specific dimension

  static const double xStep;
  static const double yStep;
  static const double zStep;

  /**
   * Type for a Geo3DDocValuesField
   * <p>
   * Each value stores a 64-bit long where the three values (x, y, and z) are
   * given 21 bits each.  Each 21-bit value represents the maximum extent in
   * that dimension for the WGS84 planet model.
   */
public:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static Geo3DDocValuesField::StaticConstructor staticConstructor;

  /**
   * Creates a new Geo3DDocValuesField with the specified x, y, and z
   * @param name field name
   * @param point is the point.
   * @throws IllegalArgumentException if the field name is null or the point is
   * out of bounds
   */
public:
  Geo3DDocValuesField(const std::wstring &name,
                      std::shared_ptr<GeoPoint> point);

  /**
   * Creates a new Geo3DDocValuesField with the specified x, y, and z
   * @param name field name
   * @param x is the x value for the point.
   * @param y is the y value for the point.
   * @param z is the z value for the point.
   * @throws IllegalArgumentException if the field name is null or x, y, or z
   * are out of bounds
   */
  Geo3DDocValuesField(const std::wstring &name, double const x, double const y,
                      double const z);

  /**
   * Change the values of this field
   * @param point is the point.
   * @throws IllegalArgumentException if the point is out of bounds
   */
  virtual void setLocationValue(std::shared_ptr<GeoPoint> point);

  /**
   * Change the values of this field
   * @param x is the x value for the point.
   * @param y is the y value for the point.
   * @param z is the z value for the point.
   * @throws IllegalArgumentException if x, y, or z are out of bounds
   */
  virtual void setLocationValue(double const x, double const y, double const z);

  /** Encode a point.
   * @param point is the point
   * @return the encoded long
   */
  static int64_t encodePoint(std::shared_ptr<GeoPoint> point);

  /** Encode a point.
   * @param x is the x value
   * @param y is the y value
   * @param z is the z value
   * @return the encoded long
   */
  static int64_t encodePoint(double const x, double const y, double const z);

  /** Decode GeoPoint value from long docvalues value.
   * @param docValue is the doc values value.
   * @return the GeoPoint.
   */
  static std::shared_ptr<GeoPoint> decodePoint(int64_t const docValue);

  /** Decode X value from long docvalues value.
   * @param docValue is the doc values value.
   * @return the x value.
   */
  static double decodeXValue(int64_t const docValue);

  /** Decode Y value from long docvalues value.
   * @param docValue is the doc values value.
   * @return the y value.
   */
  static double decodeYValue(int64_t const docValue);

  /** Decode Z value from long docvalues value.
   * @param docValue is the doc values value.
   * @return the z value.
   */
  static double decodeZValue(int64_t const docValue);

  /** Round the provided X value down, by encoding it, decrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundDownX(double const startValue);

  /** Round the provided X value up, by encoding it, incrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundUpX(double const startValue);

  /** Round the provided Y value down, by encoding it, decrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundDownY(double const startValue);

  /** Round the provided Y value up, by encoding it, incrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundUpY(double const startValue);

  /** Round the provided Z value down, by encoding it, decrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundDownZ(double const startValue);

  /** Round the provided Z value up, by encoding it, incrementing it, and
   * unencoding it.
   * @param startValue is the starting value.
   * @return the rounded value.
   */
  static double roundUpZ(double const startValue);

  // For encoding/decoding, we generally want the following behavior:
  // (1) If you encode the maximum value or the minimum value, the resulting int
  // fits in 21 bits. (2) If you decode an encoded value, you get back the
  // original value for both the minimum and maximum planet model values. (3)
  // Rounding occurs such that a small delta from the minimum and maximum planet
  // model values still returns the same values -- that is, these are in the
  // center of the range of input values that should return the minimum or
  // maximum when decoded

private:
  static int encodeX(double const x);

  static double decodeX(int const x);

  static int encodeY(double const y);

  static double decodeY(int const y);

  static int encodeZ(double const z);

  static double decodeZ(int const z);

  /** helper: checks a fieldinfo and throws exception if its definitely not a
   * Geo3DDocValuesField */
public:
  static void checkCompatible(std::shared_ptr<FieldInfo> fieldInfo);

  virtual std::wstring toString();

  /**
   * Creates a SortField for sorting by distance within a circle.
   * <p>
   * This sort orders documents by ascending distance from the location. The
   * value returned in {@link FieldDoc} for the hits contains a Double instance
   * with the distance in meters. <p> If a document is missing the field, then
   * by default it is treated as having {@link Double#POSITIVE_INFINITY}
   * distance (missing values sort last). <p> If a document contains multiple
   * values for the field, the <i>closest</i> distance from the circle center is
   * used.
   *
   * @param field field name. must not be null.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param maxRadiusMeters is the maximum radius in meters.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or circle has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newDistanceSort(const std::wstring &field, double const latitude,
                  double const longitude, double const maxRadiusMeters);

  /**
   * Creates a SortField for sorting by distance along a path.
   * <p>
   * This sort orders documents by ascending distance along the described path.
   * The value returned in {@link FieldDoc} for the hits contains a Double
   * instance with the distance in meters. <p> If a document is missing the
   * field, then by default it is treated as having {@link
   * Double#POSITIVE_INFINITY} distance (missing values sort last). <p> If a
   * document contains multiple values for the field, the <i>closest</i>
   * distance along the path is used.
   *
   * @param field field name. must not be null.
   * @param pathLatitudes latitude values for points of the path: must be within
   * standard +/-90 coordinate bounds.
   * @param pathLongitudes longitude values for points of the path: must be
   * within standard +/-180 coordinate bounds.
   * @param pathWidthMeters width of the path in meters.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or path has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newPathSort(const std::wstring &field, std::deque<double> &pathLatitudes,
              std::deque<double> &pathLongitudes,
              double const pathWidthMeters);

  // Outside distances

  /**
   * Creates a SortField for sorting by outside distance from a circle.
   * <p>
   * This sort orders documents by ascending outside distance from the circle.
   * Points within the circle have distance 0.0. The value returned in {@link
   * FieldDoc} for the hits contains a Double instance with the distance in
   * meters. <p> If a document is missing the field, then by default it is
   * treated as having {@link Double#POSITIVE_INFINITY} distance (missing values
   * sort last). <p> If a document contains multiple values for the field, the
   * <i>closest</i> distance to the circle is used.
   *
   * @param field field name. must not be null.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param maxRadiusMeters is the maximum radius in meters.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or location has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newOutsideDistanceSort(const std::wstring &field, double const latitude,
                         double const longitude, double const maxRadiusMeters);

  /**
   * Creates a SortField for sorting by outside distance from a box.
   * <p>
   * This sort orders documents by ascending outside distance from the box.
   * Points within the box have distance 0.0. The value returned in {@link
   * FieldDoc} for the hits contains a Double instance with the distance in
   * meters. <p> If a document is missing the field, then by default it is
   * treated as having {@link Double#POSITIVE_INFINITY} distance (missing values
   * sort last). <p> If a document contains multiple values for the field, the
   * <i>closest</i> distance to the box is used.
   *
   * @param field field name. must not be null.
   * @param minLatitude latitude lower bound: must be within standard +/-90
   * coordinate bounds.
   * @param maxLatitude latitude upper bound: must be within standard +/-90
   * coordinate bounds.
   * @param minLongitude longitude lower bound: must be within standard +/-180
   * coordinate bounds.
   * @param maxLongitude longitude upper bound: must be within standard +/-180
   * coordinate bounds.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or box has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newOutsideBoxSort(const std::wstring &field, double const minLatitude,
                    double const maxLatitude, double const minLongitude,
                    double const maxLongitude);

  /**
   * Creates a SortField for sorting by outside distance from a polygon.
   * <p>
   * This sort orders documents by ascending outside distance from the polygon.
   * Points within the polygon have distance 0.0. The value returned in {@link
   * FieldDoc} for the hits contains a Double instance with the distance in
   * meters. <p> If a document is missing the field, then by default it is
   * treated as having {@link Double#POSITIVE_INFINITY} distance (missing values
   * sort last). <p> If a document contains multiple values for the field, the
   * <i>closest</i> distance to the polygon is used.
   *
   * @param field field name. must not be null.
   * @param polygons is the deque of polygons to use to construct the query; must
   * be at least one.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or polygon has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newOutsidePolygonSort(const std::wstring &field,
                        std::deque<Polygon> &polygons);

  /**
   * Creates a SortField for sorting by outside distance from a large polygon.
   * This differs from the related newOutsideLargePolygonSort in that it does
   * little or no legality checking and is optimized for very large numbers of
   * polygon edges. <p> This sort orders documents by ascending outside distance
   * from the polygon.  Points within the polygon have distance 0.0. The value
   * returned in {@link FieldDoc} for the hits contains a Double instance with
   * the distance in meters. <p> If a document is missing the field, then by
   * default it is treated as having {@link Double#POSITIVE_INFINITY} distance
   * (missing values sort last).
   * <p>
   * If a document contains multiple values for the field, the <i>closest</i>
   * distance to the polygon is used.
   *
   * @param field field name. must not be null.
   * @param polygons is the deque of polygons to use to construct the query; must
   * be at least one.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or polygon has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newOutsideLargePolygonSort(const std::wstring &field,
                             std::deque<Polygon> &polygons);

  /**
   * Creates a SortField for sorting by outside distance from a path.
   * <p>
   * This sort orders documents by ascending outside distance from the described
   * path. Points within the path are given the distance of 0.0.  The value
   * returned in {@link FieldDoc} for the hits contains a Double instance with
   * the distance in meters. <p> If a document is missing the field, then by
   * default it is treated as having {@link Double#POSITIVE_INFINITY} distance
   * (missing values sort last).
   * <p>
   * If a document contains multiple values for the field, the <i>closest</i>
   * distance from the path is used.
   *
   * @param field field name. must not be null.
   * @param pathLatitudes latitude values for points of the path: must be within
   * standard +/-90 coordinate bounds.
   * @param pathLongitudes longitude values for points of the path: must be
   * within standard +/-180 coordinate bounds.
   * @param pathWidthMeters width of the path in meters.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or path has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField> newOutsidePathSort(
      const std::wstring &field, std::deque<double> &pathLatitudes,
      std::deque<double> &pathLongitudes, double const pathWidthMeters);

protected:
  std::shared_ptr<Geo3DDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<Geo3DDocValuesField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d
