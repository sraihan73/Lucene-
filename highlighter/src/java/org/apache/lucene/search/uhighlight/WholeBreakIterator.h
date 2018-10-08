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

/** Just produces one single fragment for the entire text */
class WholeBreakIterator final : public BreakIterator
{
  GET_CLASS_NAME(WholeBreakIterator)
private:
  std::shared_ptr<CharacterIterator> text;
  int start = 0;
  int end = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int current_ = 0;

public:
  int current() override;

  int first() override;

  int following(int pos) override;

  std::shared_ptr<CharacterIterator> getText() override;

  int last() override;

  int next() override;

  int next(int n) override;

  int preceding(int pos) override;

  int previous() override;

  void setText(std::shared_ptr<CharacterIterator> newText) override;

protected:
  std::shared_ptr<WholeBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<WholeBreakIterator>(
        java.text.BreakIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
