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
 * A tokenizer that divides text at whitespace characters as defined by
 * {@link Character#isWhitespace(int)}.  Note: That definition explicitly
 * excludes the non-breaking space. Adjacent sequences of non-Whitespace
 * characters form tokens.
 *
 * @see UnicodeWhitespaceTokenizer
 */
class WhitespaceTokenizer final : public CharTokenizer
{
  GET_CLASS_NAME(WhitespaceTokenizer)

  /**
   * Construct a new WhitespaceTokenizer.
   */
public:
  WhitespaceTokenizer();

  /**
   * Construct a new WhitespaceTokenizer using a given
   * {@link org.apache.lucene.util.AttributeFactory}.
   *
   * @param factory
   *          the attribute factory to use for this {@link Tokenizer}
   */
  WhitespaceTokenizer(std::shared_ptr<AttributeFactory> factory);

  /**
   * Construct a new WhitespaceTokenizer using a given
   * {@link org.apache.lucene.util.AttributeFactory}.
   *
   * @param factory the attribute factory to use for this {@link Tokenizer}
   * @param maxTokenLen maximum token length the tokenizer will emit.
   *        Must be greater than 0 and less than MAX_TOKEN_LENGTH_LIMIT
   * (1024*1024)
   * @throws IllegalArgumentException if maxTokenLen is invalid.
   */
  WhitespaceTokenizer(std::shared_ptr<AttributeFactory> factory,
                      int maxTokenLen);

  /** Collects only characters which do not satisfy
   * {@link Character#isWhitespace(int)}.*/
protected:
  bool isTokenChar(int c) override;

protected:
  std::shared_ptr<WhitespaceTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<WhitespaceTokenizer>(
        org.apache.lucene.analysis.util.CharTokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
