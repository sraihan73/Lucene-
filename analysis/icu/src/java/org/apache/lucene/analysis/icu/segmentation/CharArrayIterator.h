#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::icu::segmentation
{

/**
 * Wraps a char[] as CharacterIterator for processing with a BreakIterator
 * @lucene.experimental
 */
class CharArrayIterator final
    : public std::enable_shared_from_this<CharArrayIterator>,
      public CharacterIterator
{
  GET_CLASS_NAME(CharArrayIterator)
private:
  std::deque<wchar_t> array_;
  int start = 0;
  int index = 0;
  int length = 0;
  int limit = 0;

public:
  std::deque<wchar_t> getText();

  int getStart();

  int getLength();

  /**
   * Set a new region of text to be examined by this iterator
   *
   * @param array text buffer to examine
   * @param start offset into buffer
   * @param length maximum length to examine
   */
  void setText(std::deque<wchar_t> &array_, int start, int length);

  wchar_t current() override;

  wchar_t first() override;

  int getBeginIndex() override;

  int getEndIndex() override;

  int getIndex() override;

  wchar_t last() override;

  wchar_t next() override;

  wchar_t previous() override;

  wchar_t setIndex(int position) override;

  std::shared_ptr<CharArrayIterator> clone() override;
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
