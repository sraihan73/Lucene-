#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
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
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
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
namespace org::apache::lucene::analysis::commongrams
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/*
 * TODO: Consider implementing https://issues.apache.org/jira/browse/LUCENE-1688
 * changes to stop deque and associated constructors
 */

/**
 * Construct bigrams for frequently occurring terms while indexing. Single terms
 * are still indexed too, with bigrams overlaid. This is achieved through the
 * use of {@link PositionIncrementAttribute#setPositionIncrement(int)}. Bigrams
 * have a type of {@link #GRAM_TYPE} Example: <ul> <li>input:"the quick brown
 * fox"</li> <li>output:|"the","the-quick"|"brown"|"fox"|</li> <li>"the-quick"
 * has a position increment of 0 so it is in the same position as "the"
 * "the-quick" has a term.type() of "gram"</li>
 *
 * </ul>
 */

/*
 * Constructors and makeCommonSet based on similar code in StopFilter
 */
class CommonGramsFilter final : public TokenFilter
{
  GET_CLASS_NAME(CommonGramsFilter)

public:
  static const std::wstring GRAM_TYPE;

private:
  static constexpr wchar_t SEPARATOR = L'_';

  const std::shared_ptr<CharArraySet> commonWords;

  const std::shared_ptr<StringBuilder> buffer =
      std::make_shared<StringBuilder>();

  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttribute =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAttribute =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAttribute =
      addAttribute(PositionLengthAttribute::typeid);

  int lastStartOffset = 0;
  bool lastWasCommon = false;
  std::shared_ptr<State> savedState;

  /**
   * Construct a token stream filtering the given input using a Set of common
   * words to create bigrams. Outputs both unigrams with position increment and
   * bigrams with position increment 0 type=gram where one or both of the words
   * in a potential bigram are in the set of common words .
   *
   * @param input TokenStream input in filter chain
   * @param commonWords The set of common words.
   */
public:
  CommonGramsFilter(std::shared_ptr<TokenStream> input,
                    std::shared_ptr<CharArraySet> commonWords);

  /**
   * Inserts bigrams for common words into a token stream. For each input token,
   * output the token. If the token and/or the following token are in the deque
   * of common words also output a bigram with position increment 0 and
   * type="gram"
   *
   * TODO:Consider adding an option to not emit unigram stopwords
   * as in CDL XTF BigramStopFilter, CommonGramsQueryFilter would need to be
   * changed to work with this.
   *
   * TODO: Consider optimizing for the case of three
   * commongrams i.e "man of the year" normally produces 3 bigrams: "man-of",
   * "of-the", "the-year" but with proper management of positions we could
   * eliminate the middle bigram "of-the"and save a disk seek and a whole set of
   * position lookups.
   */
  bool incrementToken()  override;

  void reset()  override;

  // ================================================= Helper Methods
  // ================================================

  /**
   * Determines if the current token is a common term
   *
   * @return {@code true} if the current token is a common term, {@code false}
   * otherwise
   */
private:
  bool isCommon();

  /**
   * Saves this information to form the left part of a gram
   */
  void saveTermBuffer();

  /**
   * Constructs a compound token.
   */
  void gramToken();

protected:
  std::shared_ptr<CommonGramsFilter> shared_from_this()
  {
    return std::static_pointer_cast<CommonGramsFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::commongrams
