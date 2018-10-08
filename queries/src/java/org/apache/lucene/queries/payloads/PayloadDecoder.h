#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

namespace org::apache::lucene::queries::payloads
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Defines a way of converting payloads to float values, for use by {@link
 * PayloadScoreQuery}
 */
class PayloadDecoder
{
  GET_CLASS_NAME(PayloadDecoder)

  /**
   * Compute a float value based on the doc, position and payload
   * @deprecated Use {@link #computePayloadFactor(BytesRef)} - doc and position
   * can be taken into account in {@link PayloadFunction#currentScore(int,
   * std::wstring, int, int, int, float, float)}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated default float computePayloadFactor(int docID,
  // int startPosition, int endPosition, org.apache.lucene.util.BytesRef payload)
  // C++ TODO: There is no equivalent in C++ to Java default interface methods:
  //      default float computePayloadFactor(int docID, int startPosition, int
  //      endPosition, org.apache.lucene.util.BytesRef payload)
  //  {
  //    return computePayloadFactor(payload);
  //  }

  /**
   * Compute a float value for the given payload
   */
public:
  virtual float computePayloadFactor(std::shared_ptr<BytesRef> payload) = 0;

  /**
   * A {@link PayloadDecoder} that interprets the bytes of a payload as a float
   */
  static const std::shared_ptr<PayloadDecoder> FLOAT_DECODER;
};

} // #include  "core/src/java/org/apache/lucene/queries/payloads/
