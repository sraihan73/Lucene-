#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/shingle/InputWindowToken.h"

#include  "core/src/java/org/apache/lucene/analysis/shingle/CircularSequence.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
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
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * <p>A ShingleFilter constructs shingles (token n-grams) from a token stream.
 * In other words, it creates combinations of tokens as a single token.
 *
 * <p>For example, the sentence "please divide this sentence into shingles"
 * might be tokenized into shingles "please divide", "divide this",
 * "this sentence", "sentence into", and "into shingles".
 *
 * <p>This filter handles position increments &gt; 1 by inserting filler tokens
 * (tokens with termtext "_"). It does not handle a position increment of 0.
 */
class ShingleFilter final : public TokenFilter
{
  GET_CLASS_NAME(ShingleFilter)

  /**
   * filler token for when positionIncrement is more than 1
   */
public:
  static const std::wstring DEFAULT_FILLER_TOKEN;

  /**
   * default maximum shingle size is 2.
   */
  static constexpr int DEFAULT_MAX_SHINGLE_SIZE = 2;

  /**
   * default minimum shingle size is 2.
   */
  static constexpr int DEFAULT_MIN_SHINGLE_SIZE = 2;

  /**
   * default token type attribute value is "shingle"
   */
  static const std::wstring DEFAULT_TOKEN_TYPE;

  /**
   * The default string to use when joining adjacent tokens to form a shingle
   */
  static const std::wstring DEFAULT_TOKEN_SEPARATOR;

  /**
   * The sequence of input stream tokens (or filler tokens, if necessary)
   * that will be composed to form output shingles.
   */
private:
  std::deque<std::shared_ptr<InputWindowToken>> inputWindow =
      std::deque<std::shared_ptr<InputWindowToken>>();

  /**
   * The number of input tokens in the next output token.  This is the "n" in
   * "token n-grams".
   */
  std::shared_ptr<CircularSequence> gramSize;

  /**
   * Shingle and unigram text is composed here.
   */
  std::shared_ptr<StringBuilder> gramBuilder =
      std::make_shared<StringBuilder>();

  /**
   * The token type attribute value to use - default is "shingle"
   */
  std::wstring tokenType = DEFAULT_TOKEN_TYPE;

  /**
   * The string to use when joining adjacent tokens to form a shingle
   */
  std::wstring tokenSeparator = DEFAULT_TOKEN_SEPARATOR;

  /**
   * The string to insert for each position at which there is no token
   * (i.e., when position increment is greater than one).
   */
  std::deque<wchar_t> fillerToken = DEFAULT_FILLER_TOKEN.toCharArray();

  /**
   * By default, we output unigrams (individual tokens) as well as shingles
   * (token n-grams).
   */
  bool outputUnigrams = true;

  /**
   * By default, we don't override behavior of outputUnigrams.
   */
  bool outputUnigramsIfNoShingles = false;

  /**
   * maximum shingle size (number of tokens)
   */
  int maxShingleSize = 0;

  /**
   * minimum shingle size (number of tokens)
   */
  int minShingleSize = 0;

  /**
   * The remaining number of filler tokens to be inserted into the input stream
   * from which shingles are composed, to handle position increments greater
   * than one.
   */
  int numFillerTokensToInsert = 0;

  /**
   * When the next input stream token has a position increment greater than
   * one, it is stored in this field until sufficient filler tokens have been
   * inserted to account for the position increment.
   */
  std::shared_ptr<AttributeSource> nextInputStreamToken;

  /**
   * Whether or not there is a next input stream token.
   */
  bool isNextInputStreamToken = false;

  /**
   * Whether at least one unigram or shingle has been output at the current
   * position.
   */
  bool isOutputHere = false;

  /**
   * true if no shingles have been output yet (for outputUnigramsIfNoShingles).
   */
public:
  bool noShingleOutput = true;

  /**
   * Holds the State after input.end() was called, so we can
   * restore it in our end() impl.
   */
private:
  std::shared_ptr<AttributeSource::State> endState;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

  /**
   * Constructs a ShingleFilter with the specified shingle size from the
   * {@link TokenStream} <code>input</code>
   *
   * @param input input stream
   * @param minShingleSize minimum shingle size produced by the filter.
   * @param maxShingleSize maximum shingle size produced by the filter.
   */
public:
  ShingleFilter(std::shared_ptr<TokenStream> input, int minShingleSize,
                int maxShingleSize);

  /**
   * Constructs a ShingleFilter with the specified shingle size from the
   * {@link TokenStream} <code>input</code>
   *
   * @param input input stream
   * @param maxShingleSize maximum shingle size produced by the filter.
   */
  ShingleFilter(std::shared_ptr<TokenStream> input, int maxShingleSize);

  /**
   * Construct a ShingleFilter with default shingle size: 2.
   *
   * @param input input stream
   */
  ShingleFilter(std::shared_ptr<TokenStream> input);

  /**
   * Construct a ShingleFilter with the specified token type for shingle tokens
   * and the default shingle size: 2
   *
   * @param input input stream
   * @param tokenType token type for shingle tokens
   */
  ShingleFilter(std::shared_ptr<TokenStream> input,
                const std::wstring &tokenType);

  /**
   * Set the type of the shingle tokens produced by this filter.
   * (default: "shingle")
   *
   * @param tokenType token tokenType
   */
  void setTokenType(const std::wstring &tokenType);

