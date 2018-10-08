#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_INCL;

/**
 Tests methods in {@link GeoEncodingUtils}
 */
class TestGeoEncodingUtils : public LuceneTestCase
{
  GET_CLASS_NAME(TestGeoEncodingUtils)

  /**
   * step through some integers, ensuring they decode to their expected double
   * values. double values start at -90 and increase by LATITUDE_DECODE for each
   * integer. check edge cases within the double range and random doubles within
   * the range too.
   */
public:
  virtual void testLatitudeQuantization() ;

  /**
   * step through some integers, ensuring they decode to their expected double
   * values. double values start at -180 and increase by LONGITUDE_DECODE for
   * each integer. check edge cases within the double range and a random doubles
   * within the range too.
   */
  virtual void testLongitudeQuantization() ;

  // check edge/interesting cases explicitly
  virtual void testEncodeEdgeCases();

protected:
  std::shared_ptr<TestGeoEncodingUtils> shared_from_this()
  {
    return std::static_pointer_cast<TestGeoEncodingUtils>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::geo
