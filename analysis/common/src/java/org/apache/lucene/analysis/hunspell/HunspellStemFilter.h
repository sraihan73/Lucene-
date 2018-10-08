#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/hunspell/Stemmer.h"
#include  "core/src/java/org/apache/lucene/util/CharsRef.h"
#include  "core/src/java/org/apache/lucene/analysis/hunspell/Dictionary.h"
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
namespace org::apache::lucene::analysis::hunspell
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRef = org::apache::lucene::util::CharsRef;

/**
 * TokenFilter that uses hunspell affix rules and words to stem tokens.  Since
 * hunspell supports a word having multiple stems, this filter can emit multiple
 * tokens for each consumed token
 *
 * <p>
 * Note: This filter is aware of the {@link KeywordAttribute}. To prevent
 * certain terms from being passed to the stemmer
 * {@link KeywordAttribute#isKeyword()} should be set to <code>true</code>
 * in a previous {@link TokenStream}.
 *
 * Note: For including the original term as well as the stemmed version, see
 * {@link org.apache.lucene.analysis.miscellaneous.KeywordRepeatFilterFactory}
 * </p>
 *
 * @lucene.experimental
 */
class HunspellStemFilter final : public TokenFilter
{
  GET_CLASS_NAME(HunspellStemFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<Stemmer> stemmer;

  std::deque<std::shared_ptr<CharsRef>> buffer;
  std::shared_ptr<State> savedState;

  const bool dedup;
  const bool longestOnly;

  /** Create a {@link HunspellStemFilter} outputting all possible stems.
   *  @see #HunspellStemFilter(TokenStream, Dictionary, bool) */
public:
  HunspellStemFilter(std::shared_ptr<TokenStream> input,
                     std::shared_ptr<Dictionary> dictionary);

  /** Create a {@link HunspellStemFilter} outputting all possible stems.
   *  @see #HunspellStemFilter(TokenStream, Dictionary, bool, bool) */
  HunspellStemFilter(std::shared_ptr<TokenStream> input,
                     std::shared_ptr<Dictionary> dictionary, bool dedup);

  /**
   * Creates a new HunspellStemFilter that will stem tokens from the given
   * TokenStream using affix rules in the provided Dictionary
   *
   * @param input TokenStream whose tokens will be stemmed
   * @param dictionary HunspellDictionary containing the affix rules and words
   * that will be used to stem the tokens
   * @param longestOnly true if only the longest term should be output.
   */
  HunspellStemFilter(std::shared_ptr<TokenStream> input,
                     std::shared_ptr<Dictionary> dictionary, bool dedup,
                     bool longestOnly);

  bool incrementToken()  override;

  void reset()  override;

  static const std::shared_ptr<Comparator<std::shared_ptr<CharsRef>>>
      lengthComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<CharsRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<CharsRef> o1,
                std::shared_ptr<CharsRef> o2) override;
  };

protected:
  std::shared_ptr<HunspellStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<HunspellStemFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/hunspell/
