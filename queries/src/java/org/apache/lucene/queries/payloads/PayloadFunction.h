#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
using Explanation = org::apache::lucene::search::Explanation;

/**
 * An abstract class that defines a way for PayloadScoreQuery instances to
 *transform the cumulative effects of payload scores for a document.
 *
 * @see org.apache.lucene.queries.payloads.PayloadScoreQuery for more
 *information
 *
 * @lucene.experimental This class and its derivations are experimental and
 *subject to change
 *
 **/
class PayloadFunction : public std::enable_shared_from_this<PayloadFunction>
{
  GET_CLASS_NAME(PayloadFunction)

  /**
   * Calculate the score up to this point for this doc and field
   * @param docId The current doc
   * @param field The field
   * @param start The start position of the matching Span
   * @param end The end position of the matching Span
   * @param numPayloadsSeen The number of payloads seen so far
   * @param currentScore The current score so far
   * @param currentPayloadScore The score for the current payload
   * @return The new current Score
   *
   * @see Spans
   */
public:
  virtual float currentScore(int docId, const std::wstring &field, int start,
                             int end, int numPayloadsSeen, float currentScore,
                             float currentPayloadScore) = 0;

  /**
   * Calculate the final score for all the payloads seen so far for this
   * doc/field
   * @param docId The current doc
   * @param field The current field
   * @param numPayloadsSeen The total number of payloads seen on this document
   * @param payloadScore The raw score for those payloads
   * @return The final score for the payloads
   */
  virtual float docScore(int docId, const std::wstring &field,
                         int numPayloadsSeen, float payloadScore) = 0;

  virtual std::shared_ptr<Explanation> explain(int docId,
                                               const std::wstring &field,
                                               int numPayloadsSeen,
                                               float payloadScore);

  int hashCode() = 0;
  override

      bool
      equals(std::any o) = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/queries/payloads/
