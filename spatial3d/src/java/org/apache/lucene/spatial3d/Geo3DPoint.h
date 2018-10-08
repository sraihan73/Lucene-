#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::geo
{
class Polygon;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
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

using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using Polygon = org::apache::lucene::geo::Polygon;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using Query = org::apache::lucene::search::Query;

/**
 * Add this to a document to index lat/lon or x/y/z point, indexed as a 3D
 * point. Multiple values are allowed: just add multiple Geo3DPoint to the
 * document with the same field name. <p> This field defines static factory
 * methods for creating a shape query: <ul> <li>{@link #newShapeQuery
 * newShapeQuery()} for matching all points inside a specified shape
 * </ul>
 * @see PointValues
 *  @lucene.experimental */
class Geo3DPoint final : public Field
{
  GET_CLASS_NAME(Geo3DPoint)

  /** Indexing {@link FieldType}. */
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
  static Geo3DPoint::StaticConstructor staticConstructor;

  /**
   * Creates a new Geo3DPoint field with the specified latitude, longitude (in
   * degrees).
   *
   * @throws IllegalArgumentException if the field name is null or latitude or
   * longitude are out of bounds
   */
public:
  Geo3DPoint(const std::wstring &name, double latitude, double longitude);

  /**
   * Create a query for matching points within the specified distance of the
   * supplied location.
   * @param field field name. must not be null.  Note that because
   * {@link PlanetModel#WGS84} is used, this query is approximate and may have
   * up to 0.5% error.
   *
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param radiusMeters maximum distance from the center in meters: must be
   * non-negative and finite.
   * @return query matching points within this distance
   * @throws IllegalArgumentException if {@code field} is null, location has
   * invalid coordinates, or radius is invalid.
   */
  static std::shared_ptr<Query> newDistanceQuery(const std::wstring &field,
                                                 double const latitude,
                                                 double const longitude,
                                                 double const radiusMeters);

  /**
   * Create a query for matching a box.
   * <p>
   * The box may cross over the dateline.
   * @param field field name. must not be null.
   * @param minLatitude latitude lower bound: must be within standard +/-90
   * coordinate bounds.
   * @param maxLatitude latitude upper bound: must be within standard +/-90
   * coordinate bounds.
   * @param minLongitude longitude lower bound: must be within standard +/-180
   * coordinate bounds.
   * @param maxLongitude longitude upper bound: must be within standard +/-180
   * coordinate bounds.
   * @return query matching points within this box
   * @throws IllegalArgumentException if {@code field} is null, or the box has
   * invalid coordinates.
   */
  static std::shared_ptr<Query> newBoxQuery(const std::wstring &field,
                                            double const minLatitude,
                                            double const maxLatitude,
                                            double const minLongitude,
                                            double const maxLongitude);

  /**
   * Create a query for matching a polygon.  The polygon should have a limited
   * number of edges (less than 100) and be well-defined, with well-separated
   * vertices. <p> The supplied {@code polygons} must be clockwise on the
   * outside level, counterclockwise on the next level in, etc.
   * @param field field name. must not be null.
   * @param polygons is the deque of polygons to use to construct the query; must
   * be at least one.
   * @return query matching points within this polygon
   */
  static std::shared_ptr<Query> newPolygonQuery(const std::wstring &field,
                                                std::deque<Polygon> &polygons);

  /**
   * Create a query for matching a large polygon.  This differs from the related
   * newPolygonQuery in that it does little or no legality checking and is
   * optimized for very large numbers of polygon edges. <p> The supplied {@code
   * polygons} must be clockwise on the outside level, counterclockwise on the
   * next level in, etc.
   * @param field field name. must not be null.
   * @param polygons is the deque of polygons to use to construct the query; must
   * be at least one.
   * @return query matching points within this polygon
   */
  static std::shared_ptr<Query>
  newLargePolygonQuery(const std::wstring &field,
                       std::deque<Polygon> &polygons);

  /**
   * Create a query for matching a path.
   * <p>
   * @param field field name. must not be null.
   * @param pathLatitudes latitude values for points of the path: must be within
   * standard +/-90 coordinate bounds.
   * @param pathLongitudes longitude values for points of the path: must be
   * within standard +/-180 coordinate bounds.
   * @param pathWidthMeters width of the path in meters.
   * @return query matching points within this polygon
   */
  static std::shared_ptr<Query>
  newPathQuery(const std::wstring &field, std::deque<double> &pathLatitudes,
               std::deque<double> &pathLongitudes,
               double const pathWidthMeters);

  /**
   * Creates a new Geo3DPoint field with the specified x,y,z.
   *
   * @throws IllegalArgumentException if the field name is null or latitude or
   * longitude are out of bounds
   */
  Geo3DPoint(const std::wstring &name, double x, double y, double z);

private:
  void fillFieldsData(double x, double y, double z);

  // public helper methods (e.g. for queries)

  /** Encode single dimension */
public:
  static void encodeDimension(double value, std::deque<char> &bytes,
                              int offset);

  /** Decode single dimension */
  static double decodeDimension(std::deque<char> &value, int offset);

  /** Returns a query matching all points inside the provided shape.
   *
   * @param field field name. must not be {@code null}.
   * @param shape Which {@link GeoShape} to match
   */
  static std::shared_ptr<Query> newShapeQuery(const std::wstring &field,
                                              std::shared_ptr<GeoShape> shape);

  virtual std::wstring toString();

protected:
  std::shared_ptr<Geo3DPoint> shared_from_this()
  {
    return std::static_pointer_cast<Geo3DPoint>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d
