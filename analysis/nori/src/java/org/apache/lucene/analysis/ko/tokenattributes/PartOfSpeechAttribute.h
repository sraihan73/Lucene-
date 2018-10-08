#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/Attribute.h"
#include "../POS.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Dictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/ko/dict/Morpheme.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/Token.h"

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
namespace org::apache::lucene::analysis::ko::tokenattributes
{

using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Type = org::apache::lucene::analysis::ko::POS::Type;
using Token = org::apache::lucene::analysis::ko::Token;
using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;
using Attribute = org::apache::lucene::util::Attribute;

/**
 * Part of Speech attributes for Korean.
 * @lucene.experimental
 */
class PartOfSpeechAttribute : public Attribute
{
  GET_CLASS_NAME(PartOfSpeechAttribute)
  /**
   * Get the {@link Type} of the token.
   */
public:
  virtual Type getPOSType() = 0;

  /**
   * Get the left part of speech of the token.
   */
  virtual Tag getLeftPOS() = 0;

  /**
   * Get the right part of speech of the token.
   */
  virtual Tag getRightPOS() = 0;

  /**
   * Get the {@link Morpheme} decomposition of the token.
   */
  virtual std::deque<std::shared_ptr<Morpheme>> getMorphemes() = 0;

  /**
   * Set the current token.
   */
  virtual void setToken(std::shared_ptr<Token> token) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/tokenattributes/
