#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::analysis::cn::smart
{

/**
 * Internal SmartChineseAnalyzer token type constants
 * @lucene.experimental
 */
class WordType : public std::enable_shared_from_this<WordType>
{
  GET_CLASS_NAME(WordType)

  /**
   * Start of a Sentence
   */
public:
  static constexpr int SENTENCE_BEGIN = 0;

  /**
   * End of a Sentence
   */
  static constexpr int SENTENCE_END = 1;

  /**
   * Chinese Word
   */
  static constexpr int CHINESE_WORD = 2;

  /**
   * ASCII std::wstring
   */
  static constexpr int STRING = 3;

  /**
   * ASCII Alphanumeric
   */
  static constexpr int NUMBER = 4;

  /**
   * Punctuation Symbol
   */
  static constexpr int DELIMITER = 5;

  /**
   * Full-Width std::wstring
   */
  static constexpr int FULLWIDTH_STRING = 6;

  /**
   * Full-Width Alphanumeric
   */
  static constexpr int FULLWIDTH_NUMBER = 7;
};

} // namespace org::apache::lucene::analysis::cn::smart
