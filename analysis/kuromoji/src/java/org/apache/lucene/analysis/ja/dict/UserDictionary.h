#pragma once
#include "Dictionary.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class TokenInfoFST;
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
namespace org::apache::lucene::analysis::ja::dict
{

/**
 * Class for building a User Dictionary.
 * This class allows for custom segmentation of phrases.
 */
class UserDictionary final
    : public std::enable_shared_from_this<UserDictionary>,
      public Dictionary
{
  GET_CLASS_NAME(UserDictionary)

  // phrase text -> phrase ID
private:
  const std::shared_ptr<TokenInfoFST> fst;

  // holds wordid, length, length... indexed by phrase ID
  std::deque<std::deque<int>> const segmentations;

  // holds readings and POS, indexed by wordid
  std::deque<std::wstring> const data;

  static constexpr int CUSTOM_DICTIONARY_WORD_ID_OFFSET = 100000000;

public:
  static constexpr int WORD_COST = -100000;

  static constexpr int LEFT_ID = 5;

  static constexpr int RIGHT_ID = 5;

  static std::shared_ptr<UserDictionary>
  open(std::shared_ptr<Reader> reader) ;

private:
  UserDictionary(std::deque<std::deque<std::wstring>> &featureEntries) throw(
      IOException);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<std::wstring>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<UserDictionary> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<UserDictionary> outerInstance);

    int compare(std::deque<std::wstring> &left,
                std::deque<std::wstring> &right);
  };

  /**
   * Lookup words in text
   * @param chars text
   * @param off offset into text
   * @param len length of text
   * @return array of {wordId, position, length}
   */
public:
  std::deque<std::deque<int>> lookup(std::deque<wchar_t> &chars, int off,
                                       int len) ;

  std::shared_ptr<TokenInfoFST> getFST();

private:
  static std::deque<std::deque<int>> const EMPTY_RESULT;

  /**
   * Convert Map of index and wordIdAndLength to array of {wordId, index,
   * length}
   * @return array of {wordId, index, length}
   */
  std::deque<std::deque<int>>
  toIndexArray(std::unordered_map<int, std::deque<int>> &input);

public:
  std::deque<int> lookupSegmentation(int phraseID);

  int getLeftId(int wordId) override;

  int getRightId(int wordId) override;

  int getWordCost(int wordId) override;

  std::wstring getReading(int wordId, std::deque<wchar_t> &surface, int off,
                          int len) override;

  std::wstring getPartOfSpeech(int wordId) override;

  std::wstring getBaseForm(int wordId, std::deque<wchar_t> &surface, int off,
                           int len) override;

  std::wstring getPronunciation(int wordId, std::deque<wchar_t> &surface,
                                int off, int len) override;

  std::wstring getInflectionType(int wordId) override;

  std::wstring getInflectionForm(int wordId) override;

private:
  std::deque<std::wstring> getAllFeaturesArray(int wordId);

  std::wstring getFeature(int wordId, std::deque<int> &fields);
};

} // namespace org::apache::lucene::analysis::ja::dict
