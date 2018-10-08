#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/RandomGeo3dShapeGenerator.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/RecursivePrefixTreeStrategy.h"

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

using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using RandomGeo3dShapeGenerator =
    org::apache::lucene::spatial3d::geom::RandomGeo3dShapeGenerator;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.locationtech.spatial4j.distance.DistanceUtils.DEGREES_TO_RADIANS;

class Geo3dRptTest : public RandomSpatialOpStrategyTestCase
{
  GET_CLASS_NAME(Geo3dRptTest)

private:
  std::shared_ptr<PlanetModel> planetModel;
  std::shared_ptr<RandomGeo3dShapeGenerator> shapeGenerator;
  std::shared_ptr<SpatialPrefixTree> grid;
  std::shared_ptr<RecursivePrefixTreeStrategy> rptStrategy;

  void setupGrid();

protected:
  virtual std::shared_ptr<RecursivePrefixTreeStrategy> newRPT();

private:
  void setupStrategy();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailure1() throws java.io.IOException
  virtual void testFailure1() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureLucene6535() throws
  // java.io.IOException
  virtual void testFailureLucene6535() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 30) public void testOperations()
  // throws java.io.IOException
  virtual void testOperations() ;

protected:
  std::shared_ptr<Shape> randomIndexedShape() override;

  std::shared_ptr<Shape> randomQueryShape() override;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOperationsFromFile() throws
  // java.io.IOException
  virtual void testOperationsFromFile() ;

  // TODO move to a new test class?
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWKT() throws Exception
  virtual void testWKT() ;

protected:
  std::shared_ptr<Geo3dRptTest> shared_from_this()
  {
    return std::static_pointer_cast<Geo3dRptTest>(
        org.apache.lucene.spatial.prefix
            .RandomSpatialOpStrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
