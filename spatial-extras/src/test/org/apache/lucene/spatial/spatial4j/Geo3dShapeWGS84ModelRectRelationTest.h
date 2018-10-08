#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
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
namespace org::apache::lucene::spatial::spatial4j
{

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;

class Geo3dShapeWGS84ModelRectRelationTest : public ShapeRectRelationTestCase
{
  GET_CLASS_NAME(Geo3dShapeWGS84ModelRectRelationTest)

public:
  std::shared_ptr<PlanetModel> planetModel = PlanetModel::WGS84;

  Geo3dShapeWGS84ModelRectRelationTest();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailure1()
  virtual void testFailure1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailure2()
  virtual void testFailure2();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailure3()
  virtual void testFailure3();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void pointBearingTest()
  virtual void pointBearingTest();

protected:
  std::shared_ptr<Geo3dShapeWGS84ModelRectRelationTest> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dShapeWGS84ModelRectRelationTest>(
        ShapeRectRelationTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::spatial4j