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
namespace org::apache::lucene::analysis::icu::segmentation
{

using com::ibm::icu::text::RuleBasedBreakIterator;

/**
 * Class that allows for tailored Unicode Text Segmentation on
 * a per-writing system basis.
 * @lucene.experimental
 */
class ICUTokenizerConfig
    : public std::enable_shared_from_this<ICUTokenizerConfig>
{
  GET_CLASS_NAME(ICUTokenizerConfig)
  /** Rule status for emoji sequences */
public:
  static constexpr int EMOJI_SEQUENCE_STATUS = 299;

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
  ICUTokenizerConfig();
  /** Return a breakiterator capable of processing a given script. */
  virtual std::shared_ptr<RuleBasedBreakIterator>
  getBreakIterator(int script) = 0;
  /** Return a token type value for a given script and BreakIterator
   *  rule status. */
  virtual std::wstring getType(int script, int ruleStatus) = 0;
  /** true if Han, Hiragana, and Katakana scripts should all be returned as
   * Japanese */
  virtual bool combineCJ() = 0;
};

} // namespace org::apache::lucene::analysis::icu::segmentation
