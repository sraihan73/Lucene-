#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

class TestPolygon : public LuceneTestCase
{
  GET_CLASS_NAME(TestPolygon)

  /** null polyLats not allowed */
public:
  virtual void testPolygonNullPolyLats();

  /** null polyLons not allowed */
  virtual void testPolygonNullPolyLons();

  /** polygon needs at least 3 vertices */
  virtual void testPolygonLine();

  /** polygon needs same number of latitudes as longitudes */
  virtual void testPolygonBogus();

  /** polygon must be closed */
  virtual void testPolygonNotClosed();

  virtual void testGeoJSONPolygon() ;

  virtual void testGeoJSONPolygonWithHole() ;

  // a MultiPolygon returns multiple Polygons
  virtual void testGeoJSONMultiPolygon() ;

  // make sure type can appear last (JSON allows arbitrary key/value order for
  // objects)
  virtual void testGeoJSONTypeComesLast() ;

  // make sure Polygon inside a type: Feature also works
  virtual void testGeoJSONPolygonFeature() ;

  // make sure MultiPolygon inside a type: Feature also works
  virtual void testGeoJSONMultiPolygonFeature() ;

  // FeatureCollection with one geometry is allowed:
  virtual void
  testGeoJSONFeatureCollectionWithSinglePolygon() ;

  // stuff after the object is not allowed
  virtual void testIllegalGeoJSONExtraCrapAtEnd() ;

  virtual void testIllegalGeoJSONLinkedCRS() ;

  // FeatureCollection with more than one geometry is not supported:
  virtual void testIllegalGeoJSONMultipleFeatures() ;

protected:
  std::shared_ptr<TestPolygon> shared_from_this()
  {
    return std::static_pointer_cast<TestPolygon>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::geo
