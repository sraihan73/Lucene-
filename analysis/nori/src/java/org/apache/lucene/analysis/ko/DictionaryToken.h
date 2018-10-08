#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "KoreanTokenizer.h"
#include "POS.h"
#include "Token.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Dictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/ko/KoreanTokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/POS.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Morpheme.h"

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
namespace org::apache::lucene::analysis::ko
{

using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;

/**
 * A token stored in a {@link Dictionary}.
 */
class DictionaryToken : public Token
{
  GET_CLASS_NAME(DictionaryToken)
private:
  const int wordId;
  const KoreanTokenizer::Type type;
  const std::shared_ptr<Dictionary> dictionary;

public:
  DictionaryToken(KoreanTokenizer::Type type,
                  std::shared_ptr<Dictionary> dictionary, int wordId,
                  std::deque<wchar_t> &surfaceForm, int offset, int length,
                  int startOffset, int endOffset);

  virtual std::wstring toString();

  /**
   * Returns the type of this token
   * @return token type, not null
   */
  virtual KoreanTokenizer::Type getType();

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

  POS::Type getPOSType() override;

  POS::Tag getLeftPOS() override;

  POS::Tag getRightPOS() override;

  std::wstring getReading() override;

  std::deque<std::shared_ptr<Dictionary::Morpheme>> getMorphemes() override;

protected:
  std::shared_ptr<DictionaryToken> shared_from_this()
  {
    return std::static_pointer_cast<DictionaryToken>(Token::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/
