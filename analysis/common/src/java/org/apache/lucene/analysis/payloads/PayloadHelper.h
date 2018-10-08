#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::analysis::payloads
{

/**
 * Utility methods for encoding payloads.
 *
 **/
class PayloadHelper : public std::enable_shared_from_this<PayloadHelper>
{
  GET_CLASS_NAME(PayloadHelper)

public:
  static std::deque<char> encodeFloat(float payload);

  static std::deque<char> encodeFloat(float payload, std::deque<char> &data,
                                       int offset);

  static std::deque<char> encodeInt(int payload);

  static std::deque<char> encodeInt(int payload, std::deque<char> &data,
                                     int offset);

  /**
   * @see #decodeFloat(byte[], int)
   * @see #encodeFloat(float)
   * @return the decoded float
   */
  static float decodeFloat(std::deque<char> &bytes);

  /**
   * Decode the payload that was encoded using {@link #encodeFloat(float)}.
   * NOTE: the length of the array must be at least offset + 4 long.
   * @param bytes The bytes to decode
   * @param offset The offset into the array.
   * @return The float that was encoded
   *
   * @see #encodeFloat(float)
   */
  static float decodeFloat(std::deque<char> &bytes, int offset);

  static int decodeInt(std::deque<char> &bytes, int offset);
};

} // namespace org::apache::lucene::analysis::payloads
