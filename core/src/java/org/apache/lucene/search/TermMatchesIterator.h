#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PostingsEnum;
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

namespace org::apache::lucene::search
{

using PostingsEnum = org::apache::lucene::index::PostingsEnum;

/**
 * A {@link MatchesIterator} over a single term's postings deque
 */
class TermMatchesIterator
    : public std::enable_shared_from_this<TermMatchesIterator>,
      public MatchesIterator
{
  GET_CLASS_NAME(TermMatchesIterator)

private:
  int upto = 0;
  int pos = 0;
  const std::shared_ptr<PostingsEnum> pe;

  /**
   * Create a new {@link TermMatchesIterator} for the given term and postings
   * deque
   */
public:
  TermMatchesIterator(std::shared_ptr<PostingsEnum> pe) ;

  bool next()  override;

  int startPosition() override;

  int endPosition() override;

  int startOffset()  override;

  int endOffset()  override;
};

} // namespace org::apache::lucene::search
