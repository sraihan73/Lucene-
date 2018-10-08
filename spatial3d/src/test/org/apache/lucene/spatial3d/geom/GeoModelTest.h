#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

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
namespace org::apache::lucene::spatial3d::geom
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertTrue;

/**
 * Test basic plane functionality.
 */
class GeoModelTest : public std::enable_shared_from_this<GeoModelTest>
{
  GET_CLASS_NAME(GeoModelTest)

protected:
  static const std::shared_ptr<PlanetModel> scaledModel;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicCircle()
  virtual void testBasicCircle();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicRectangle()
  virtual void testBasicRectangle();
};

} // namespace org::apache::lucene::spatial3d::geom
