#pragma once
#include "stringhelper.h"
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::ja::dict
{

/**
 * Dictionary interface for retrieving morphological data
 * by id.
 */
class Dictionary
{
  GET_CLASS_NAME(Dictionary)

public:
  static const std::wstring INTERNAL_SEPARATOR;

  /**
   * Get left id of specified word
   * @return left id
   */
  virtual int getLeftId(int wordId) = 0;

  /**
   * Get right id of specified word
   * @return right id
   */
  virtual int getRightId(int wordId) = 0;

  /**
   * Get word cost of specified word
   * @return word's cost
   */
  virtual int getWordCost(int wordId) = 0;

  /**
   * Get Part-Of-Speech of tokens
   * @param wordId word ID of token
   * @return Part-Of-Speech of the token
   */
  virtual std::wstring getPartOfSpeech(int wordId) = 0;

  /**
   * Get reading of tokens
   * @param wordId word ID of token
   * @return Reading of the token
   */
  virtual std::wstring getReading(int wordId, std::deque<wchar_t> &surface,
                                  int off, int len) = 0;

  /**
   * Get base form of word
   * @param wordId word ID of token
   * @return Base form (only different for inflected words, otherwise null)
   */
  virtual std::wstring getBaseForm(int wordId, std::deque<wchar_t> &surface,
                                   int off, int len) = 0;

  /**
   * Get pronunciation of tokens
   * @param wordId word ID of token
   * @return Pronunciation of the token
   */
  virtual std::wstring getPronunciation(int wordId,
                                        std::deque<wchar_t> &surface, int off,
                                        int len) = 0;

  /**
   * Get inflection type of tokens
   * @param wordId word ID of token
   * @return inflection type, or null
   */
  virtual std::wstring getInflectionType(int wordId) = 0;

  /**
   * Get inflection form of tokens
   * @param wordId word ID of token
   * @return inflection form, or null
   */
  virtual std::wstring getInflectionForm(int wordId) = 0;
  // TODO: maybe we should have a optimal method, a non-typesafe
  // 'getAdditionalData' if other dictionaries like unidic have additional data
};

} // namespace org::apache::lucene::analysis::ja::dict
