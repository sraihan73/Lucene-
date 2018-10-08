#pragma once
#include "../POS.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/POS.h"

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
namespace org::apache::lucene::analysis::ko::dict
{

using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Type = org::apache::lucene::analysis::ko::POS::Type;

/**
 * Dictionary interface for retrieving morphological data
 * by id.
 */
class Dictionary
{
  GET_CLASS_NAME(Dictionary)
  /**
   * A morpheme extracted from a compound token.
   */
private:
  class Morpheme : public std::enable_shared_from_this<Morpheme>
  {
    GET_CLASS_NAME(Morpheme)
  public:
    const Tag posTag;
    const std::wstring surfaceForm;

    Morpheme(Tag posTag, const std::wstring &surfaceForm);
  };

  /**
   * Get left id of specified word
   */
public:
  virtual int getLeftId(int wordId) = 0;

  /**
   * Get right id of specified word
   */
  virtual int getRightId(int wordId) = 0;

  /**
   * Get word cost of specified word
   */
  virtual int getWordCost(int wordId) = 0;

  /**
   * Get the {@link Type} of specified word (morpheme, compound, inflect or
   * pre-analysis)
   */
  virtual Type getPOSType(int wordId) = 0;

  /**
   * Get the left {@link Tag} of specfied word.
   *
   * For {@link Type#MORPHEME} and {@link Type#COMPOUND} the left and right POS
   * are the same.
   */
  virtual Tag getLeftPOS(int wordId) = 0;

  /**
   * Get the right {@link Tag} of specfied word.
   *
   * For {@link Type#MORPHEME} and {@link Type#COMPOUND} the left and right POS
   * are the same.
   */
  virtual Tag getRightPOS(int wordId) = 0;

  /**
   * Get the reading of specified word (mainly used for Hanja to Hangul
   * conversion).
   */
  virtual std::wstring getReading(int wordId) = 0;

  /**
   * Get the morphemes of specified word (e.g. 가깝으나: 가깝 + 으나).
   */
  virtual std::deque<std::shared_ptr<Morpheme>>
  getMorphemes(int wordId, std::deque<wchar_t> &surfaceForm, int off,
               int len) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/dict/
