#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/geo/Polygon.h"

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

/*
  We accept either a whole type: Feature, like this:

    { "type": "Feature",
      "geometry": {
         "type": "Polygon",
         "coordinates": [
           [ [100.0, 0.0], [101.0, 0.0], [101.0, 1.0],
             [100.0, 1.0], [100.0, 0.0] ]
           ]
       },
       "properties": {
         "prop0": "value0",
         "prop1": {"this": "that"}
         }
       }

   Or the inner object with type: Multi/Polygon.

   Or a type: FeatureCollection, if it has only one Feature which is a Polygon
  or MultiPolyon.

   type: MultiPolygon (union of polygons) is also accepted.
*/

/** Does minimal parsing of a GeoJSON object, to extract either Polygon or
 * MultiPolygon, either directly as a the top-level type, or if the top-level
 * type is Feature, as the geometry of that feature. */

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") class
// SimpleGeoJSONPolygonParser
class SimpleGeoJSONPolygonParser
    : public std::enable_shared_from_this<SimpleGeoJSONPolygonParser>
{
public:
  const std::wstring input;

private:
  int upto = 0;
  std::wstring polyType;
  std::deque<std::any> coordinates;

public:
  SimpleGeoJSONPolygonParser(const std::wstring &input);

  virtual std::deque<std::shared_ptr<Polygon>> parse() ;

  /** path is the "address" by keys of where we are, e.g. geometry.coordinates
   */
private:
  void parseObject(const std::wstring &path) ;

  /** Returns true if the object path is a valid location to see a Multi/Polygon
   * geometry */
  bool isValidGeometryPath(const std::wstring &path);

  std::shared_ptr<Polygon>
  parsePolygon(std::deque<std::any> &coordinates) ;

  /** Parses [[lat, lon], [lat, lon] ...] into 2d double array */
  std::deque<std::deque<double>>
  parsePoints(std::deque<std::any> &o) ;

  std::deque<std::any>
  parseArray(const std::wstring &path) ;

  std::shared_ptr<Number> parseNumber() ;

  std::wstring parseString() ;

  wchar_t peek() ;

  /** Scans across whitespace and consumes the expected character, or throws
   * {@code ParseException} if the character is wrong */
  void scan(wchar_t expected) ;

  void readEnd() ;

  /** Scans the expected string, or throws {@code ParseException} */
  void scan(const std::wstring &expected) ;

  static bool isJSONWhitespace(wchar_t ch);

  /** When calling this, upto should be at the position of the incorrect
   * character! */
  std::shared_ptr<ParseException>
  newParseException(const std::wstring &details) ;
};

} // #include  "core/src/java/org/apache/lucene/geo/
