#pragma once
#include "JapaneseTokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ja/dict/Dictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

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
namespace org::apache::lucene::analysis::ja
{

using Type = org::apache::lucene::analysis::ja::JapaneseTokenizer::Type;
using Dictionary = org::apache::lucene::analysis::ja::dict::Dictionary;

/**
 * Analyzed token with morphological data from its dictionary.
 */
class Token : public std::enable_shared_from_this<Token>
{
  GET_CLASS_NAME(Token)
private:
  const std::shared_ptr<Dictionary> dictionary;

  const int wordId;

  std::deque<wchar_t> const surfaceForm;
  const int offset;
  const int length;

  const int position;
  int positionLength = 0;

  const Type type;

public:
  Token(int wordId, std::deque<wchar_t> &surfaceForm, int offset, int length,
        Type type, int position, std::shared_ptr<Dictionary> dictionary);

  virtual std::wstring toString();

  /**
   * @return surfaceForm
   */
  virtual std::deque<wchar_t> getSurfaceForm();

  /**
   * @return offset into surfaceForm
   */
  virtual int getOffset();

  /**
   * @return length of surfaceForm
   */
  virtual int getLength();

  /**
   * @return surfaceForm as a std::wstring
   */
  virtual std::wstring getSurfaceFormString();

  /**
   * @return reading. null if token doesn't have reading.
   */
  virtual std::wstring getReading();

  /**
   * @return pronunciation. null if token doesn't have pronunciation.
   */
  virtual std::wstring getPronunciation();

  /**
   * @return part of speech.
   */
  virtual std::wstring getPartOfSpeech();

  /**
   * @return inflection type or null
   */
  virtual std::wstring getInflectionType();

  /**
   * @return inflection form or null
   */
  virtual std::wstring getInflectionForm();

  /**
   * @return base form or null if token is not inflected
   */
  virtual std::wstring getBaseForm();

  /**
   * Returns the type of this token
   * @return token type, not null
   */
  virtual Type getType();

  /**
   * Returns true if this token is known word
   * @return true if this token is in standard dictionary. false if not.
   */
  virtual bool isKnown();

  /**
   * Returns true if this token is unknown word
   * @return true if this token is unknown word. false if not.
   */
  virtual bool isUnknown();

  /**
   * Returns true if this token is defined in user dictionary
   * @return true if this token is in user dictionary. false if not.
   */
  virtual bool isUser();

  /**
   * Get index of this token in input text
   * @return position of token
   */
  virtual int getPosition();

  /**
   * Set the position length (in tokens) of this token.  For normal
   * tokens this is 1; for compound tokens it's &gt; 1.
   */
  virtual void setPositionLength(int positionLength);

  /**
   * Get the length (in tokens) of this token.  For normal
   * tokens this is 1; for compound tokens it's &gt; 1.
   * @return position length of token
   */
  virtual int getPositionLength();
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/
