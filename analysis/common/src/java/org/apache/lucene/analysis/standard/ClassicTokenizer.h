#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardAnalyzer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::standard
{
class ClassicTokenizerImpl;
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
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/** A grammar-based tokenizer constructed with JFlex
 *
 * <p> This should be a good tokenizer for most European-language documents:
 *
 * <ul>
 *   <li>Splits words at punctuation characters, removing punctuation. However,
 * a dot that's not followed by whitespace is considered part of a token.
 *   <li>Splits words at hyphens, unless there's a number in the token, in which
 * case the whole token is interpreted as a product number and is not split.
 *   <li>Recognizes email addresses and internet hostnames as one token.
 * </ul>
 *
 * <p>Many applications have specific tokenizer needs.  If this tokenizer does
 * not suit your application, please consider copying this source code
 * directory to your project and maintaining your own grammar-based tokenizer.
 *
 * ClassicTokenizer was named StandardTokenizer in Lucene versions prior to 3.1.
 * As of 3.1, {@link StandardTokenizer} implements Unicode text segmentation,
 * as specified by UAX#29.
 */

class ClassicTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(ClassicTokenizer)
  /** A private instance of the JFlex-constructed scanner */
private:
  std::shared_ptr<ClassicTokenizerImpl> scanner;

public:
  static constexpr int ALPHANUM = 0;
  static constexpr int APOSTROPHE = 1;
  static constexpr int ACRONYM = 2;
  static constexpr int COMPANY = 3;
  static constexpr int EMAIL = 4;
  static constexpr int HOST = 5;
  static constexpr int NUM = 6;
  static constexpr int CJ = 7;

  static constexpr int ACRONYM_DEP = 8;

  /** std::wstring token types that correspond to token type int constants */
  static std::deque<std::wstring> const TOKEN_TYPES;

private:
  int skippedPositions = 0;

  int maxTokenLength = StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH;

  /** Set the max allowed token length.  Any token longer
   *  than this is skipped. */
public:
  void setMaxTokenLength(int length);

  /** @see #setMaxTokenLength */
  int getMaxTokenLength();

  /**
   * Creates a new instance of the {@link ClassicTokenizer}.  Attaches
   * the <code>input</code> to the newly created JFlex scanner.
   *
   * See http://issues.apache.org/jira/browse/LUCENE-1068
   */
  ClassicTokenizer();

  /**
   * Creates a new ClassicTokenizer with a given {@link
   * org.apache.lucene.util.AttributeFactory}
   */
  ClassicTokenizer(std::shared_ptr<AttributeFactory> factory);

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

  virtual ~ClassicTokenizer();

  void reset()  override;

protected:
  std::shared_ptr<ClassicTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<ClassicTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
