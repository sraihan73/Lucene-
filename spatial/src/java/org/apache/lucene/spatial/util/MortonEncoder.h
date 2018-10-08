#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::spatial::util
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;

/**
 * Quantizes lat/lon points and bit interleaves them into a binary morton code
 * in the range of 0x00000000... : 0xFFFFFFFF...
 * https://en.wikipedia.org/wiki/Z-order_curve
 *
 * This is useful for bitwise operations in raster space
 *
 * @lucene.experimental
 */
class MortonEncoder : public std::enable_shared_from_this<MortonEncoder>
{
  GET_CLASS_NAME(MortonEncoder)

private:
  MortonEncoder();

  /**
   * Main encoding method to quantize lat/lon points and bit interleave them
   * into a binary morton code in the range of 0x00000000... : 0xFFFFFFFF...
   *
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @return bit interleaved encoded values as a 64-bit {@code long}
   * @throws IllegalArgumentException if latitude or longitude is out of bounds
   */
public:
  static int64_t encode(double latitude, double longitude);

  /**
   * Quantizes lat/lon points and bit interleaves them into a sortable morton
   * code ranging from 0x00 : 0xFF...
   * https://en.wikipedia.org/wiki/Z-order_curve
   * This is useful for bitwise operations in raster space
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @return bit interleaved encoded values as a 64-bit {@code long}
   * @throws IllegalArgumentException if latitude or longitude is out of bounds
   */
  static int64_t encodeCeil(double latitude, double longitude);

  /** decode latitude value from morton encoded geo point */
  static double decodeLatitude(int64_t const hash);

  /** decode longitude value from morton encoded geo point */
  static double decodeLongitude(int64_t const hash);

  /** Converts a long value into a full 64 bit string (useful for debugging) */
  static std::wstring geoTermToString(int64_t term);
};

} // namespace org::apache::lucene::spatial::util
