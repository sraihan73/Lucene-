#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/geo/Rectangle.h"

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

/** static methods for testing geo */
class GeoTestUtil : public std::enable_shared_from_this<GeoTestUtil>
{
  GET_CLASS_NAME(GeoTestUtil)

  /** returns next pseudorandom latitude (anywhere) */
public:
  static double nextLatitude();

  /** returns next pseudorandom longitude (anywhere) */
  static double nextLongitude();

  /**
   * Returns next double within range.
   * <p>
   * Don't pass huge numbers or infinity or anything like that yet. may have
   * bugs!
   */
  // the goal is to adjust random number generation to test edges, create more
  // duplicates, create "one-offs" in floating point space, etc. we do this by
  // first picking a good "base value" (explicitly targeting edges, zero if
  // allowed, or "discrete values"). but it also ensures we pick any double in
  // the range and generally still produces randomish looking numbers. then we
  // sometimes perturb that by one ulp.
private:
  static double nextDoubleInternal(double low, double high);

  /** returns next pseudorandom latitude, kinda close to {@code otherLatitude}
   */
  static double nextLatitudeNear(double otherLatitude, double delta);

  /** returns next pseudorandom longitude, kinda close to {@code otherLongitude}
   */
  static double nextLongitudeNear(double otherLongitude, double delta);

  /**
   * returns next pseudorandom latitude, kinda close to {@code
   * minLatitude/maxLatitude} <b>NOTE:</b>minLatitude/maxLatitude are merely
   * guidelines. the returned value is sometimes outside of that range! this is
   * to facilitate edge testing of lines
   */
  static double nextLatitudeBetween(double minLatitude, double maxLatitude);

  /**
   * returns next pseudorandom longitude, kinda close to {@code
   * minLongitude/maxLongitude} <b>NOTE:</b>minLongitude/maxLongitude are merely
   * guidelines. the returned value is sometimes outside of that range! this is
   * to facilitate edge testing of lines
   */
  static double nextLongitudeBetween(double minLongitude, double maxLongitude);

  /** Returns the next point around a line (more or less) */
  static std::deque<double> nextPointAroundLine(double lat1, double lon1,
                                                 double lat2, double lon2);

  /** Returns next point (lat/lon) for testing near a Box. It may cross the
   * dateline */
public:
  static std::deque<double>
  nextPointNear(std::shared_ptr<Rectangle> rectangle);

  /** Returns next point (lat/lon) for testing near a Polygon */
  // see http://www-ma2.upc.es/geoc/Schirra-pointPolygon.pdf for more info on
  // some of these strategies
  static std::deque<double> nextPointNear(std::shared_ptr<Polygon> polygon);

  /** Returns next box for testing near a Polygon */
  static std::shared_ptr<Rectangle>
  nextBoxNear(std::shared_ptr<Polygon> polygon);

  /** returns next pseudorandom box: can cross the 180th meridian */
  static std::shared_ptr<Rectangle> nextBox();

  /** returns next pseudorandom box: does not cross the 180th meridian */
  static std::shared_ptr<Rectangle> nextBoxNotCrossingDateline();

  /** Makes an n-gon, centered at the provided lat/lon, and each vertex
   * approximately distanceMeters away from the center.
   *
   * Do not invoke me across the dateline or a pole!! */
  static std::shared_ptr<Polygon> createRegularPolygon(double centerLat,
                                                       double centerLon,
                                                       double radiusMeters,
                                                       int gons);

  /** returns next pseudorandom polygon */
  static std::shared_ptr<Polygon> nextPolygon();

private:
  static std::shared_ptr<Rectangle> nextBoxInternal(double lat0, double lat1,
                                                    double lon0, double lon1,
                                                    bool canCrossDateLine);

  static std::shared_ptr<Polygon> boxPolygon(std::shared_ptr<Rectangle> box);

  static std::shared_ptr<Polygon>
  trianglePolygon(std::shared_ptr<Rectangle> box);

  static std::shared_ptr<Polygon> surpriseMePolygon();

  /** Keep it simple, we don't need to take arbitrary Random for geo tests */
  static std::shared_ptr<Random> random();

  /**
   * Returns svg of polygon for debugging.
   * <p>
   * You can pass any number of objects:
   * Polygon: polygon with optional holes
   * Polygon[]: arrays of polygons for convenience
   * Rectangle: for a box
   * double[2]: as latitude,longitude for a point
   * <p>
   * At least one object must be a polygon. The viewBox is formed around all
   * polygons found in the arguments.
   */
public:
  static std::wstring toSVG(std::deque<std::any> &objects);

  /**
   * Simple slow point in polygon check (for testing)
   */
  // direct port of PNPOLY C code
  // (https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html) this
  // allows us to improve the code yet still ensure we have its properties it is
  // under the BSD license
  // (https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html#License%20to%20Use)
  //
  // Copyright (c) 1970-2003, Wm. Randolph Franklin
  //
  // Permission is hereby granted, free of charge, to any person obtaining a
  // copy of this software and associated documentation files (the "Software"),
  // to deal in the Software without restriction, including without limitation
  // the rights to use, copy, modify, merge, publish, distribute, sublicense,
  // and/or sell copies of the Software, and to permit persons to whom the
  // Software is furnished to do so, subject to the following conditions:
  //
  // 1. Redistributions of source code must retain the above copyright
  //    notice, this deque of conditions and the following disclaimers.
  // 2. Redistributions in binary form must reproduce the above copyright
  //    notice in the documentation and/or other materials provided with
  //    the distribution.
  // 3. The name of W. Randolph Franklin may not be used to endorse or
  //    promote products derived from this Software without specific
  //    prior written permission.
  //
  // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  // FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  // DEALINGS IN THE SOFTWARE.
  static bool containsSlowly(std::shared_ptr<Polygon> polygon, double latitude,
                             double longitude);
};

} // #include  "core/src/java/org/apache/lucene/geo/