  /**
   * Shall the output stream contain the input tokens (unigrams) as well as
   * shingles? (default: true.)
   *
   * @param outputUnigrams Whether or not the output stream shall contain
   * the input tokens (unigrams)
   */
  void setOutputUnigrams(bool outputUnigrams);

  /**
   * <p>Shall we override the behavior of outputUnigrams==false for those
   * times when no shingles are available (because there are fewer than
   * minShingleSize tokens in the input stream)? (default: false.)
   * <p>Note that if outputUnigrams==true, then unigrams are always output,
   * regardless of whether any shingles are available.
   *
   * @param outputUnigramsIfNoShingles Whether or not to output a single
   * unigram when no shingles are available.
   */
  void setOutputUnigramsIfNoShingles(bool outputUnigramsIfNoShingles);

  /**
   * Set the max shingle size (default: 2)
   *
   * @param maxShingleSize max size of output shingles
   */
  void setMaxShingleSize(int maxShingleSize);

  /**
   * <p>Set the min shingle size (default: 2).
   * <p>This method requires that the passed in minShingleSize is not greater
   * than maxShingleSize, so make sure that maxShingleSize is set before
   * calling this method.
   * <p>The unigram output option is independent of the min shingle size.
   *
   * @param minShingleSize min size of output shingles
   */
  void setMinShingleSize(int minShingleSize);

  /**
   * Sets the string to use when joining adjacent tokens to form a shingle
   * @param tokenSeparator used to separate input stream tokens in output
   * shingles
   */
  void setTokenSeparator(const std::wstring &tokenSeparator);

  /**
   * Sets the string to insert for each position at which there is no token
   * (i.e., when position increment is greater than one).
   *
   * @param fillerToken string to insert at each position where there is no
   * token
   */
  void setFillerToken(const std::wstring &fillerToken);

  bool incrementToken()  override;

private:
  bool exhausted = false;

  /**
   * <p>Get the next token from the input stream.
   * <p>If the next token has <code>positionIncrement &gt; 1</code>,
   * <code>positionIncrement - 1</code> {@link #fillerToken}s are
   * inserted first.
   * @param target Where to put the new token; if null, a new instance is
   * created.
   * @return On success, the populated token; null otherwise
   * @throws IOException if the input stream has a problem
   */
  std::shared_ptr<InputWindowToken>
  getNextToken(std::shared_ptr<InputWindowToken> target) ;

public:
  void end()  override;

  /**
   * <p>Fills {@link #inputWindow} with input stream tokens, if available,
   * shifting to the right if the window was previously full.
   * <p>Resets {@link #gramSize} to its minimum value.
   *
   * @throws IOException if there's a problem getting the next token
   */
private:
  void shiftInputWindow() ;

public:
  void reset()  override;

  /**
   * <p>An instance of this class is used to maintain the number of input
   * stream tokens that will be used to compose the next unigram or shingle:
   * {@link #gramSize}.
   * <p><code>gramSize</code> will take on values from the circular sequence
   * <b>{ [ 1, ] {@link #minShingleSize} [ , ... , {@link #maxShingleSize} ]
   * }</b>. <p>1 is included in the circular sequence only if
   * {@link #outputUnigrams} = true.
   */
private:
  class CircularSequence : public std::enable_shared_from_this<CircularSequence>
  {
    GET_CLASS_NAME(CircularSequence)
  private:
    std::shared_ptr<ShingleFilter> outerInstance;

    int value = 0;
    int previousValue = 0;
    int minValue = 0;

  public:
    CircularSequence(std::shared_ptr<ShingleFilter> outerInstance);

    /**
     * @return the current value.
     * @see #advance()
     */
    virtual int getValue();

    /**
     * <p>Increments this circular number's value to the next member in the
     * circular sequence
     * <code>gramSize</code> will take on values from the circular sequence
     * <b>{ [ 1, ] {@link #minShingleSize} [ , ... , {@link #maxShingleSize} ]
     * }</b>. <p>1 is included in the circular sequence only if
     * {@link #outputUnigrams} = true.
     */
    virtual void advance();

    /**
     * <p>Sets this circular number's value to the first member of the
     * circular sequence
     * <p><code>gramSize</code> will take on values from the circular sequence
     * <b>{ [ 1, ] {@link #minShingleSize} [ , ... , {@link #maxShingleSize} ]
     * }</b>. <p>1 is included in the circular sequence only if
     * {@link #outputUnigrams} = true.
     */
    virtual void reset();

    /**
     * <p>Returns true if the current value is the first member of the circular
     * sequence.
     * <p>If {@link #outputUnigrams} = true, the first member of the circular
     * sequence will be 1; otherwise, it will be {@link #minShingleSize}.
     *
     * @return true if the current value is the first member of the circular
     *  sequence; false otherwise
     */
    virtual bool atMinValue();

    /**
     * @return the value this instance had before the last advance() call
     */
    virtual int getPreviousValue();
  };

private:
  class InputWindowToken : public std::enable_shared_from_this<InputWindowToken>
  {
    GET_CLASS_NAME(InputWindowToken)
  public:
    const std::shared_ptr<AttributeSource> attSource;
    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;
    bool isFiller = false;

    InputWindowToken(std::shared_ptr<AttributeSource> attSource);
  };

protected:
  std::shared_ptr<ShingleFilter> shared_from_this()
  {
    return std::static_pointer_cast<ShingleFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/shingle/
