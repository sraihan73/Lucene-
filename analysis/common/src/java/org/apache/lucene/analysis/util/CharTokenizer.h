#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include  "core/src/java/org/apache/lucene/analysis/CharacterBuffer.h"

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
namespace org::apache::lucene::analysis::util
{

using CharacterBuffer =
    org::apache::lucene::analysis::CharacterUtils::CharacterBuffer;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.standard.StandardTokenizer.MAX_TOKEN_LENGTH_LIMIT;

/**
 * An abstract base class for simple, character-oriented tokenizers.
 * <p>
 * The base class also provides factories to create instances of
 * {@code CharTokenizer} using Java 8 lambdas or method references.
GET_CLASS_NAME(also)
 * It is possible to create an instance which behaves exactly like
 * {@link LetterTokenizer}:
 * <pre class="prettyprint lang-java">
 * Tokenizer tok = CharTokenizer.fromTokenCharPredicate(Character::isLetter);
 * </pre>
 */
class CharTokenizer : public Tokenizer
{
  GET_CLASS_NAME(CharTokenizer)

  /**
   * Creates a new {@link CharTokenizer} instance
   */
public:
  CharTokenizer();

  /**
   * Creates a new {@link CharTokenizer} instance
   *
   * @param factory
   *          the attribute factory to use for this {@link Tokenizer}
   */
  CharTokenizer(std::shared_ptr<AttributeFactory> factory);

  /**
   * Creates a new {@link CharTokenizer} instance
   *
   * @param factory the attribute factory to use for this {@link Tokenizer}
   * @param maxTokenLen maximum token length the tokenizer will emit.
   *        Must be greater than 0 and less than MAX_TOKEN_LENGTH_LIMIT
   * (1024*1024)
   * @throws IllegalArgumentException if maxTokenLen is invalid.
   */
  CharTokenizer(std::shared_ptr<AttributeFactory> factory, int maxTokenLen);

  /**
   * Creates a new instance of CharTokenizer using a custom predicate, supplied
   * as method reference or lambda expression. The predicate should return
   * {@code true} for all valid token characters. <p> This factory is intended
   * to be used with lambdas or method references. E.g., an elegant way to
   * create an instance which behaves exactly as {@link LetterTokenizer} is:
   * <pre class="prettyprint lang-java">
   * Tokenizer tok = CharTokenizer.fromTokenCharPredicate(Character::isLetter);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromTokenCharPredicate(std::function<bool(int)> &tokenCharPredicate);

  /**
   * Creates a new instance of CharTokenizer with the supplied attribute factory
   * using a custom predicate, supplied as method reference or lambda
   * expression. The predicate should return {@code true} for all valid token
   * characters. <p> This factory is intended to be used with lambdas or method
   * references. E.g., an elegant way to create an instance which behaves
   * exactly as {@link LetterTokenizer} is: <pre class="prettyprint lang-java">
   * Tokenizer tok = CharTokenizer.fromTokenCharPredicate(factory,
   * Character::isLetter);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromTokenCharPredicate(std::shared_ptr<AttributeFactory> factory,
                         std::function<bool(int)> &tokenCharPredicate);

  /**
   * Creates a new instance of CharTokenizer using a custom predicate, supplied
   * as method reference or lambda expression. The predicate should return
   * {@code true} for all valid token characters. This factory also takes a
   * function to normalize chars, e.g., lowercasing them, supplied as method
   * reference or lambda expression. <p> This factory is intended to be used
   * with lambdas or method references. E.g., an elegant way to create an
   * instance which behaves exactly as {@link LowerCaseTokenizer} is: <pre
   * class="prettyprint lang-java"> Tokenizer tok =
   * CharTokenizer.fromTokenCharPredicate(Character::isLetter,
   * Character::toLowerCase);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromTokenCharPredicate(std::function<bool(int)> &tokenCharPredicate,
                         std::function<int(int)> &normalizer);

  /**
   * Creates a new instance of CharTokenizer with the supplied attribute factory
   * using a custom predicate, supplied as method reference or lambda
   * expression. The predicate should return {@code true} for all valid token
   * characters. This factory also takes a function to normalize chars, e.g.,
   * lowercasing them, supplied as method reference or lambda expression. <p>
   * This factory is intended to be used with lambdas or method references.
   * E.g., an elegant way to create an instance which behaves exactly as {@link
   * LowerCaseTokenizer} is: <pre class="prettyprint lang-java"> Tokenizer tok =
   * CharTokenizer.fromTokenCharPredicate(factory, Character::isLetter,
   * Character::toLowerCase);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromTokenCharPredicate(std::shared_ptr<AttributeFactory> factory,
                         std::function<bool(int)> &tokenCharPredicate,
                         std::function<int(int)> &normalizer);

private:
  class CharTokenizerAnonymousInnerClass;

  /**
   * Creates a new instance of CharTokenizer using a custom predicate, supplied
   * as method reference or lambda expression. The predicate should return
   * {@code true} for all valid token separator characters. This method is
   * provided for convenience to easily use predicates that are negated (they
   * match the separator characters, not the token characters). <p> This factory
   * is intended to be used with lambdas or method references. E.g., an elegant
   * way to create an instance which behaves exactly as {@link
   * WhitespaceTokenizer} is: <pre class="prettyprint lang-java"> Tokenizer tok
   * = CharTokenizer.fromSeparatorCharPredicate(Character::isWhitespace);
   * </pre>
   */
public:
  static std::shared_ptr<CharTokenizer>
  fromSeparatorCharPredicate(std::function<bool(int)> &separatorCharPredicate);

