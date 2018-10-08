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

/** Represents int[], as a slice (offset + length) into an
 *  existing int[].  The {@link #ints} member should never be null; use
 *  {@link #EMPTY_INTS} if necessary.
 *
 *  @lucene.internal */
class IntsRef final : public std::enable_shared_from_this<IntsRef>,
                      public Comparable<std::shared_ptr<IntsRef>>,
                      public Cloneable
{
  GET_CLASS_NAME(IntsRef)
  /** An empty integer array for convenience */
public:
  static std::deque<int> const EMPTY_INTS;

  /** The contents of the IntsRef. Should never be {@code null}. */
  std::deque<int> ints;
  /** Offset of first valid integer. */
  int offset = 0;
  /** Length of used ints. */
  int length = 0;

  /** Create a IntsRef with {@link #EMPTY_INTS} */
  IntsRef();

  /**
   * Create a IntsRef pointing to a new array of size <code>capacity</code>.
   * Offset and length will both be zero.
   */
  IntsRef(int capacity);

  /** This instance will directly reference ints w/o making a copy.
   * ints should not be null.
   */
  IntsRef(std::deque<int> &ints, int offset, int length);

  /**
   * Returns a shallow clone of this instance (the underlying ints are
   * <b>not</b> copied and will be shared by both the returned object and this
   * object.
   *
   * @see #deepCopyOf
   */
  std::shared_ptr<IntsRef> clone() override;

  virtual int hashCode();

  bool equals(std::any other) override;

  bool intsEquals(std::shared_ptr<IntsRef> other);

  /** Signed int order comparison */
  int compareTo(std::shared_ptr<IntsRef> other) override;

  virtual std::wstring toString();

  /**
   * Creates a new IntsRef that points to a copy of the ints from
   * <code>other</code>
   * <p>
   * The returned IntsRef will have a length of other.length
   * and an offset of zero.
   */
  static std::shared_ptr<IntsRef> deepCopyOf(std::shared_ptr<IntsRef> other);

  /**
   * Performs internal consistency checks.
   * Always returns true (or throws IllegalStateException)
   */
  bool isValid();
};

} // namespace org::apache::lucene::util
