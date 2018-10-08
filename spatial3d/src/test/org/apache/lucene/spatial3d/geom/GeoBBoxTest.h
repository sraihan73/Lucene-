#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
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
namespace org::apache::lucene::spatial3d::geom
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertTrue;

class GeoBBoxTest : public std::enable_shared_from_this<GeoBBoxTest>
{
  GET_CLASS_NAME(GeoBBoxTest)

protected:
  static const double DEGREES_TO_RADIANS;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBBoxDegenerate()
  virtual void testBBoxDegenerate();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBBoxPointWithin()
  virtual void testBBoxPointWithin();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBBoxExpand()
  virtual void testBBoxExpand();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBBoxBounds()
  virtual void testBBoxBounds();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureCase1()
  virtual void testFailureCase1();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureCase2()
  virtual void testFailureCase2();
};

} // namespace org::apache::lucene::spatial3d::geom