  /**
   * Creates a new instance of CharTokenizer with the supplied attribute factory
   * using a custom predicate, supplied as method reference or lambda
   * expression. The predicate should return {@code true} for all valid token
   * separator characters. <p> This factory is intended to be used with lambdas
   * or method references. E.g., an elegant way to create an instance which
   * behaves exactly as {@link WhitespaceTokenizer} is: <pre class="prettyprint
   * lang-java"> Tokenizer tok =
   * CharTokenizer.fromSeparatorCharPredicate(factory, Character::isWhitespace);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromSeparatorCharPredicate(std::shared_ptr<AttributeFactory> factory,
                             std::function<bool(int)> &separatorCharPredicate);

  /**
   * Creates a new instance of CharTokenizer using a custom predicate, supplied
   * as method reference or lambda expression. The predicate should return
   * {@code true} for all valid token separator characters. This factory also
   * takes a function to normalize chars, e.g., lowercasing them, supplied as
   * method reference or lambda expression. <p> This factory is intended to be
   * used with lambdas or method references. E.g., an elegant way to create an
   * instance which behaves exactly as the combination {@link
   * WhitespaceTokenizer} and {@link LowerCaseFilter} is: <pre
   * class="prettyprint lang-java"> Tokenizer tok =
   * CharTokenizer.fromSeparatorCharPredicate(Character::isWhitespace,
   * Character::toLowerCase);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromSeparatorCharPredicate(std::function<bool(int)> &separatorCharPredicate,
                             std::function<int(int)> &normalizer);

  /**
   * Creates a new instance of CharTokenizer with the supplied attribute factory
   * using a custom predicate. The predicate should return {@code true} for all
   * valid token separator characters. This factory also takes a function to
   * normalize chars, e.g., lowercasing them, supplied as method reference or
   * lambda expression. <p> This factory is intended to be used with lambdas or
   * method references. E.g., an elegant way to create an instance which behaves
   * exactly as {@link WhitespaceTokenizer} and {@link LowerCaseFilter} is: <pre
   * class="prettyprint lang-java"> Tokenizer tok =
   * CharTokenizer.fromSeparatorCharPredicate(factory, Character::isWhitespace,
   * Character::toLowerCase);
   * </pre>
   */
  static std::shared_ptr<CharTokenizer>
  fromSeparatorCharPredicate(std::shared_ptr<AttributeFactory> factory,
                             std::function<bool(int)> &separatorCharPredicate,
                             std::function<int(int)> &normalizer);

private:
  int offset = 0, bufferIndex = 0, dataLen = 0, finalOffset = 0;

public:
  static constexpr int DEFAULT_MAX_WORD_LEN = 255;

private:
  static constexpr int IO_BUFFER_SIZE = 4096;
  const int maxTokenLen;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<CharacterBuffer> ioBuffer =
      CharacterUtils::newCharacterBuffer(IO_BUFFER_SIZE);

  /**
   * Returns true iff a codepoint should be included in a token. This tokenizer
   * generates as tokens adjacent sequences of codepoints which satisfy this
   * predicate. Codepoints for which this is false are used to define token
   * boundaries and are not included in tokens.
   */
protected:
  virtual bool isTokenChar(int c) = 0;

  /**
   * Called on each token character to normalize it before it is added to the
   * token. The default implementation does nothing. Subclasses may use this to,
   * e.g., lowercase tokens.
   */
  virtual int normalize(int c);

public:
  bool incrementToken()  override final;

  void end()  override final;

  void reset()  override;

protected:
  std::shared_ptr<CharTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<CharTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
class CharTokenizer::CharTokenizerAnonymousInnerClass : public CharTokenizer
{
  GET_CLASS_NAME(CharTokenizer::CharTokenizerAnonymousInnerClass)
private:
  std::function<bool(int)> tokenCharPredicate;
  std::function<int(int)> normalizer;

public:
  CharTokenizerAnonymousInnerClass(std::shared_ptr<AttributeFactory> factory,
                                   std::function<bool(int)> &tokenCharPredicate,
                                   std::function<int(int)> &normalizer);

protected:
  bool isTokenChar(int c) override;

  int normalize(int c) override;

protected:
  std::shared_ptr<CharTokenizerAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<CharTokenizerAnonymousInnerClass>(
        CharTokenizer::shared_from_this());
  }
};
