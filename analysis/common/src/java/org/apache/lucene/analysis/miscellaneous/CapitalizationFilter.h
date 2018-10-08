#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <cctype>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * A filter to apply normal capitalization rules to Tokens.  It will make the
 * first letter capital and the rest lower case. <p> This filter is particularly
 * useful to build nice looking facet parameters.  This filter is not
 * appropriate if you intend to use a prefix query.
 */
class CapitalizationFilter final : public TokenFilter
{
  GET_CLASS_NAME(CapitalizationFilter)
public:
  static const int DEFAULT_MAX_WORD_COUNT = std::numeric_limits<int>::max();
  static const int DEFAULT_MAX_TOKEN_LENGTH = std::numeric_limits<int>::max();

private:
  const bool onlyFirstWord;
  const std::shared_ptr<CharArraySet> keep;
  const bool forceFirstLetter;
  const std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix;

  const int minWordLength;
  const int maxWordCount;
  const int maxTokenLength;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Creates a CapitalizationFilter with the default parameters.
   * <p>
   * Calls {@link #CapitalizationFilter(TokenStream, bool, CharArraySet,
   * bool, std::deque, int, int, int) CapitalizationFilter(in, true, null,
   * true, null, 0, DEFAULT_MAX_WORD_COUNT, DEFAULT_MAX_TOKEN_LENGTH)}
   */
public:
  CapitalizationFilter(std::shared_ptr<TokenStream> in_);

  /**
   * Creates a CapitalizationFilter with the specified parameters.
   * @param in input tokenstream
   * @param onlyFirstWord should each word be capitalized or all of the words?
   * @param keep a keep word deque.  Each word that should be kept separated by
   * whitespace.
   * @param forceFirstLetter Force the first letter to be capitalized even if it
   * is in the keep deque.
   * @param okPrefix do not change word capitalization if a word begins with
   * something in this deque.
   * @param minWordLength how long the word needs to be to get capitalization
   * applied.  If the minWordLength is 3, "and" &gt; "And" but "or" stays "or".
   * @param maxWordCount if the token contains more then maxWordCount words, the
   * capitalization is assumed to be correct.
   * @param maxTokenLength ???
   */
  CapitalizationFilter(
      std::shared_ptr<TokenStream> in_, bool onlyFirstWord,
      std::shared_ptr<CharArraySet> keep, bool forceFirstLetter,
      std::shared_ptr<std::deque<std::deque<wchar_t>>> okPrefix,
      int minWordLength, int maxWordCount, int maxTokenLength);

  bool incrementToken()  override;

private:
  void processWord(std::deque<wchar_t> &buffer, int offset, int length,
                   int wordCount);

protected:
  std::shared_ptr<CapitalizationFilter> shared_from_this()
  {
    return std::static_pointer_cast<CapitalizationFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
