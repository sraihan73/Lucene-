#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/MatchesIterator.h"

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

namespace org::apache::lucene::search
{

class AssertingMatchesIterator
    : public std::enable_shared_from_this<AssertingMatchesIterator>,
      public MatchesIterator
{
  GET_CLASS_NAME(AssertingMatchesIterator)

private:
  const std::shared_ptr<MatchesIterator> in_;
  State state = State::UNPOSITIONED;

private:
  enum class State { GET_CLASS_NAME(State) UNPOSITIONED, ITERATING, EXHAUSTED };

public:
  AssertingMatchesIterator(std::shared_ptr<MatchesIterator> in_);

  bool next()  override;

  int startPosition() override;

  int endPosition() override;

  int startOffset()  override;

  int endOffset()  override;
};

} // #include  "core/src/java/org/apache/lucene/search/
