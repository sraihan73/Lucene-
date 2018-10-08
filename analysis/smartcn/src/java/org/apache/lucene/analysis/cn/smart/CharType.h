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
 * Internal SmartChineseAnalyzer character type constants.
 * @lucene.experimental
 */
class CharType : public std::enable_shared_from_this<CharType>
{
  GET_CLASS_NAME(CharType)

  /**
   * Punctuation Characters
   */
public:
  static constexpr int DELIMITER = 0;

  /**
   * Letters
   */
  static constexpr int LETTER = 1;

  /**
   * Numeric Digits
   */
  static constexpr int DIGIT = 2;

  /**
   * Han Ideographs
   */
  static constexpr int HANZI = 3;

  /**
   * Characters that act as a space
   */
  static constexpr int SPACE_LIKE = 4;

  /**
   * Full-Width letters
   */
  static constexpr int FULLWIDTH_LETTER = 5;

  /**
   * Full-Width alphanumeric characters
   */
  static constexpr int FULLWIDTH_DIGIT = 6;

  /**
   * Other (not fitting any of the other categories)
   */
  static constexpr int OTHER = 7;

  /**
   * Surrogate character
   */
  static constexpr int SURROGATE = 8;
};

} // namespace org::apache::lucene::analysis::cn::smart
