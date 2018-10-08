#pragma once
#include "stringhelper.h"
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

/**
 * An iterator over match positions (and optionally offsets) for a single
 * document and field
 *
 * To iterate over the matches, call {@link #next()} until it returns {@code
 * false}, retrieving positions and/or offsets after each call.  You should not
 * call the position or offset methods before {@link #next()} has been called,
 * or after {@link #next()} has returned {@code false}.
 *
 * Matches are ordered by start position, and then by end position.  Match
 * intervals may overlap.
 *
 * @see Weight#matches(LeafReaderContext, int)
 *
 * @lucene.experimental
 */
class MatchesIterator
{
  GET_CLASS_NAME(MatchesIterator)

  /**
   * Advance the iterator to the next match position
   * @return {@code true} if matches have not been exhausted
   */
public:
  virtual bool next() = 0;

  /**
   * The start position of the current match
   *
   * Should only be called after {@link #next()} has returned {@code true}
   */
  virtual int startPosition() = 0;

  /**
   * The end position of the current match
   *
   * Should only be called after {@link #next()} has returned {@code true}
   */
  virtual int endPosition() = 0;

  /**
   * The starting offset of the current match, or {@code -1} if offsets are not
   * available
   *
   * Should only be called after {@link #next()} has returned {@code true}
   */
  virtual int startOffset() = 0;

  /**
   * The ending offset of the current match, or {@code -1} if offsets are not
   * available
   *
   * Should only be called after {@link #next()} has returned {@code true}
   */
  virtual int endOffset() = 0;
};

} // namespace org::apache::lucene::search
