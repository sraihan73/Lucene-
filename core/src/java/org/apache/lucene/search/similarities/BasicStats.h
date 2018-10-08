#pragma once
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
namespace org::apache::lucene::search::similarities
{

/**
 * Stores all statistics commonly used ranking methods.
 * @lucene.experimental
 */
class BasicStats : public Similarity::SimWeight
{
  GET_CLASS_NAME(BasicStats)
public:
  const std::wstring field;
  /** The number of documents. */
protected:
  int64_t numberOfDocuments = 0;
  /** The total number of tokens in the field. */
  int64_t numberOfFieldTokens = 0;
  /** The average field length. */
  float avgFieldLength = 0;
  /** The document frequency. */
  int64_t docFreq = 0;
  /** The total number of occurrences of this term across all documents. */
  int64_t totalTermFreq = 0;

  // -------------------------- Boost-related stuff --------------------------

  /** A query boost. Should be applied as a multiplicative factor to the score.
   */
  const float boost;

  /** Constructor. */
public:
  BasicStats(const std::wstring &field, float boost);

  // ------------------------- Getter/setter methods -------------------------

  /** Returns the number of documents. */
  virtual int64_t getNumberOfDocuments();

  /** Sets the number of documents. */
  virtual void setNumberOfDocuments(int64_t numberOfDocuments);

  /**
   * Returns the total number of tokens in the field.
   * @see Terms#getSumTotalTermFreq()
   */
  virtual int64_t getNumberOfFieldTokens();

  /**
   * Sets the total number of tokens in the field.
   * @see Terms#getSumTotalTermFreq()
   */
  virtual void setNumberOfFieldTokens(int64_t numberOfFieldTokens);

  /** Returns the average field length. */
  virtual float getAvgFieldLength();

  /** Sets the average field length. */
  virtual void setAvgFieldLength(float avgFieldLength);

  /** Returns the document frequency. */
  virtual int64_t getDocFreq();

  /** Sets the document frequency. */
  virtual void setDocFreq(int64_t docFreq);

  /** Returns the total number of occurrences of this term across all documents.
   */
  virtual int64_t getTotalTermFreq();

  /** Sets the total number of occurrences of this term across all documents. */
  virtual void setTotalTermFreq(int64_t totalTermFreq);

  /** Returns the total boost. */
  virtual float getBoost();

protected:
  std::shared_ptr<BasicStats> shared_from_this()
  {
    return std::static_pointer_cast<BasicStats>(
        Similarity.SimWeight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
