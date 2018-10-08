#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::uhighlight
{
class Passage;
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
namespace org::apache::lucene::search::uhighlight
{

/**
 * Ranks passages found by {@link UnifiedHighlighter}.
 * <p>
 * Each passage is scored as a miniature document within the document.
 * The final score is computed as {@link #norm} * &sum; ({@link #weight} *
 * {@link #tf}). The default implementation is {@link #norm} * BM25.
 *
 * @lucene.experimental
 */
class PassageScorer : public std::enable_shared_from_this<PassageScorer>
{
  GET_CLASS_NAME(PassageScorer)

  // TODO: this formula is completely made up. It might not provide relevant
  // snippets!

  /**
   * BM25 k1 parameter, controls term frequency normalization
   */
public:
  const float k1;
  /**
   * BM25 b parameter, controls length normalization.
   */
  const float b;
  /**
   * A pivot used for length normalization.
   */
  const float pivot;

  /**
   * Creates PassageScorer with these default values:
   * <ul>
   * <li>{@code k1 = 1.2},
   * <li>{@code b = 0.75}.
   * <li>{@code pivot = 87}
   * </ul>
   */
  PassageScorer();

  /**
   * Creates PassageScorer with specified scoring parameters
   *
   * @param k1    Controls non-linear term frequency normalization (saturation).
   * @param b     Controls to what degree passage length normalizes tf values.
   * @param pivot Pivot value for length normalization (some rough idea of
   * average sentence length in characters).
   */
  PassageScorer(float k1, float b, float pivot);

  /**
   * Computes term importance, given its in-document statistics.
   *
   * @param contentLength length of document in characters
   * @param totalTermFreq number of time term occurs in document
   * @return term importance
   */
  virtual float weight(int contentLength, int totalTermFreq);

  /**
   * Computes term weight, given the frequency within the passage
   * and the passage's length.
   *
   * @param freq       number of occurrences of within this passage
   * @param passageLen length of the passage in characters.
   * @return term weight
   */
  virtual float tf(int freq, int passageLen);

  /**
   * Normalize a passage according to its position in the document.
   * <p>
   * Typically passages towards the beginning of the document are
   * more useful for summarizing the contents.
   * <p>
   * The default implementation is <code>1 + 1/log(pivot + passageStart)</code>
   *
   * @param passageStart start offset of the passage
   * @return a boost value multiplied into the passage's core.
   */
  virtual float norm(int passageStart);

  virtual float score(std::shared_ptr<Passage> passage, int contentLength);
};

} // namespace org::apache::lucene::search::uhighlight
