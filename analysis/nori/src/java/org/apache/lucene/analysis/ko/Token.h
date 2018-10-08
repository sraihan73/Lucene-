#pragma once
#include "POS.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
}

namespace org::apache::lucene::analysis::ko::dict
{
class Dictionary;
}
namespace org::apache::lucene::analysis::ko::dict
{
class Morpheme;
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
namespace org::apache::lucene::analysis::ko
{

using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;

/**
 * Analyzed token with morphological data.
 */
class Token : public std::enable_shared_from_this<Token>
{
  GET_CLASS_NAME(Token)
private:
  std::deque<wchar_t> const surfaceForm;
  const int offset;
  const int length;

  const int startOffset;
  const int endOffset;
  int posIncr = 1;
  int posLen = 1;

public:
  Token(std::deque<wchar_t> &surfaceForm, int offset, int length,
        int startOffset, int endOffset);

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
   * Get the {@link POS.Type} of the token.
   */
  virtual POS::Type getPOSType() = 0;

  /**
   * Get the left part of speech of the token.
   */
  virtual POS::Tag getLeftPOS() = 0;

  /**
   * Get the right part of speech of the token.
   */
  virtual POS::Tag getRightPOS() = 0;

  /**
   * Get the reading of the token.
   */
  virtual std::wstring getReading() = 0;

  /**
   * Get the {@link Morpheme} decomposition of the token.
   */
  virtual std::deque<std::shared_ptr<Morpheme>> getMorphemes() = 0;

  /**
   * Get the start offset of the term in the analyzed text.
   */
  virtual int getStartOffset();

  /**
   * Get the end offset of the term in the analyzed text.
   */
  virtual int getEndOffset();

  virtual void setPositionIncrement(int posIncr);

  virtual int getPositionIncrement();

  virtual void setPositionLength(int posLen);

  virtual int getPositionLength();
};

} // namespace org::apache::lucene::analysis::ko