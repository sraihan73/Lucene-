#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <memory>

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
namespace org::apache::lucene::util
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.cos;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.asin;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.haversinMeters;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.haversinSortKey;

class TestSloppyMath : public LuceneTestCase
{
  GET_CLASS_NAME(TestSloppyMath)
  // accuracy for cos()
public:
  static double COS_DELTA;
  // accuracy for asin()
  static double ASIN_DELTA;
  // accuracy for haversinMeters()
  static double HAVERSIN_DELTA;
  // accuracy for haversinMeters() for "reasonable" distances (< 1000km)
  static double REASONABLE_HAVERSIN_DELTA;

  virtual void testCos();

  virtual void testAsin();

  virtual void testHaversin();

  /** Test this method sorts the same way as real haversin */
  virtual void testHaversinSortKey();

  virtual void testHaversinFromSortKey();

  virtual void testAgainstSlowVersion();

  /**
   * Step across the whole world to find huge absolute errors.
   * Don't rely on random number generator to pick these massive distances. */
  virtual void testAcrossWholeWorldSteps();

  virtual void testAgainstSlowVersionReasonable();

  // simple incorporation of the wikipedia formula
private:
  static double slowHaversin(double lat1, double lon1, double lat2,
                             double lon2);

protected:
  std::shared_ptr<TestSloppyMath> shared_from_this()
  {
    return std::static_pointer_cast<TestSloppyMath>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
