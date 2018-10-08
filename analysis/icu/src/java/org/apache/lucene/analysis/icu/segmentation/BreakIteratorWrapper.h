#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/CharArrayIterator.h"

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
namespace org::apache::lucene::analysis::icu::segmentation
{

using com::ibm::icu::text::RuleBasedBreakIterator;
using com::ibm::icu::text::UnicodeSet;

/**
 * Wraps RuleBasedBreakIterator, making object reuse convenient and
 * emitting a rule status for emoji sequences.
 * @lucene.experimental
 */
class BreakIteratorWrapper final
    : public std::enable_shared_from_this<BreakIteratorWrapper>
{
  GET_CLASS_NAME(BreakIteratorWrapper)
private:
  const std::shared_ptr<CharArrayIterator> textIterator =
      std::make_shared<CharArrayIterator>();
  const std::shared_ptr<RuleBasedBreakIterator> rbbi;
  std::deque<wchar_t> text;
  int start = 0;
  int status = 0;

public:
  BreakIteratorWrapper(std::shared_ptr<RuleBasedBreakIterator> rbbi);

  int current();

  int getRuleStatus();

  int next();

  /** Returns current rule status for the text between breaks. (determines token
   * type) */
private:
  int calcStatus(int current, int next);

  // See unicode doc L2/16-315 and also the RBBI rules for rationale.
  // we don't include regional indicators here, because they aren't ambiguous
  // for tagging, they need only be treated special for segmentation.
public:
  static const std::shared_ptr<UnicodeSet> EMOJI_RK;

  /** Returns true if the current text represents emoji character or sequence */
private:
  bool isEmoji(int current, int next);

public:
  void setText(std::deque<wchar_t> &text, int start, int length);
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
