#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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

/**
 * Methods for manipulating strings.
 *
 * @lucene.internal
 */
class StringHelper : public std::enable_shared_from_this<StringHelper>
{
  GET_CLASS_NAME(StringHelper)

  /**
   * Compares two {@link BytesRef}, element by element, and returns the
   * number of elements common to both arrays (from the start of each).
   * This method assumes currentTerm comes after priorTerm.
   *
   * @param priorTerm The first {@link BytesRef} to compare
   * @param currentTerm The second {@link BytesRef} to compare
   * @return The number of common elements (from the start of each).
   */
public:
  static int bytesDifference(std::shared_ptr<BytesRef> priorTerm,
                             std::shared_ptr<BytesRef> currentTerm);

  /**
   * Returns the length of {@code currentTerm} needed for use as a sort key.
   * so that {@link BytesRef#compareTo(BytesRef)} still returns the same result.
   * This method assumes currentTerm comes after priorTerm.
   */
  static int sortKeyLength(std::shared_ptr<BytesRef> priorTerm,
                           std::shared_ptr<BytesRef> currentTerm);

private:
  StringHelper();

  /**
   * Returns <code>true</code> iff the ref starts with the given prefix.
   * Otherwise <code>false</code>.
   *
   * @param ref
   *         the {@code byte[]} to test
   * @param prefix
   *         the expected prefix
   * @return Returns <code>true</code> iff the ref starts with the given prefix.
   *         Otherwise <code>false</code>.
   */
public:
  static bool startsWith(std::deque<char> &ref,
                         std::shared_ptr<BytesRef> prefix);

  /**
   * Returns <code>true</code> iff the ref starts with the given prefix.
   * Otherwise <code>false</code>.
   *
   * @param ref
   *          the {@link BytesRef} to test
   * @param prefix
   *          the expected prefix
   * @return Returns <code>true</code> iff the ref starts with the given prefix.
   *         Otherwise <code>false</code>.
   */
  static bool startsWith(std::shared_ptr<BytesRef> ref,
                         std::shared_ptr<BytesRef> prefix);

  /**
   * Returns <code>true</code> iff the ref ends with the given suffix. Otherwise
   * <code>false</code>.
   *
   * @param ref
   *          the {@link BytesRef} to test
   * @param suffix
   *          the expected suffix
   * @return Returns <code>true</code> iff the ref ends with the given suffix.
   *         Otherwise <code>false</code>.
   */
  static bool endsWith(std::shared_ptr<BytesRef> ref,
                       std::shared_ptr<BytesRef> suffix);

  /** Pass this as the seed to {@link #murmurhash3_x86_32}. */

  // Poached from Guava: set a different salt/seed
  // for each JVM instance, to frustrate hash key collision
  // denial of service attacks, and to catch any places that
  // somehow rely on hash function/order across JVM
  // instances:
  static const int GOOD_FAST_HASH_SEED = 0;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StringHelper::StaticConstructor staticConstructor;

  /** Returns the MurmurHash3_x86_32 hash.
   * Original source/tests at https://github.com/yonik/java_util/
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public static int
  // murmurhash3_x86_32(byte[] data, int offset, int len, int seed)
  static int murmurhash3_x86_32(std::deque<char> &data, int offset, int len,
                                int seed);

  static int murmurhash3_x86_32(std::shared_ptr<BytesRef> bytes, int seed);

  // Holds 128 bit unsigned value:
private:
  static std::shared_ptr<int64_t> nextId;
  static const std::shared_ptr<int64_t> mask128;
  static std::mutex idLock;

  /** length in bytes of an ID */
public:
  static constexpr int ID_LENGTH = 16;

  /** Generates a non-cryptographic globally unique id. */
  static std::deque<char> randomId();

  /**
   * Helper method to render an ID as a string, for debugging
   * <p>
   * Returns the string {@code (null)} if the id is null.
   * Otherwise, returns a string representation for debugging.
   * Never throws an exception. The returned string may
   * indicate if the id is definitely invalid.
   */
  static std::wstring idToString(std::deque<char> &id);

  /** Just converts each int in the incoming {@link IntsRef} to each byte
   *  in the returned {@link BytesRef}, throwing {@code
   * IllegalArgumentException} if any int value is out of bounds for a byte. */
  static std::shared_ptr<BytesRef>
  intsRefToBytesRef(std::shared_ptr<IntsRef> ints);

  /** Compares a fixed length slice of two byte arrays interpreted as
   *  big-endian unsigned values.  Returns positive int if a &gt; b,
   *  negative int if a &lt; b and 0 if a == b
   *
   * @deprecated Use FutureArrays.compareUnsigned instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static int compare(int count, byte[] a,
  // int aOffset, byte[] b, int bOffset)
  static int compare(int count, std::deque<char> &a, int aOffset,
                     std::deque<char> &b, int bOffset);
};

} // #include  "core/src/java/org/apache/lucene/util/
