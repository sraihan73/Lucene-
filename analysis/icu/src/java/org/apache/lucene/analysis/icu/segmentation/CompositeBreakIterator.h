#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/ICUTokenizerConfig.h"

#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/BreakIteratorWrapper.h"
#include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/ScriptIterator.h"

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

using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UProperty;

/**
 * An internal BreakIterator for multilingual text, following recommendations
 * from: UAX #29: Unicode Text Segmentation. (http://unicode.org/reports/tr29/)
 * <p>
 * See http://unicode.org/reports/tr29/#Tailoring for the motivation of this
 * design.
 * <p>
 * Text is first divided into script boundaries. The processing is then
 * delegated to the appropriate break iterator for that specific script.
 * <p>
 * This break iterator also allows you to retrieve the ISO 15924 script code
 * associated with a piece of text.
 * <p>
 * See also UAX #29, UTR #24
 * @lucene.experimental
 */
class CompositeBreakIterator final
    : public std::enable_shared_from_this<CompositeBreakIterator>
{
  GET_CLASS_NAME(CompositeBreakIterator)
private:
  const std::shared_ptr<ICUTokenizerConfig> config;
  std::deque<std::shared_ptr<BreakIteratorWrapper>> const wordBreakers =
      std::deque<std::shared_ptr<BreakIteratorWrapper>>(
          1 + UCharacter::getIntPropertyMaxValue(UProperty::SCRIPT));

  std::shared_ptr<BreakIteratorWrapper> rbbi;
  const std::shared_ptr<ScriptIterator> scriptIterator;

  std::deque<wchar_t> text;

public:
  CompositeBreakIterator(std::shared_ptr<ICUTokenizerConfig> config);

  /**
   * Retrieve the next break position. If the RBBI range is exhausted within the
   * script boundary, examine the next script boundary.
   *
   * @return the next break position or BreakIterator.DONE
   */
  int next();

  /**
   * Retrieve the current break position.
   *
   * @return the current break position or BreakIterator.DONE
   */
  int current();

  /**
   * Retrieve the rule status code (token type) from the underlying break
   * iterator
   *
   * @return rule status code (see RuleBasedBreakIterator constants)
   */
  int getRuleStatus();

  /**
   * Retrieve the UScript script code for the current token. This code can be
   * decoded with UScript into a name or ISO 15924 code.
   *
   * @return UScript script code for the current token.
   */
  int getScriptCode();

  /**
   * Set a new region of text to be examined by this iterator
   *
   * @param text buffer of text
   * @param start offset into buffer
   * @param length maximum length to examine
   */
  void setText(std::deque<wchar_t> &text, int start, int length);

private:
  std::shared_ptr<BreakIteratorWrapper> getBreakIterator(int scriptCode);
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
