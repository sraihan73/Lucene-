#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/shingle/Token.h"

#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

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

namespace org::apache::lucene::analysis::shingle
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * A FixedShingleFilter constructs shingles (token n-grams) from a token stream.
 * In other words, it creates combinations of tokens as a single token.
 * <p>
 * Unlike the {@link ShingleFilter}, FixedShingleFilter only emits shingles of a
 * fixed size, and never emits unigrams, even at the end of a TokenStream. In
 * addition, if the filter encounters stacked tokens (eg synonyms), then it will
 * output stacked shingles
 * <p>
 * For example, the sentence "please divide this sentence into shingles"
 * might be tokenized into shingles "please divide", "divide this",
 * "this sentence", "sentence into", and "into shingles".
 * <p>
 * This filter handles position increments &gt; 1 by inserting filler tokens
 * (tokens with termtext "_").
 *
 * @lucene.experimental
 */
class FixedShingleFilter final : public TokenFilter
{
  GET_CLASS_NAME(FixedShingleFilter)

private:
  const std::shared_ptr<Deque<std::shared_ptr<Token>>> tokenPool =
      std::make_shared<ArrayDeque<std::shared_ptr<Token>>>();

  static constexpr int MAX_SHINGLE_STACK_SIZE = 1000;
  static constexpr int MAX_SHINGLE_SIZE = 4;

  const int shingleSize;
  const std::wstring tokenSeparator;

  const std::shared_ptr<Token> gapToken =
      std::make_shared<Token>(std::make_shared<AttributeSource>());
  const std::shared_ptr<Token> endToken =
      std::make_shared<Token>(std::make_shared<AttributeSource>());

  const std::shared_ptr<PositionIncrementAttribute> incAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

  std::deque<std::shared_ptr<Token>> currentShingleTokens;
  int currentShingleStackSize = 0;
  bool inputStreamExhausted = false;

  /**
   * Creates a FixedShingleFilter over an input token stream
   *
   * @param input       the input stream
   * @param shingleSize the shingle size
   */
public:
  FixedShingleFilter(std::shared_ptr<TokenStream> input, int shingleSize);

  /**
   * Creates a FixedShingleFilter over an input token stream
   *
   * @param input          the input tokenstream
   * @param shingleSize    the shingle size
   * @param tokenSeparator a std::wstring to use as a token separator
   * @param fillerToken    a std::wstring to use to represent gaps in the input stream
   * (due to eg stopwords)
   */
  FixedShingleFilter(std::shared_ptr<TokenStream> input, int shingleSize,
                     const std::wstring &tokenSeparator,
                     const std::wstring &fillerToken);

  bool incrementToken()  override;

  void reset()  override;

  void end()  override;

private:
  void finishInnerStream() ;

  std::shared_ptr<Token> lastTokenInShingle();

  bool resetShingleRoot(std::shared_ptr<Token> token) ;

  bool nextShingle() ;

  // check if the next token in the tokenstream is at the same position as this
  // one
  bool lastInStack(std::shared_ptr<Token> token) ;

  bool advanceStack() ;

  std::shared_ptr<Token> newToken();

  void recycleToken(std::shared_ptr<Token> token);

  // for testing
public:
  int instantiatedTokenCount();

private:
  std::shared_ptr<Token>
  nextTokenInGraph(std::shared_ptr<Token> token) ;

  std::shared_ptr<Token>
  nextTokenInStream(std::shared_ptr<Token> token) ;

private:
  class Token : public std::enable_shared_from_this<Token>
  {
    GET_CLASS_NAME(Token)
  public:
    const std::shared_ptr<AttributeSource> attSource;
    const std::shared_ptr<PositionIncrementAttribute> posIncAtt;
    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

    std::shared_ptr<Token> nextToken;

    Token(std::shared_ptr<AttributeSource> attSource);

    virtual int posInc();

    virtual std::shared_ptr<std::wstring> term();

    virtual int startOffset();

    virtual int endOffset();

    virtual void reset(std::shared_ptr<AttributeSource> attSource);

    virtual std::wstring toString();
  };

protected:
  std::shared_ptr<FixedShingleFilter> shared_from_this()
  {
    return std::static_pointer_cast<FixedShingleFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/shingle/
