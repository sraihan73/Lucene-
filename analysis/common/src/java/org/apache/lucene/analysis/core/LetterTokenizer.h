#pragma once
#include "../util/CharTokenizer.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::core
{

using CharTokenizer = org::apache::lucene::analysis::util::CharTokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * A LetterTokenizer is a tokenizer that divides text at non-letters. That's to
 * say, it defines tokens as maximal strings of adjacent letters, as defined by
 * java.lang.Character.isLetter() predicate.
 * <p>
 * Note: this does a decent job for most European languages, but does a terrible
 * job for some Asian languages, where words are not separated by spaces.
 * </p>
 */

class LetterTokenizer : public CharTokenizer
{
  GET_CLASS_NAME(LetterTokenizer)

  /**
   * Construct a new LetterTokenizer.
   */
public:
  LetterTokenizer();

  /**
   * Construct a new LetterTokenizer using a given
   * {@link org.apache.lucene.util.AttributeFactory}.
   *
   * @param factory
   *          the attribute factory to use for this {@link Tokenizer}
   */
  LetterTokenizer(std::shared_ptr<AttributeFactory> factory);

  /**
   * Construct a new LetterTokenizer using a given
   * {@link org.apache.lucene.util.AttributeFactory}.
   *
   * @param factory the attribute factory to use for this {@link Tokenizer}
   * @param maxTokenLen maximum token length the tokenizer will emit.
   *        Must be greater than 0 and less than MAX_TOKEN_LENGTH_LIMIT
   (1024*1024)
   * @throws IllegalArgumentException if maxTokenLen is invalid.

   */
  LetterTokenizer(std::shared_ptr<AttributeFactory> factory, int maxTokenLen);

  /** Collects only characters which satisfy
   * {@link Character#isLetter(int)}.*/
protected:
  bool isTokenChar(int c) override;

protected:
  std::shared_ptr<LetterTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<LetterTokenizer>(
        org.apache.lucene.analysis.util.CharTokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
