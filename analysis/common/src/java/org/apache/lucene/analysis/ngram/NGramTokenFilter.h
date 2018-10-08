#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
namespace org::apache::lucene::analysis::ngram
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Tokenizes the input into n-grams of the given size(s).
 * As of Lucene 4.4, this token filter:<ul>
 * <li>handles supplementary characters correctly,</li>
 * <li>emits all n-grams for the same token at the same position,</li>
 * <li>does not modify offsets,</li>
 * <li>sorts n-grams by their offset in the original token first, then
 * increasing length (meaning that "abc" will give "a", "ab", "abc", "b", "bc",
 * "c").</li></ul>
 * <p>If you were using this {@link TokenFilter} to perform partial
 * highlighting, this won't work anymore since this filter doesn't update
 * offsets. You should modify your analysis chain to use {@link NGramTokenizer},
 * and potentially override {@link NGramTokenizer#isTokenChar(int)} to perform
 * pre-tokenization.
 */
class NGramTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(NGramTokenFilter)
  /**
   * @deprecated since 7.4 - this value will be required.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final int DEFAULT_MIN_NGRAM_SIZE =
  // 1;
  static constexpr int DEFAULT_MIN_NGRAM_SIZE = 1;

  /**
   * @deprecated since 7.4 - this value will be required.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final int DEFAULT_MAX_NGRAM_SIZE =
  // 2;
  static constexpr int DEFAULT_MAX_NGRAM_SIZE = 2;
  static constexpr bool DEFAULT_PRESERVE_ORIGINAL = false;

private:
  const int minGram;
  const int maxGram;
  const bool preserveOriginal;

  std::deque<wchar_t> curTermBuffer;
  int curTermLength = 0;
  int curTermCodePointCount = 0;
  int curGramSize = 0;
  int curPos = 0;
  int curPosIncr = 0;
  std::shared_ptr<State> state;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  /**
   * Creates an NGramTokenFilter that, for a given input term, produces all
   * contained n-grams with lengths &gt;= minGram and &lt;= maxGram. Will
   * optionally preserve the original term when its length is outside of the
   * defined range.
   *
   * Note: Care must be taken when choosing minGram and maxGram; depending
   * on the input token size, this filter potentially produces a huge number
   * of terms.
   *
   * @param input {@link TokenStream} holding the input to be tokenized
   * @param minGram the minimum length of the generated n-grams
   * @param maxGram the maximum length of the generated n-grams
   * @param preserveOriginal Whether or not to keep the original term when it
   * is shorter than minGram or longer than maxGram
   */
public:
  NGramTokenFilter(std::shared_ptr<TokenStream> input, int minGram, int maxGram,
                   bool preserveOriginal);

  /**
   * Creates an NGramTokenFilter that produces n-grams of the indicated size.
   *
   * @param input {@link TokenStream} holding the input to be tokenized
   * @param gramSize the size of n-grams to generate.
   */
  NGramTokenFilter(std::shared_ptr<TokenStream> input, int gramSize);

  /**
   * Creates an NGramTokenFilter that, for a given input term, produces all
   * contained n-grams with lengths &gt;= minGram and &lt;= maxGram.
   *
   * <p>
   * Behaves the same as
   * {@link #NGramTokenFilter(TokenStream, int, int, bool)
   * NGramTokenFilter(input, minGram, maxGram, false)}
   *
   * @param input {@link TokenStream} holding the input to be tokenized
   * @param minGram the minimum length of the generated n-grams
   * @param maxGram the maximum length of the generated n-grams
   *
   * @deprecated since 7.4. Use
   * {@link #NGramTokenFilter(TokenStream, int, int, bool)} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public
  // NGramTokenFilter(org.apache.lucene.analysis.TokenStream input, int minGram,
  // int maxGram)
  NGramTokenFilter(std::shared_ptr<TokenStream> input, int minGram,
                   int maxGram);

  /**
   * Creates NGramTokenFilter with default min and max n-grams.
   *
   * <p>
   * Behaves the same as
   * {@link #NGramTokenFilter(TokenStream, int, int, bool)
   * NGramTokenFilter(input, 1, 2, false)}
   *
   * @param input {@link TokenStream} holding the input to be tokenized
   * @deprecated since 7.4. Use
   * {@link #NGramTokenFilter(TokenStream, int, int, bool)} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public
  // NGramTokenFilter(org.apache.lucene.analysis.TokenStream input)
  NGramTokenFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override final;

  void reset()  override;

  void end()  override;

protected:
  std::shared_ptr<NGramTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<NGramTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ngram
