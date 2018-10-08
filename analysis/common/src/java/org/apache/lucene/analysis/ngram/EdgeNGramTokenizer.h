#pragma once
#include "NGramTokenizer.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeFactory;
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
namespace org::apache::lucene::analysis::ngram
{

using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Tokenizes the input from an edge into n-grams of given size(s).
 * <p>
 * This {@link Tokenizer} create n-grams from the beginning edge of a input
token.
 * <p><a name="match_version"></a>As of Lucene 4.4, this class supports
 * {@link #isTokenChar(int) pre-tokenization} and correctly handles
GET_CLASS_NAME(supports)
 * supplementary characters.
 */
class EdgeNGramTokenizer : public NGramTokenizer
{
  GET_CLASS_NAME(EdgeNGramTokenizer)
public:
  static constexpr int DEFAULT_MAX_GRAM_SIZE = 1;
  static constexpr int DEFAULT_MIN_GRAM_SIZE = 1;

  /**
   * Creates EdgeNGramTokenizer that can generate n-grams in the sizes of the
   * given range
   *
   * @param minGram the smallest n-gram to generate
   * @param maxGram the largest n-gram to generate
   */
  EdgeNGramTokenizer(int minGram, int maxGram);

  /**
   * Creates EdgeNGramTokenizer that can generate n-grams in the sizes of the
   * given range
   *
   * @param factory {@link org.apache.lucene.util.AttributeFactory} to use
   * @param minGram the smallest n-gram to generate
   * @param maxGram the largest n-gram to generate
   */
  EdgeNGramTokenizer(std::shared_ptr<AttributeFactory> factory, int minGram,
                     int maxGram);

protected:
  std::shared_ptr<EdgeNGramTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<EdgeNGramTokenizer>(
        NGramTokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ngram
