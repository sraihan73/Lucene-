#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/standard/UAX29URLEmailTokenizerImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

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
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * This class implements Word Break rules from the Unicode Text Segmentation
 * algorithm, as specified in
 * <a href="http://unicode.org/reports/tr29/">Unicode Standard Annex #29</a>
 * URLs and email addresses are also tokenized according to the relevant RFCs.
 * <p>
 * Tokens produced are of the following types:
 * <ul>
 *   <li>&lt;ALPHANUM&gt;: A sequence of alphabetic and numeric characters</li>
 *   <li>&lt;NUM&gt;: A number</li>
 *   <li>&lt;URL&gt;: A URL</li>
 *   <li>&lt;EMAIL&gt;: An email address</li>
 *   <li>&lt;SOUTHEAST_ASIAN&gt;: A sequence of characters from South and
 * Southeast Asian languages, including Thai, Lao, Myanmar, and Khmer</li>
 *   <li>&lt;IDEOGRAPHIC&gt;: A single CJKV ideographic character</li>
 *   <li>&lt;HIRAGANA&gt;: A single hiragana character</li>
 * </ul>
 */

class UAX29URLEmailTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(UAX29URLEmailTokenizer)
  /** A private instance of the JFlex-constructed scanner */
private:
  const std::shared_ptr<UAX29URLEmailTokenizerImpl> scanner;

public:
  static constexpr int ALPHANUM = 0;
  static constexpr int NUM = 1;
  static constexpr int SOUTHEAST_ASIAN = 2;
  static constexpr int IDEOGRAPHIC = 3;
  static constexpr int HIRAGANA = 4;
  static constexpr int KATAKANA = 5;
  static constexpr int HANGUL = 6;
  static constexpr int URL = 7;
  static constexpr int EMAIL = 8;

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
   * {@link UAX29URLEmailAnalyzer#DEFAULT_MAX_TOKEN_LENGTH}.
   *
   * @throws IllegalArgumentException if the given length is outside of the
   *  range [1, {@value #MAX_TOKEN_LENGTH_LIMIT}].
   */
public:
  void setMaxTokenLength(int length);

  /** @see #setMaxTokenLength */
  int getMaxTokenLength();

  /**
   * Creates a new instance of the UAX29URLEmailTokenizer.  Attaches
   * the <code>input</code> to the newly created JFlex scanner.

   */
  UAX29URLEmailTokenizer();

  /**
   * Creates a new UAX29URLEmailTokenizer with a given {@link AttributeFactory}
   */
  UAX29URLEmailTokenizer(std::shared_ptr<AttributeFactory> factory);

private:
  std::shared_ptr<UAX29URLEmailTokenizerImpl> getScanner();

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

public:
  bool incrementToken()  override final;

  void end()  override final;

  virtual ~UAX29URLEmailTokenizer();

  void reset()  override;

protected:
  std::shared_ptr<UAX29URLEmailTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<UAX29URLEmailTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
