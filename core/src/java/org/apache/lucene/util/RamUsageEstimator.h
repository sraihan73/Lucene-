#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
 * Estimates the size (memory representation) of Java objects.
 * <p>
 * This class uses assumptions that were discovered for the Hotspot
 * virtual machine. If you use a non-OpenJDK/Oracle-based JVM,
 * the measurements may be slightly wrong.
 *
 * @see #shallowSizeOf(Object)
 * @see #shallowSizeOfInstance(Class)
 *
 * @lucene.internal
 */
class RamUsageEstimator final
    : public std::enable_shared_from_this<RamUsageEstimator>
{
  GET_CLASS_NAME(RamUsageEstimator)

  /** One kilobyte bytes. */
public:
  static constexpr int64_t ONE_KB = 1024;

  /** One megabyte bytes. */
  static const int64_t ONE_MB = ONE_KB * ONE_KB;

  /** One gigabyte bytes.*/
  static const int64_t ONE_GB = ONE_KB * ONE_MB;

  /** No instantiation. */
private:
  RamUsageEstimator();

  /**
   * Number of bytes used to represent a {@code bool} in binary form
   * @deprecated use {@code 1} instead.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_BOOLEAN = 1;
  static constexpr int NUM_BYTES_BOOLEAN = 1;
  /**
   * Number of bytes used to represent a {@code byte} in binary form
   * @deprecated use {@code 1} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_BYTE = 1;
  static constexpr int NUM_BYTES_BYTE = 1;
  /**
   * Number of bytes used to represent a {@code char} in binary form
   * @deprecated use {@link Character#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_CHAR =
  // Character.BYTES;
  static const int NUM_BYTES_CHAR = Character::BYTES;
  /**
   * Number of bytes used to represent a {@code short} in binary form
   * @deprecated use {@link Short#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_SHORT =
  // Short.BYTES;
  static const int NUM_BYTES_SHORT = Short::BYTES;
  /**
   * Number of bytes used to represent an {@code int} in binary form
   * @deprecated use {@link Integer#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_INT =
  // Integer.BYTES;
  static const int NUM_BYTES_INT = Integer::BYTES;
  /**
   * Number of bytes used to represent a {@code float} in binary form
   * @deprecated use {@link Float#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_FLOAT =
  // Float.BYTES;
  static const int NUM_BYTES_FLOAT = Float::BYTES;
  /**
   * Number of bytes used to represent a {@code long} in binary form
   * @deprecated use {@link Long#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_LONG =
  // Long.BYTES;
  static const int NUM_BYTES_LONG = Long::BYTES;
  /**
   * Number of bytes used to represent a {@code double} in binary form
   * @deprecated use {@link Double#BYTES} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static int NUM_BYTES_DOUBLE =
  // Double.BYTES;
  static const int NUM_BYTES_DOUBLE = Double::BYTES;

  /**
   * True, iff compressed references (oops) are enabled by this JVM
   */
  static const bool COMPRESSED_REFS_ENABLED = false;

  /**
   * Number of bytes this JVM uses to represent an object reference.
   */
  static const int NUM_BYTES_OBJECT_REF = 0;

  /**
   * Number of bytes to represent an object header (no fields, no alignments).
   */
  static const int NUM_BYTES_OBJECT_HEADER = 0;

  /**
   * Number of bytes to represent an array header (no content, but with
   * alignments).
   */
  static const int NUM_BYTES_ARRAY_HEADER = 0;

  /**
   * A constant specifying the object alignment boundary inside the JVM. Objects
   * will always take a full multiple of this constant, possibly wasting some
   * space.
   */
  static const int NUM_BYTES_OBJECT_ALIGNMENT = 0;

  /**
   * Sizes of primitive classes.
   */
private:
  static const std::unordered_map<std::type_info, int> primitiveSizes;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static RamUsageEstimator::StaticConstructor staticConstructor;

  /**
   * JVMs typically cache small longs. This tries to find out what the range is.
   */
public:
  static const int64_t LONG_CACHE_MIN_VALUE = 0, LONG_CACHE_MAX_VALUE = 0;
  static const int LONG_SIZE = 0;

  /** For testing only */
  static const bool JVM_IS_HOTSPOT_64BIT = false;

  static const std::wstring MANAGEMENT_FACTORY_CLASS;
  static const std::wstring HOTSPOT_BEAN_CLASS;

  /**
   * Initialize constants and try to collect information about the JVM
   * internals.
   */

  /**
   * Aligns an object size to be the next multiple of {@link
   * #NUM_BYTES_OBJECT_ALIGNMENT}.
   */
  static int64_t alignObjectSize(int64_t size);

  /**
   * Return the size of the provided {@link Long} object, returning 0 if it is
   * cached by the JVM and its shallow size otherwise.
   */
  static int64_t sizeOf(std::optional<int64_t> &value);

  /** Returns the size in bytes of the byte[] object. */
  static int64_t sizeOf(std::deque<char> &arr);

  /** Returns the size in bytes of the bool[] object. */
  static int64_t sizeOf(std::deque<bool> &arr);

  /** Returns the size in bytes of the char[] object. */
  static int64_t sizeOf(std::deque<wchar_t> &arr);

  /** Returns the size in bytes of the short[] object. */
  static int64_t sizeOf(std::deque<short> &arr);

  /** Returns the size in bytes of the int[] object. */
  static int64_t sizeOf(std::deque<int> &arr);

  /** Returns the size in bytes of the float[] object. */
  static int64_t sizeOf(std::deque<float> &arr);

  /** Returns the size in bytes of the long[] object. */
  static int64_t sizeOf(std::deque<int64_t> &arr);

  /** Returns the size in bytes of the double[] object. */
  static int64_t sizeOf(std::deque<double> &arr);

  /** Returns the shallow size in bytes of the Object[] object. */
  // Use this method instead of #shallowSizeOf(Object) to avoid costly
  // reflection
  static int64_t shallowSizeOf(std::deque<std::any> &arr);

  /**
   * Estimates a "shallow" memory usage of the given object. For arrays, this
   * will be the memory taken by array storage (no subreferences will be
   * followed). For objects, this will be the memory taken by the fields.
   *
   * JVM object alignments are also applied.
   */
  static int64_t shallowSizeOf(std::any obj);

  /**
   * Returns the shallow instance size in bytes an instance of the given class
   * would occupy. This works with all conventional classes and primitive types,
   * but not with arrays (the size then depends on the number of elements and
   * varies from object to object).
   *
   * @see #shallowSizeOf(Object)
   * @throws IllegalArgumentException if {@code clazz} is an array class.
   */
  static int64_t shallowSizeOfInstance(std::type_info clazz);

private:
  class PrivilegedActionAnonymousInnerClass
      : public PrivilegedAction<std::deque<std::shared_ptr<Field>>>
  {
    GET_CLASS_NAME(PrivilegedActionAnonymousInnerClass)
  private:
    std::type_info target;

  public:
    PrivilegedActionAnonymousInnerClass(std::type_info target);

    std::deque<std::shared_ptr<Field>> run() override;

  protected:
    std::shared_ptr<PrivilegedActionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PrivilegedActionAnonymousInnerClass>(
          java.security.PrivilegedAction<Field[]>::shared_from_this());
    }
  };

  /**
   * Return shallow size of any <code>array</code>.
   */
private:
  static int64_t shallowSizeOfArray(std::any array_);

  /**
   * This method returns the maximum representation size of an object.
   * <code>sizeSoFar</code> is the object's size measured so far. <code>f</code>
   * is the field being probed.
   *
   * <p>The returned offset will be the maximum of whatever was measured so far
   * and <code>f</code> field's offset and representation size (unaligned).
   */
public:
  static int64_t adjustForField(int64_t sizeSoFar,
                                  std::shared_ptr<Field> f);

  /**
   * Returns <code>size</code> in human-readable units (GB, MB, KB or bytes).
   */
  static std::wstring humanReadableUnits(int64_t bytes);

  /**
   * Returns <code>size</code> in human-readable units (GB, MB, KB or bytes).
   */
  static std::wstring humanReadableUnits(int64_t bytes,
                                         std::shared_ptr<DecimalFormat> df);

  /**
   * Return the size of the provided array of {@link Accountable}s by summing
   * up the shallow size of the array and the
   * {@link Accountable#ramBytesUsed() memory usage} reported by each
   * {@link Accountable}.
   */
  static int64_t
  sizeOf(std::deque<std::shared_ptr<Accountable>> &accountables);
};

} // #include  "core/src/java/org/apache/lucene/util/
