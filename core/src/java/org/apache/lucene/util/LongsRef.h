#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
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
namespace org::apache::lucene::util
{

/** Represents long[], as a slice (offset + length) into an
 *  existing long[].  The {@link #longs} member should never be null; use
 *  {@link #EMPTY_LONGS} if necessary.
 *
 *  @lucene.internal */
class LongsRef final : public std::enable_shared_from_this<LongsRef>,
                       public Comparable<std::shared_ptr<LongsRef>>,
                       public Cloneable
{
  GET_CLASS_NAME(LongsRef)
  /** An empty long array for convenience */
public:
  static std::deque<int64_t> const EMPTY_LONGS;

  /** The contents of the LongsRef. Should never be {@code null}. */
  std::deque<int64_t> longs;
  /** Offset of first valid long. */
  int offset = 0;
  /** Length of used longs. */
  int length = 0;

  /** Create a LongsRef with {@link #EMPTY_LONGS} */
  LongsRef();

  /**
   * Create a LongsRef pointing to a new array of size <code>capacity</code>.
   * Offset and length will both be zero.
   */
  LongsRef(int capacity);

  /** This instance will directly reference longs w/o making a copy.
   * longs should not be null */
  LongsRef(std::deque<int64_t> &longs, int offset, int length);

  /**
   * Returns a shallow clone of this instance (the underlying longs are
   * <b>not</b> copied and will be shared by both the returned object and this
   * object.
   *
   * @see #deepCopyOf
   */
  std::shared_ptr<LongsRef> clone() override;

  virtual int hashCode();

  bool equals(std::any other) override;

  bool longsEquals(std::shared_ptr<LongsRef> other);

  /** Signed int order comparison */
  int compareTo(std::shared_ptr<LongsRef> other) override;

  virtual std::wstring toString();

  /**
   * Creates a new LongsRef that points to a copy of the longs from
   * <code>other</code>
   * <p>
   * The returned IntsRef will have a length of other.length
   * and an offset of zero.
   */
  static std::shared_ptr<LongsRef> deepCopyOf(std::shared_ptr<LongsRef> other);

  /**
   * Performs internal consistency checks.
   * Always returns true (or throws IllegalStateException)
   */
  bool isValid();
};

} // #include  "core/src/java/org/apache/lucene/util/
