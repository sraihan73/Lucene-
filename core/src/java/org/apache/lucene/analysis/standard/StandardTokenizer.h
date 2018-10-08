#pragma once
#include "../Tokenizer.h"
#include "StandardAnalyzer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::standard
{
class StandardTokenizerImpl;
}

namespace org::apache::lucene::util
{
class AttributeFactory;
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
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
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

namespace org::apache::lucene::analysis::standard
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/** A grammar-based tokenizer constructed with JFlex.
 * <p>
 * This class implements the Word Break rules from the
 * Unicode Text Segmentation algorithm, as specified in
 * <a href="http://unicode.org/reports/tr29/">Unicode Standard Annex #29</a>.
 * <p>Many applications have specific tokenizer needs.  If this tokenizer does
 * not suit your application, please consider copying this source code
 * directory to your project and maintaining your own grammar-based tokenizer.
 */

class StandardTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(StandardTokenizer)
  /** A private instance of the JFlex-constructed scanner */
private:
  std::shared_ptr<StandardTokenizerImpl> scanner;

  /** Alpha/numeric token type */
public:
  static constexpr int ALPHANUM = 0;
  /** Numeric token type */
  static constexpr int NUM = 1;
  /** Southeast Asian token type */
  static constexpr int SOUTHEAST_ASIAN = 2;
  /** Ideographic token type */
  static constexpr int IDEOGRAPHIC = 3;
  /** Hiragana token type */
  static constexpr int HIRAGANA = 4;
  /** Katakana token type */
  static constexpr int KATAKANA = 5;
  /** Hangul token type */
  static constexpr int HANGUL = 6;
  /** Emoji token type. */
  static constexpr int EMOJI = 7;

  /** std::wstring token types that correspond to token type int constants */
  static std::deque<std::wstring> const TOKEN_TYPES;

  /** Absolute maximum sized token */
  static constexpr int MAX_TOKEN_LENGTH_LIMIT = 1024 * 1024;

private:
  int skippedPositions = 0;

  int maxTokenLength = StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH;

  /**
   * Set the max allowed token length.  Tokens larger than this will be chopped
   * up at this token length and emitted as multiple tokens.  If you need to
   * skip such large tokens, you could increase this max length, and then
   * use {@code LengthFilter} to remove long tokens.  The default is
   * {@link StandardAnalyzer#DEFAULT_MAX_TOKEN_LENGTH}.
   *
   * @throws IllegalArgumentException if the given length is outside of the
   *  range [1, {@value #MAX_TOKEN_LENGTH_LIMIT}].
   */
public:
  void setMaxTokenLength(int length);

  /** Returns the current maximum token length
   *
   *  @see #setMaxTokenLength */
  int getMaxTokenLength();

  /**
   * Creates a new instance of the {@link
   org.apache.lucene.analysis.standard.StandardTokenizer}.  Attaches
   * the <code>input</code> to the newly created JFlex scanner.

   * See http://issues.apache.org/jira/browse/LUCENE-1068
   */
  StandardTokenizer();

  /**
   * Creates a new StandardTokenizer with a given {@link
   * org.apache.lucene.util.AttributeFactory}
   */
  StandardTokenizer(std::shared_ptr<AttributeFactory> factory);

private:
  void init();

  // this tokenizer generates three attributes:
  // term offset, positionIncrement and type
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.analysis.TokenStream#next()
   */
public:
  bool incrementToken()  override final;

  void end()  override final;

  virtual ~StandardTokenizer();

  void reset()  override;

protected:
  std::shared_ptr<StandardTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<StandardTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
