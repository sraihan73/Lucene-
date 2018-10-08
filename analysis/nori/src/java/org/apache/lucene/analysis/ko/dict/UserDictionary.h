#pragma once
#include "../POS.h"
#include "Dictionary.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ko::dict
{
class TokenInfoFST;
}

namespace org::apache::lucene::analysis::ko
{
class POS;
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
namespace org::apache::lucene::analysis::ko::dict
{

using POS = org::apache::lucene::analysis::ko::POS;

/**
 * Class for building a User Dictionary.
 * This class allows for adding custom nouns (세종) or compounds (세종시 세종
 * 시).
 */
class UserDictionary final
    : public std::enable_shared_from_this<UserDictionary>,
      public Dictionary
{
  GET_CLASS_NAME(UserDictionary)
  // text -> wordID
private:
  const std::shared_ptr<TokenInfoFST> fst;

public:
  static constexpr int WORD_COST = -100000;

  // NNG left
  static constexpr short LEFT_ID = 1781;

  // NNG right
  static constexpr short RIGHT_ID = 3534;
  // NNG right with hangul and a coda on the last char
  static constexpr short RIGHT_ID_T = 3534;
  // NNG right with hangul and no coda on the last char
  static constexpr short RIGHT_ID_F = 3535;

  // length, length... indexed by compound ID or null for simple noun
private:
  std::deque<std::deque<int>> const segmentations;
  std::deque<short> const rightIds;

public:
  static std::shared_ptr<UserDictionary>
  open(std::shared_ptr<Reader> reader) ;

private:
  UserDictionary(std::deque<std::wstring> &entries) ;

public:
  std::shared_ptr<TokenInfoFST> getFST();

  int getLeftId(int wordId) override;

  int getRightId(int wordId) override;

  int getWordCost(int wordId) override;

  POS::Type getPOSType(int wordId) override;

  POS::Tag getLeftPOS(int wordId) override;

  POS::Tag getRightPOS(int wordId) override;

  std::wstring getReading(int wordId) override;

  std::deque<std::shared_ptr<Morpheme>>
  getMorphemes(int wordId, std::deque<wchar_t> &surfaceForm, int off,
               int len) override;

  /**
   * Lookup words in text
   * @param chars text
   * @param off offset into text
   * @param len length of text
   * @return array of wordId
   */
  std::deque<int> lookup(std::deque<wchar_t> &chars, int off,
                          int len) ;
};

} // namespace org::apache::lucene::analysis::ko::dict
