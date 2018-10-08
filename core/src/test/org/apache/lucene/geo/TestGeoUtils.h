#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include <cmath>
#include <iostream>
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests class for methods in GeoUtils
 *
 * @lucene.experimental
 */
class TestGeoUtils : public LuceneTestCase
{
  GET_CLASS_NAME(TestGeoUtils)

  // We rely heavily on GeoUtils.circleToBBox so we test it here:
public:
  virtual void testRandomCircleToBBox() ;

  // similar to testRandomCircleToBBox, but different, less evil, maybe simpler
  virtual void testBoundingBoxOpto();

  // test we can use haversinSortKey() for distance queries.
  virtual void testHaversinOpto();

  /** Test infinite radius covers whole earth */
  virtual void testInfiniteRect();

  virtual void testAxisLat();

  // TODO: does not really belong here, but we test it like this for now
  // we can make a fake IndexReader to send boxes directly to Point visitors
  // instead?
  virtual void testCircleOpto() ;

  static double randomInRange(double min, double max);

  static bool isDisjoint(double centerLat, double centerLon, double radius,
                         double axisLat, double latMin, double latMax,
                         double lonMin, double lonMax);

  virtual void testWithin90LonDegrees();

protected:
  std::shared_ptr<TestGeoUtils> shared_from_this()
  {
    return std::static_pointer_cast<TestGeoUtils>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/geo/
