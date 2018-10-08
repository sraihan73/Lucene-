#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
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

/** Draws shapes on the earth surface and renders using the very cool
 * http://www.webglearth.org.
 *
 * Just instantiate this class, add the things you want plotted, and call {@link
 * #finish} to get the resulting HTML that you should save and load with a
 * browser. */

class EarthDebugger : public std::enable_shared_from_this<EarthDebugger>
{
  GET_CLASS_NAME(EarthDebugger)
public:
  const std::shared_ptr<StringBuilder> b = std::make_shared<StringBuilder>();

private:
  int nextShape = 0;
  bool finished = false;

public:
  EarthDebugger();

  EarthDebugger(double centerLat, double centerLon, double altitudeMeters);

  virtual void addPolygon(std::shared_ptr<Polygon> poly);

  virtual void addPolygon(std::shared_ptr<Polygon> poly,
                          const std::wstring &color);

private:
  static double MAX_KM_PER_STEP;

  // Web GL earth connects dots by tunneling under the earth, so we approximate
  // a great circle by sampling it, to minimize how deep in the earth each
  // segment tunnels:
  int getStepCount(double minLat, double maxLat, double minLon, double maxLon);

  // first point is inclusive, last point is exclusive!
  void drawSegment(double minLat, double maxLat, double minLon, double maxLon);

public:
  virtual void addRect(double minLat, double maxLat, double minLon,
                       double maxLon);

  virtual void addRect(double minLat, double maxLat, double minLon,
                       double maxLon, const std::wstring &color);

  /** Draws a line a fixed latitude, spanning the min/max longitude */
  virtual void addLatLine(double lat, double minLon, double maxLon);

  /** Draws a line a fixed longitude, spanning the min/max latitude */
  virtual void addLonLine(double minLat, double maxLat, double lon);

  virtual void addPoint(double lat, double lon);

  virtual void addCircle(double centerLat, double centerLon,
                         double radiusMeters, bool alsoAddBBox);

  virtual std::wstring finish();

private:
  static void inverseHaversin(std::shared_ptr<StringBuilder> b,
                              double centerLat, double centerLon,
                              double radiusMeters);
  // craziness for plotting stuff :)

  static double wrapLat(double lat);

  static double wrapLon(double lon);
};

} // #include  "core/src/java/org/apache/lucene/geo/
