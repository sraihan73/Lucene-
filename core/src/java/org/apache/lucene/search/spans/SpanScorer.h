#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"

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
namespace org::apache::lucene::search::spans
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * A basic {@link Scorer} over {@link Spans}.
 * @lucene.experimental
 */
class SpanScorer : public Scorer
{
  GET_CLASS_NAME(SpanScorer)

protected:
  const std::shared_ptr<Spans> spans;
  const std::shared_ptr<Similarity::SimScorer> docScorer;

  /** accumulated sloppy freq (computed in setFreqCurrentDoc) */
private:
  float freq = 0;
  /** number of matches (computed in setFreqCurrentDoc) */
  int numMatches = 0;
  int lastScoredDoc = -1; // last doc we called setFreqCurrentDoc() for

  /** Sole constructor. */
public:
  SpanScorer(std::shared_ptr<SpanWeight> weight, std::shared_ptr<Spans> spans,
             std::shared_ptr<Similarity::SimScorer> docScorer);

  /** return the Spans for this Scorer **/
  virtual std::shared_ptr<Spans> getSpans();

  int docID() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

  /**
   * Score the current doc. The default implementation scores the doc
   * with the similarity using the slop-adjusted {@link #freq}.
   */
protected:
  virtual float scoreCurrentDoc() ;

  /**
   * Sets {@link #freq} and {@link #numMatches} for the current document.
   * <p>
   * This will be called at most once per document.
   */
  void setFreqCurrentDoc() ;

  /**
   * Ensure setFreqCurrentDoc is called, if not already called for the current
   * doc.
   */
private:
  void ensureFreq() ;

public:
  float score()  override final;

  /** Returns the intermediate "sloppy freq" adjusted for edit distance
   *  @lucene.internal */
  float sloppyFreq() ;

protected:
  std::shared_ptr<SpanScorer> shared_from_this()
  {
    return std::static_pointer_cast<SpanScorer>(
        org.apache.lucene.search.Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
