#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * Character category data.
 */
class CharacterDefinition final
    : public std::enable_shared_from_this<CharacterDefinition>
{
  GET_CLASS_NAME(CharacterDefinition)

public:
  static const std::wstring FILENAME_SUFFIX;
  static const std::wstring HEADER;
  static constexpr int VERSION = 1;

  static const int CLASS_COUNT = CharacterClass::values()->length;

  // only used internally for lookup:
private:
  enum class CharacterClass {
    GET_CLASS_NAME(CharacterClass) NGRAM,
    DEFAULT,
    SPACE,
    SYMBOL,
    NUMERIC,
    ALPHA,
    CYRILLIC,
    GREEK,
    HIRAGANA,
    KATAKANA,
    KANJI,
    KANJINUMERIC
  };

private:
  std::deque<char> const characterCategoryMap = std::deque<char>(0x10000);

  std::deque<bool> const invokeMap = std::deque<bool>(CLASS_COUNT);
  std::deque<bool> const groupMap = std::deque<bool>(CLASS_COUNT);

  // the classes:
public:
  static const char NGRAM =
      static_cast<char>(static_cast<int>(CharacterClass::NGRAM));
  static const char DEFAULT =
      static_cast<char>(static_cast<int>(CharacterClass::DEFAULT));
  static const char SPACE =
      static_cast<char>(static_cast<int>(CharacterClass::SPACE));
  static const char SYMBOL =
      static_cast<char>(static_cast<int>(CharacterClass::SYMBOL));
  static const char NUMERIC =
      static_cast<char>(static_cast<int>(CharacterClass::NUMERIC));
  static const char ALPHA =
      static_cast<char>(static_cast<int>(CharacterClass::ALPHA));
  static const char CYRILLIC =
      static_cast<char>(static_cast<int>(CharacterClass::CYRILLIC));
  static const char GREEK =
      static_cast<char>(static_cast<int>(CharacterClass::GREEK));
  static const char HIRAGANA =
      static_cast<char>(static_cast<int>(CharacterClass::HIRAGANA));
  static const char KATAKANA =
      static_cast<char>(static_cast<int>(CharacterClass::KATAKANA));
  static const char KANJI =
      static_cast<char>(static_cast<int>(CharacterClass::KANJI));
  static const char KANJINUMERIC =
      static_cast<char>(static_cast<int>(CharacterClass::KANJINUMERIC));

private:
  CharacterDefinition() ;

public:
  char getCharacterClass(wchar_t c);

  bool isInvoke(wchar_t c);

  bool isGroup(wchar_t c);

  bool isKanji(wchar_t c);

  static char lookupCharacterClass(const std::wstring &characterClassName);

  static std::shared_ptr<CharacterDefinition> getInstance();

private:
  class SingletonHolder : public std::enable_shared_from_this<SingletonHolder>
  {
    GET_CLASS_NAME(SingletonHolder)
  public:
    static const std::shared_ptr<CharacterDefinition> INSTANCE;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static SingletonHolder::StaticConstructor staticConstructor;
  };
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/dict/
