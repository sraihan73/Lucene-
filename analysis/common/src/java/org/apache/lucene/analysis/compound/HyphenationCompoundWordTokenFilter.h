#pragma once
#include "CompoundWordTokenFilterBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::compound::hyphenation
{
class HyphenationTree;
}

namespace org::apache::lucene::analysis
{
class CharArraySet;
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
namespace org::apache::lucene::analysis::compound
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using org::xml::sax::InputSource;

/**
 * A {@link org.apache.lucene.analysis.TokenFilter} that decomposes compound
 * words found in many Germanic languages.
 *
 * "Donaudampfschiff" becomes Donau, dampf, schiff so that you can find
 * "Donaudampfschiff" even when you only enter "schiff". It uses a hyphenation
 * grammar and a word dictionary to achieve this.
 */
class HyphenationCompoundWordTokenFilter : public CompoundWordTokenFilterBase
{
  GET_CLASS_NAME(HyphenationCompoundWordTokenFilter)
private:
  std::shared_ptr<HyphenationTree> hyphenator;

  /**
   * Creates a new {@link HyphenationCompoundWordTokenFilter} instance.
   *
   * @param input
   *          the {@link org.apache.lucene.analysis.TokenStream} to process
   * @param hyphenator
   *          the hyphenation pattern tree to use for hyphenation
   * @param dictionary
   *          the word dictionary to match against.
   */
public:
  HyphenationCompoundWordTokenFilter(
      std::shared_ptr<TokenStream> input,
      std::shared_ptr<HyphenationTree> hyphenator,
      std::shared_ptr<CharArraySet> dictionary);

  /**
   * Creates a new {@link HyphenationCompoundWordTokenFilter} instance.
   *
   * @param input
   *          the {@link org.apache.lucene.analysis.TokenStream} to process
   * @param hyphenator
   *          the hyphenation pattern tree to use for hyphenation
   * @param dictionary
   *          the word dictionary to match against.
   * @param minWordSize
   *          only words longer than this get processed
   * @param minSubwordSize
   *          only subwords longer than this get to the output stream
   * @param maxSubwordSize
   *          only subwords shorter than this get to the output stream
   * @param onlyLongestMatch
   *          Add only the longest matching subword to the stream
   */
  HyphenationCompoundWordTokenFilter(
      std::shared_ptr<TokenStream> input,
      std::shared_ptr<HyphenationTree> hyphenator,
      std::shared_ptr<CharArraySet> dictionary, int minWordSize,
      int minSubwordSize, int maxSubwordSize, bool onlyLongestMatch);

  /**
   * Create a HyphenationCompoundWordTokenFilter with no dictionary.
   * <p>
   * Calls {@link
   * #HyphenationCompoundWordTokenFilter(org.apache.lucene.analysis.TokenStream,
   * org.apache.lucene.analysis.compound.hyphenation.HyphenationTree,
   * org.apache.lucene.analysis.CharArraySet, int, int, int, bool)
   * HyphenationCompoundWordTokenFilter(matchVersion, input, hyphenator,
   * null, minWordSize, minSubwordSize, maxSubwordSize }
   */
  HyphenationCompoundWordTokenFilter(
      std::shared_ptr<TokenStream> input,
      std::shared_ptr<HyphenationTree> hyphenator, int minWordSize,
      int minSubwordSize, int maxSubwordSize);

  /**
   * Create a HyphenationCompoundWordTokenFilter with no dictionary.
   * <p>
   * Calls {@link
   * #HyphenationCompoundWordTokenFilter(org.apache.lucene.analysis.TokenStream,
   * org.apache.lucene.analysis.compound.hyphenation.HyphenationTree, int, int,
   * int) HyphenationCompoundWordTokenFilter(matchVersion, input, hyphenator,
   * DEFAULT_MIN_WORD_SIZE, DEFAULT_MIN_SUBWORD_SIZE, DEFAULT_MAX_SUBWORD_SIZE }
   */
  HyphenationCompoundWordTokenFilter(
      std::shared_ptr<TokenStream> input,
      std::shared_ptr<HyphenationTree> hyphenator);

  /**
   * Create a hyphenator tree
   *
   * @param hyphenationFilename the filename of the XML grammar to load
   * @return An object representing the hyphenation patterns
   * @throws java.io.IOException If there is a low-level I/O error.
   */
  static std::shared_ptr<HyphenationTree> getHyphenationTree(
      const std::wstring &hyphenationFilename) ;

  /**
   * Create a hyphenator tree
   *
   * @param hyphenationSource the InputSource pointing to the XML grammar
   * @return An object representing the hyphenation patterns
   * @throws java.io.IOException If there is a low-level I/O error.
   */
  static std::shared_ptr<HyphenationTree> getHyphenationTree(
      std::shared_ptr<InputSource> hyphenationSource) ;

protected:
  void decompose() override;

protected:
  std::shared_ptr<HyphenationCompoundWordTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<HyphenationCompoundWordTokenFilter>(
        CompoundWordTokenFilterBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::compound
