#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharacterUtils.h"

#include  "core/src/java/org/apache/lucene/analysis/CharacterBuffer.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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
namespace org::apache::lucene::analysis::ngram
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Tokenizes the input into n-grams of the given size(s).
 * <p>On the contrary to {@link NGramTokenFilter}, this class sets offsets so
 * that characters between startOffset and endOffset in the original stream are
 * the same as the term chars.
 * <p>For example, "abcde" would be tokenized as (minGram=2, maxGram=3):
 * <table summary="ngram tokens example">
 * <tr><th>Term</th><td>ab</td><td>abc</td><td>bc</td><td>bcd</td><td>cd</td><td>cde</td><td>de</td></tr>
 * <tr><th>Position
 * increment</th><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
 * <tr><th>Position
 * length</th><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
 * <tr><th>Offsets</th><td>[0,2[</td><td>[0,3[</td><td>[1,3[</td><td>[1,4[</td><td>[2,4[</td><td>[2,5[</td><td>[3,5[</td></tr>
 * </table>
 * <a name="version"></a>
 * <p>This tokenizer changed a lot in Lucene 4.4 in order to:<ul>
 * <li>tokenize in a streaming fashion to support streams which are larger
 * than 1024 chars (limit of the previous version),
 * <li>count grams based on unicode code points instead of java chars (and
 * never split in the middle of surrogate pairs),
 * <li>give the ability to {@link #isTokenChar(int) pre-tokenize} the stream
 * before computing n-grams.</ul>
 * <p>Additionally, this class doesn't trim trailing whitespaces and emits
 * tokens in a different order, tokens are now emitted by increasing start
 * offsets while they used to be emitted by increasing lengths (which prevented
 * from supporting large input streams).
 */
// non-final to allow for overriding isTokenChar, but all other methods should
// be final
class NGramTokenizer : public Tokenizer
{
  GET_CLASS_NAME(NGramTokenizer)
public:
  static constexpr int DEFAULT_MIN_NGRAM_SIZE = 1;
  static constexpr int DEFAULT_MAX_NGRAM_SIZE = 2;

private:
  std::shared_ptr<CharacterUtils::CharacterBuffer> charBuffer;
  std::deque<int> buffer; // like charBuffer, but converted to code points
  int bufferStart = 0, bufferEnd = 0; // remaining slice in buffer
  int offset = 0;
  int gramSize = 0;
  int minGram = 0, maxGram = 0;
  bool exhausted = false;
  int lastCheckedChar = 0;  // last offset in the buffer that we checked
  int lastNonTokenChar = 0; // last offset that we found to not be a token char
  bool edgesOnly = false;   // leading edges n-grams only

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

public:
  NGramTokenizer(int minGram, int maxGram, bool edgesOnly);

  /**
   * Creates NGramTokenizer with given min and max n-grams.
   * @param minGram the smallest n-gram to generate
   * @param maxGram the largest n-gram to generate
   */
  NGramTokenizer(int minGram, int maxGram);

  NGramTokenizer(std::shared_ptr<AttributeFactory> factory, int minGram,
                 int maxGram, bool edgesOnly);

  /**
   * Creates NGramTokenizer with given min and max n-grams.
   * @param factory {@link org.apache.lucene.util.AttributeFactory} to use
   * @param minGram the smallest n-gram to generate
   * @param maxGram the largest n-gram to generate
   */
  NGramTokenizer(std::shared_ptr<AttributeFactory> factory, int minGram,
                 int maxGram);

  /**
   * Creates NGramTokenizer with default min and max n-grams.
   */
  NGramTokenizer();

private:
  void init(int minGram, int maxGram, bool edgesOnly);

public:
  bool incrementToken()  override final;

private:
  void updateLastNonTokenChar();

  /** Consume one code point. */
  void consume();

  /** Only collect characters which satisfy this condition. */
protected:
  virtual bool isTokenChar(int chr);

public:
  void end()  override final;

  void reset()  override final;

protected:
  std::shared_ptr<NGramTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<NGramTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ngram/
