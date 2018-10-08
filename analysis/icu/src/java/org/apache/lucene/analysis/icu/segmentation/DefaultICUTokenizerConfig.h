#pragma once
#include "ICUTokenizerConfig.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;

using com::ibm::icu::text::BreakIterator;
using com::ibm::icu::text::RuleBasedBreakIterator;
using com::ibm::icu::util::ULocale;

/**
 * Default {@link ICUTokenizerConfig} that is generally applicable
 * to many languages.
 * <p>
 * Generally tokenizes Unicode text according to UAX#29
 * ({@link BreakIterator#getWordInstance(ULocale)
 * BreakIterator.getWordInstance(ULocale.ROOT)}), but with the following
 * tailorings: <ul> <li>Thai, Lao, Myanmar, Khmer, and CJK text is broken into
 * words with a dictionary.
 * </ul>
 * @lucene.experimental
 */
class DefaultICUTokenizerConfig : public ICUTokenizerConfig
{
  GET_CLASS_NAME(DefaultICUTokenizerConfig)
  /** Token type for words containing ideographic characters */
public:
  static const std::wstring WORD_IDEO;
  /** Token type for words containing Japanese hiragana */
  static const std::wstring WORD_HIRAGANA;
  /** Token type for words containing Japanese katakana */
  static const std::wstring WORD_KATAKANA;
  /** Token type for words containing Korean hangul  */
  static const std::wstring WORD_HANGUL;
  /** Token type for words that contain letters */
  static const std::wstring WORD_LETTER;
  /** Token type for words that appear to be numbers */
  static const std::wstring WORD_NUMBER;
  /** Token type for words that appear to be emoji sequences */
  static const std::wstring WORD_EMOJI;

  /*
   * the default breakiterators in use. these can be expensive to
   * instantiate, cheap to clone.
   */
  // we keep the cjk breaking separate, thats because it cannot be customized
  // (because dictionary is only triggered when kind = WORD, but kind = LINE by
  // default and we have no non-evil way to change it)
private:
  static const std::shared_ptr<BreakIterator> cjkBreakIterator;

  // TODO: if the wrong version of the ICU jar is used, loading these data files
  // may give a strange error. maybe add an explicit check?
  // http://icu-project.org/apiref/icu4j/com/ibm/icu/util/VersionInfo.html

  // the same as ROOT, except no dictionary segmentation for cjk
  static const std::shared_ptr<RuleBasedBreakIterator> defaultBreakIterator;
  static const std::shared_ptr<RuleBasedBreakIterator> myanmarSyllableIterator;

  // TODO: deprecate this bool? you only care if you are doing super-expert
  // stuff...
  const bool cjkAsWords;
  const bool myanmarAsWords;

  /**
   * Creates a new config. This object is lightweight, but the first
   * time the class is referenced, breakiterators will be initialized.
   * @param cjkAsWords true if cjk text should undergo dictionary-based
   * segmentation, otherwise text will be segmented according to UAX#29
   * defaults. If this is true, all Han+Hiragana+Katakana words will be tagged
   * as IDEOGRAPHIC.
   * @param myanmarAsWords true if Myanmar text should undergo dictionary-based
   * segmentation, otherwise it will be tokenized as syllables.
   */
public:
  DefaultICUTokenizerConfig(bool cjkAsWords, bool myanmarAsWords);

  bool combineCJ() override;

  std::shared_ptr<RuleBasedBreakIterator> getBreakIterator(int script) override;

  std::wstring getType(int script, int ruleStatus) override;

private:
  static std::shared_ptr<RuleBasedBreakIterator>
  readBreakIterator(const std::wstring &filename);

protected:
  std::shared_ptr<DefaultICUTokenizerConfig> shared_from_this()
  {
    return std::static_pointer_cast<DefaultICUTokenizerConfig>(
        ICUTokenizerConfig::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu::segmentation
