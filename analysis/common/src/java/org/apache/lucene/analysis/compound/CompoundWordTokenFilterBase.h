#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/compound/CompoundToken.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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
namespace org::apache::lucene::analysis::compound
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

/**
 * Base class for decomposition token filters.
 */
class CompoundWordTokenFilterBase : public TokenFilter
{
  GET_CLASS_NAME(CompoundWordTokenFilterBase)
  /**
   * The default for minimal word length that gets decomposed
   */
public:
  static constexpr int DEFAULT_MIN_WORD_SIZE = 5;

  /**
   * The default for minimal length of subwords that get propagated to the
   * output of this filter
   */
  static constexpr int DEFAULT_MIN_SUBWORD_SIZE = 2;

  /**
   * The default for maximal length of subwords that get propagated to the
   * output of this filter
   */
  static constexpr int DEFAULT_MAX_SUBWORD_SIZE = 15;

protected:
  const std::shared_ptr<CharArraySet> dictionary;
  const std::deque<std::shared_ptr<CompoundToken>> tokens;
  const int minWordSize;
  const int minSubwordSize;
  const int maxSubwordSize;
  const bool onlyLongestMatch;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

private:
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);

  std::shared_ptr<State> current;

protected:
  CompoundWordTokenFilterBase(std::shared_ptr<TokenStream> input,
                              std::shared_ptr<CharArraySet> dictionary,
                              bool onlyLongestMatch);

  CompoundWordTokenFilterBase(std::shared_ptr<TokenStream> input,
                              std::shared_ptr<CharArraySet> dictionary);

  CompoundWordTokenFilterBase(std::shared_ptr<TokenStream> input,
                              std::shared_ptr<CharArraySet> dictionary,
                              int minWordSize, int minSubwordSize,
                              int maxSubwordSize, bool onlyLongestMatch);

public:
  bool incrementToken()  override final;

  /** Decomposes the current {@link #termAtt} and places {@link CompoundToken}
   * instances in the {@link #tokens} deque. The original token may not be placed
   * in the deque, as it is automatically passed through this filter.
   */
protected:
  virtual void decompose() = 0;

public:
  void reset()  override;

  /**
   * Helper class to hold decompounded token information
   */
protected:
  class CompoundToken : public std::enable_shared_from_this<CompoundToken>
  {
    GET_CLASS_NAME(CompoundToken)
  private:
    std::shared_ptr<CompoundWordTokenFilterBase> outerInstance;

  public:
    const std::shared_ptr<std::wstring> txt;
    const int startOffset, endOffset;

    /** Construct the compound token based on a slice of the current {@link
     * CompoundWordTokenFilterBase#termAtt}. */
    CompoundToken(std::shared_ptr<CompoundWordTokenFilterBase> outerInstance,
                  int offset, int length);
  };

protected:
  std::shared_ptr<CompoundWordTokenFilterBase> shared_from_this()
  {
    return std::static_pointer_cast<CompoundWordTokenFilterBase>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/
