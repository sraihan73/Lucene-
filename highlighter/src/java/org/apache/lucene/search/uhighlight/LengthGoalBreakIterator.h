#pragma once
#include "stringhelper.h"
#include <any>
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

namespace org::apache::lucene::search::uhighlight
{

/**
 * Wraps another {@link BreakIterator} to skip past breaks that would result in
 * passages that are too short.  It's still possible to get a short passage but
 * only at the very end of the input text. <p> Important: This is not a general
 * purpose {@link BreakIterator}; it's only designed to work in a way compatible
 * with the {@link UnifiedHighlighter}.  Some assumptions are checked with Java
 * assertions.
 *
 * @lucene.experimental
 */
class LengthGoalBreakIterator : public BreakIterator
{
  GET_CLASS_NAME(LengthGoalBreakIterator)

private:
  const std::shared_ptr<BreakIterator> baseIter;
  const int lengthGoal;
  const bool isMinimumLength; // if false then is "closest to" length

  /** Breaks will be at least {@code minLength} apart (to the extent possible).
   */
public:
  static std::shared_ptr<LengthGoalBreakIterator>
  createMinLength(std::shared_ptr<BreakIterator> baseIter, int minLength);

  /** Breaks will be on average {@code targetLength} apart; the closest break to
   * this target (before or after) is chosen. */
  static std::shared_ptr<LengthGoalBreakIterator>
  createClosestToLength(std::shared_ptr<BreakIterator> baseIter,
                        int targetLength);

private:
  LengthGoalBreakIterator(std::shared_ptr<BreakIterator> baseIter,
                          int lengthGoal, bool isMinimumLength);

  // note: the only methods that will get called are setText(txt), getText(),
  // getSummaryPassagesNoHighlight: current(), first(), next()
  // highlightOffsetsEnums: preceding(int), and following(int)
  //   Nonetheless we make some attempt to implement the rest; mostly
  //   delegating.

public:
  virtual std::wstring toString();

  std::any clone() override;

  std::shared_ptr<CharacterIterator> getText() override;

  void setText(const std::wstring &newText) override;

  void setText(std::shared_ptr<CharacterIterator> newText) override;

  int current() override;

  int first() override;

  int last() override;

  int next(int n) override;

  // called by getSummaryPassagesNoHighlight to generate default summary.
  int next() override;

  int previous() override;

  // called while the current position is the start of a new passage; find end
  // of passage
  int following(int followingIdx) override;

private:
  int moveToBreak(int idx);

  // called at start of new Passage given first word start offset
public:
  int preceding(int offset) override;

  bool isBoundary(int offset) override;

protected:
  std::shared_ptr<LengthGoalBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<LengthGoalBreakIterator>(
        java.text.BreakIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
