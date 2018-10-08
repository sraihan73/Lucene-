#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"

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

namespace org::apache::lucene::search
{

using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * Base class for exact and sloppy phrase matching
 *
 * To find matches on a document, first advance {@link #approximation} to the
 * relevant document, then call {@link #reset()}.  Clients can then call
 * {@link #nextMatch()} to iterate over the matches
 */
class PhraseMatcher : public std::enable_shared_from_this<PhraseMatcher>
{
  GET_CLASS_NAME(PhraseMatcher)

protected:
  const std::shared_ptr<DocIdSetIterator> approximation;

private:
  const float matchCost;

public:
  PhraseMatcher(std::shared_ptr<DocIdSetIterator> approximation,
                float matchCost);

  /**
   * An upper bound on the number of possible matches on this document
   */
  virtual float maxFreq() = 0;

  /**
   * Called after {@link #approximation} has been advanced
   */
  virtual void reset() = 0;

  /**
   * Find the next match on the current document, returning {@code false} if
   * there are none.
   */
  virtual bool nextMatch() = 0;

  /**
   * The slop-adjusted weight of the current match
   *
   * The sum of the slop-adjusted weights is used as the freq for scoring
   */
  virtual float
  sloppyWeight(std::shared_ptr<Similarity::SimScorer> simScorer) = 0;

  /**
   * The start position of the current match
   */
  virtual int startPosition() = 0;

  /**
   * The end position of the current match
   */
  virtual int endPosition() = 0;

  /**
   * The start offset of the current match
   */
  virtual int startOffset() = 0;

  /**
   * The end offset of the current match
   */
  virtual int endOffset() = 0;

  /**
   * An estimate of the average cost of finding all matches on a document
   *
   * @see TwoPhaseIterator#matchCost()
   */
  virtual float getMatchCost();
};

} // #include  "core/src/java/org/apache/lucene/search/
