#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::geo
{

/**
 * Represents a closed polygon on the earth's surface.  You can either construct
 * the Polygon directly yourself with {@code double[]} coordinates, or use
 * {@link Polygon#fromGeoJSON} if you have a polygon already encoded as a <a
 * href="http://geojson.org/geojson-spec.html">GeoJSON</a> string. <p> NOTES:
 * <ol>
 *   <li>Coordinates must be in clockwise order, except for holes. Holes must be
 * in counter-clockwise order. <li>The polygon must be closed: the first and
 * last coordinates need to have the same values. <li>The polygon must not be
 * self-crossing, otherwise may result in unexpected behavior. <li>All
 * latitude/longitude values must be in decimal degrees. <li>Polygons cannot
 * cross the 180th meridian. Instead, use two polygons: one on each side.
 *   <li>For more advanced GeoSpatial indexing and query operations see the
 * {@code spatial-extras} module
 * </ol>
 * @lucene.experimental
 */
class Polygon final : public std::enable_shared_from_this<Polygon>
{
  GET_CLASS_NAME(Polygon)
private:
  std::deque<double> const polyLats;
  std::deque<double> const polyLons;
  std::deque<std::shared_ptr<Polygon>> const holes;

  /** minimum latitude of this polygon's bounding box area */
public:
  const double minLat;
  /** maximum latitude of this polygon's bounding box area */
  const double maxLat;
  /** minimum longitude of this polygon's bounding box area */
  const double minLon;
  /** maximum longitude of this polygon's bounding box area */
  const double maxLon;

  /**
   * Creates a new Polygon from the supplied latitude/longitude array, and
   * optionally any holes.
   */
  Polygon(std::deque<double> &polyLats, std::deque<double> &polyLons,
          std::deque<Polygon> &holes);

  /** Returns a copy of the internal latitude array */
  std::deque<double> getPolyLats();

  /** Returns a copy of the internal longitude array */
  std::deque<double> getPolyLons();

  /** Returns a copy of the internal holes array */
  std::deque<std::shared_ptr<Polygon>> getHoles();

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

  /** Parses a standard GeoJSON polygon string.  The type of the incoming
   * GeoJSON object must be a Polygon or MultiPolygon, optionally embedded under
   * a "type: Feature".  A Polygon will return as a length 1 array, while a
   * MultiPolygon will be 1 or more in length.
   *
   *  <p>See <a href="http://geojson.org/geojson-spec.html">the GeoJSON
   * specification</a>. */
  static std::deque<std::shared_ptr<Polygon>>
  fromGeoJSON(const std::wstring &geojson) ;
};

} // #include  "core/src/java/org/apache/lucene/geo/
