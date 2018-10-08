#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::search::uhighlight
{

/**
 * Virtually slices the text on both sides of every occurrence of the specified
 * character. If the slice is 0-length which happens for adjacent slice
 * characters or when they are at the beginning or end, that character is
 * reported as a boundary. For every slice between the specified characters, it
 * is further processed with a specified BreakIterator. A consequence is that
 * the enclosed BreakIterator will never "see" the splitting character. <br>
 * <em>Note: {@link #setText(CharacterIterator)} is unsupported. Use the string
 * version.</em>
 *
 * @lucene.experimental
 */
class SplittingBreakIterator : public BreakIterator
{
  GET_CLASS_NAME(SplittingBreakIterator)
private:
  const std::shared_ptr<BreakIterator> baseIter;
  const wchar_t sliceChar;

  std::wstring text;
  int sliceStartIdx = 0;
  int sliceEndIdx = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int current_ = 0;

public:
  SplittingBreakIterator(std::shared_ptr<BreakIterator> baseIter,
                         wchar_t sliceChar);

  void setText(std::shared_ptr<CharacterIterator> newText) override;

  void setText(const std::wstring &newText) override;

  std::shared_ptr<CharacterIterator> getText() override;

  int current() override;

  int first() override;

  int last() override;

  int next() override;

  int previous() override;

  int following(int offset) override;

  int preceding(int offset) override;

  int next(int n) override;

protected:
  std::shared_ptr<SplittingBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<SplittingBreakIterator>(
        java.text.BreakIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
