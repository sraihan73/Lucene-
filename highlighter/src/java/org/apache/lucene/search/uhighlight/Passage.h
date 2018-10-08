#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Represents a passage (typically a sentence of the document).
 * <p>
 * A passage contains {@link #getNumMatches} highlights from the query,
 * and the offsets and query terms that correspond with each match.
 *
 * @lucene.experimental
 */
class Passage : public std::enable_shared_from_this<Passage>
{
  GET_CLASS_NAME(Passage)
private:
  int startOffset = -1;
  int endOffset = -1;
  float score = 0.0f;

  std::deque<int> matchStarts = std::deque<int>(8);
  std::deque<int> matchEnds = std::deque<int>(8);
  std::deque<std::shared_ptr<BytesRef>> matchTerms =
      std::deque<std::shared_ptr<BytesRef>>(8);
  std::deque<int> matchTermFreqInDoc = std::deque<int>(8);
  int numMatches = 0;

  /** @lucene.internal */
public:
  virtual void addMatch(int startOffset, int endOffset,
                        std::shared_ptr<BytesRef> term, int termFreqInDoc);

  /** @lucene.internal */
  virtual void reset();

  /** For debugging.  ex:
   * Passage[0-22]{yin[0-3],yang[4-8],yin[10-13]}score=2.4964213 */
  virtual std::wstring toString();

  /**
   * Start offset of this passage.
   *
   * @return start index (inclusive) of the passage in the
   * original content: always &gt;= 0.
   */
  virtual int getStartOffset();

  /**
   * End offset of this passage.
   *
   * @return end index (exclusive) of the passage in the
   * original content: always &gt;= {@link #getStartOffset()}
   */
  virtual int getEndOffset();

  virtual int getLength();

  /**
   * Passage's score.
   */
  virtual float getScore();

  virtual void setScore(float score);

  /**
   * Number of term matches available in
   * {@link #getMatchStarts}, {@link #getMatchEnds},
   * {@link #getMatchTerms}
   */
  virtual int getNumMatches();

  /**
   * Start offsets of the term matches, in increasing order.
   * <p>
   * Only {@link #getNumMatches} are valid. Note that these
   * offsets are absolute (not relative to {@link #getStartOffset()}).
   */
  virtual std::deque<int> getMatchStarts();

  /**
   * End offsets of the term matches, corresponding with {@link
   * #getMatchStarts}. <p> Only {@link #getNumMatches} are valid. Note that its
   * possible that an end offset could exceed beyond the bounds of the passage
   * ({@link #getEndOffset()}), if the Analyzer produced a term which spans a
   * passage boundary.
   */
  virtual std::deque<int> getMatchEnds();

  /**
   * BytesRef (term text) of the matches, corresponding with {@link
   * #getMatchStarts()}. <p> Only {@link #getNumMatches()} are valid.
   */
  virtual std::deque<std::shared_ptr<BytesRef>> getMatchTerms();

  virtual std::deque<int> getMatchTermFreqsInDoc();

  /** @lucene.internal */
  virtual void setStartOffset(int startOffset);

  /** @lucene.internal */
  virtual void setEndOffset(int endOffset);
};

} // namespace org::apache::lucene::search::uhighlight
