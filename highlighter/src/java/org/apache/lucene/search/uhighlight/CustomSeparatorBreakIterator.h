#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
 * A {@link BreakIterator} that breaks the text whenever a certain separator,
 * provided as a constructor argument, is found.
 */
class CustomSeparatorBreakIterator final : public BreakIterator
{
  GET_CLASS_NAME(CustomSeparatorBreakIterator)

private:
  const wchar_t separator;
  std::shared_ptr<CharacterIterator> text;
  // C++ NOTE: Fields cannot have the same name as methods:
  int current_ = 0;

public:
  CustomSeparatorBreakIterator(wchar_t separator);

  int current() override;

  int first() override;

  int last() override;

  int next() override;

private:
  int advanceForward();

public:
  int following(int pos) override;

  int previous() override;

private:
  int advanceBackward();

public:
  int preceding(int pos) override;

  int next(int n) override;

  std::shared_ptr<CharacterIterator> getText() override;

  void setText(std::shared_ptr<CharacterIterator> newText) override;

protected:
  std::shared_ptr<CustomSeparatorBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<CustomSeparatorBreakIterator>(
        java.text.BreakIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
