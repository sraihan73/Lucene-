#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * This class converts alphabetic, numeric, and symbolic Unicode characters
 * which are not in the first 127 ASCII characters (the "Basic Latin" Unicode
 * block) into their ASCII equivalents, if one exists.
 *
 * Characters from the following Unicode blocks are converted; however, only
 * those characters with reasonable ASCII alternatives are converted:
 *
 * <ul>
 *   <li>C1 Controls and Latin-1 Supplement: <a
 * href="http://www.unicode.org/charts/PDF/U0080.pdf">http://www.unicode.org/charts/PDF/U0080.pdf</a>
 *   <li>Latin Extended-A: <a
 * href="http://www.unicode.org/charts/PDF/U0100.pdf">http://www.unicode.org/charts/PDF/U0100.pdf</a>
 *   <li>Latin Extended-B: <a
 * href="http://www.unicode.org/charts/PDF/U0180.pdf">http://www.unicode.org/charts/PDF/U0180.pdf</a>
 *   <li>Latin Extended Additional: <a
 * href="http://www.unicode.org/charts/PDF/U1E00.pdf">http://www.unicode.org/charts/PDF/U1E00.pdf</a>
 *   <li>Latin Extended-C: <a
 * href="http://www.unicode.org/charts/PDF/U2C60.pdf">http://www.unicode.org/charts/PDF/U2C60.pdf</a>
 *   <li>Latin Extended-D: <a
 * href="http://www.unicode.org/charts/PDF/UA720.pdf">http://www.unicode.org/charts/PDF/UA720.pdf</a>
 *   <li>IPA Extensions: <a
 * href="http://www.unicode.org/charts/PDF/U0250.pdf">http://www.unicode.org/charts/PDF/U0250.pdf</a>
 *   <li>Phonetic Extensions: <a
 * href="http://www.unicode.org/charts/PDF/U1D00.pdf">http://www.unicode.org/charts/PDF/U1D00.pdf</a>
 *   <li>Phonetic Extensions Supplement: <a
 * href="http://www.unicode.org/charts/PDF/U1D80.pdf">http://www.unicode.org/charts/PDF/U1D80.pdf</a>
 *   <li>General Punctuation: <a
 * href="http://www.unicode.org/charts/PDF/U2000.pdf">http://www.unicode.org/charts/PDF/U2000.pdf</a>
 *   <li>Superscripts and Subscripts: <a
 * href="http://www.unicode.org/charts/PDF/U2070.pdf">http://www.unicode.org/charts/PDF/U2070.pdf</a>
 *   <li>Enclosed Alphanumerics: <a
 * href="http://www.unicode.org/charts/PDF/U2460.pdf">http://www.unicode.org/charts/PDF/U2460.pdf</a>
 *   <li>Dingbats: <a
 * href="http://www.unicode.org/charts/PDF/U2700.pdf">http://www.unicode.org/charts/PDF/U2700.pdf</a>
 *   <li>Supplemental Punctuation: <a
 * href="http://www.unicode.org/charts/PDF/U2E00.pdf">http://www.unicode.org/charts/PDF/U2E00.pdf</a>
 *   <li>Alphabetic Presentation Forms: <a
 * href="http://www.unicode.org/charts/PDF/UFB00.pdf">http://www.unicode.org/charts/PDF/UFB00.pdf</a>
 *   <li>Halfwidth and Fullwidth Forms: <a
 * href="http://www.unicode.org/charts/PDF/UFF00.pdf">http://www.unicode.org/charts/PDF/UFF00.pdf</a>
 * </ul>
 *
 * See: <a
 * href="http://en.wikipedia.org/wiki/Latin_characters_in_Unicode">http://en.wikipedia.org/wiki/Latin_characters_in_Unicode</a>
 *
 * For example, '&agrave;' will be replaced by 'a'.
 */
class ASCIIFoldingFilter final : public TokenFilter
{
  GET_CLASS_NAME(ASCIIFoldingFilter)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttr =
      addAttribute(PositionIncrementAttribute::typeid);
  const bool preserveOriginal;
  std::deque<wchar_t> output = std::deque<wchar_t>(512);
  int outputPos = 0;
  std::shared_ptr<State> state;

public:
  ASCIIFoldingFilter(std::shared_ptr<TokenStream> input);

  /**
   * Create a new {@link ASCIIFoldingFilter}.
   *
   * @param input
   *          TokenStream to filter
   * @param preserveOriginal
   *          should the original tokens be kept on the input stream with a 0
   *position increment from the folded tokens?
   **/
  ASCIIFoldingFilter(std::shared_ptr<TokenStream> input, bool preserveOriginal);

  /**
   * Does the filter preserve the original tokens?
   */
  bool isPreserveOriginal();

  bool incrementToken()  override;

  void reset()  override;

  /**
   * Converts characters above ASCII to their ASCII equivalents.  For example,
   * accents are removed from accented characters.
   * @param input The string to fold
   * @param length The number of characters in the input string
   */
  void foldToASCII(std::deque<wchar_t> &input, int length);

  /**
   * Check if foldToASCII generated a different token.
   * @param input original term
   * @param inputLength length of the original term
   * @return true if foldToASCII generated a different token
   */
private:
  bool needToPreserve(std::deque<wchar_t> &input, int inputLength);

  /**
   * Converts characters above ASCII to their ASCII equivalents.  For example,
   * accents are removed from accented characters.
   * @param input     The characters to fold
   * @param inputPos  Index of the first character to fold
   * @param output    The result of the folding. Should be of size &gt;= {@code
   * length * 4}.
   * @param outputPos Index of output where to put the result of the folding
   * @param length    The number of characters to fold
   * @return length of output
   * @lucene.internal
   */
public:
  static int foldToASCII(std::deque<wchar_t> &input, int inputPos,
                         std::deque<wchar_t> &output, int outputPos,
                         int length);

protected:
  std::shared_ptr<ASCIIFoldingFilter> shared_from_this()
  {
    return std::static_pointer_cast<ASCIIFoldingFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
