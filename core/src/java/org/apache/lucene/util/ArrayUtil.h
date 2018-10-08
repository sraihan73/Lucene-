#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
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

/**
 * Methods for manipulating arrays.
 *
 * @lucene.internal
 */

class ArrayUtil final : public std::enable_shared_from_this<ArrayUtil>
{
  GET_CLASS_NAME(ArrayUtil)

  /** Maximum length for an array (Integer.MAX_VALUE -
   * RamUsageEstimator.NUM_BYTES_ARRAY_HEADER). */
public:
  static const int MAX_ARRAY_LENGTH = std::numeric_limits<int>::max() -
                                      RamUsageEstimator::NUM_BYTES_ARRAY_HEADER;

private:
  ArrayUtil();

  /*
     Begin Apache Harmony code

     Revision taken on Friday, June 12.
     https://svn.apache.org/repos/asf/harmony/enhanced/classlib/archive/java6/modules/luni/src/main/java/java/lang/Integer.java

   */

  /**
   * Parses a char array into an int.
   * @param chars the character array
   * @param offset The offset into the array
   * @param len The length
   * @return the int
   * @throws NumberFormatException if it can't parse
   */
public:
  static int parseInt(std::deque<wchar_t> &chars, int offset,
                      int len) ;

  /**
   * Parses the string argument as if it was an int value and returns the
   * result. Throws NumberFormatException if the string does not represent an
   * int quantity. The second argument specifies the radix to use when parsing
   * the value.
   *
   * @param chars a string representation of an int quantity.
   * @param radix the base to use for conversion.
   * @return int the value represented by the argument
   * @throws NumberFormatException if the argument could not be parsed as an int
   * quantity.
   */
  static int parseInt(std::deque<wchar_t> &chars, int offset, int len,
                      int radix) ;

private:
  static int parse(std::deque<wchar_t> &chars, int offset, int len, int radix,
                   bool negative) ;

  /*

 END APACHE HARMONY CODE
  */

  /** Returns an array size &gt;= minTargetSize, generally
   *  over-allocating exponentially to achieve amortized
   *  linear-time cost as the array grows.
   *
   *  NOTE: this was originally borrowed from Python 2.4.2
   *  listobject.c sources (attribution in LICENSE.txt), but
   *  has now been substantially changed based on
   *  discussions from java-dev thread with subject "Dynamic
   *  array reallocation algorithms", started on Jan 12
   *  2010.
   *
   * @param minTargetSize Minimum required value to be returned.
   * @param bytesPerElement Bytes used by each element of
   * the array.  See constants in {@link RamUsageEstimator}.
   *
   * @lucene.internal
   */

public:
  static int oversize(int minTargetSize, int bytesPerElement);

  template <typename T>
  static std::deque<T> grow(std::deque<T> &array_, int minSize);

  static std::deque<short> grow(std::deque<short> &array_, int minSize);

  static std::deque<short> grow(std::deque<short> &array_);

  static std::deque<float> grow(std::deque<float> &array_, int minSize);

  static std::deque<float> grow(std::deque<float> &array_);

  static std::deque<double> grow(std::deque<double> &array_, int minSize);

  static std::deque<double> grow(std::deque<double> &array_);

  static std::deque<int> grow(std::deque<int> &array_, int minSize);

  static std::deque<int> grow(std::deque<int> &array_);

  static std::deque<int64_t> grow(std::deque<int64_t> &array_,
                                     int minSize);

  static std::deque<int64_t> grow(std::deque<int64_t> &array_);

  static std::deque<char> grow(std::deque<char> &array_, int minSize);

  static std::deque<char> grow(std::deque<char> &array_);

  static std::deque<wchar_t> grow(std::deque<wchar_t> &array_, int minSize);

  static std::deque<wchar_t> grow(std::deque<wchar_t> &array_);

  /**
   * Returns hash of chars in range start (inclusive) to
   * end (inclusive)
   */
  static int hashCode(std::deque<wchar_t> &array_, int start, int end);

  /** Swap values stored in slots <code>i</code> and <code>j</code> */
  template <typename T>
  static void swap(std::deque<T> &arr, int i, int j);

  // intro-sorts

  /**
   * Sorts the given array slice using the {@link Comparator}. This method uses
   * the intro sort algorithm, but falls back to insertion sort for small
   * arrays.
   * @param fromIndex start index (inclusive)
   * @param toIndex end index (exclusive)
   */
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void introSort(T[] a, int fromIndex, int
  // toIndex, java.util.Comparator<? super T> comp)
  static void introSort(std::deque<T> &a, int fromIndex, int toIndex,
                        std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given array using the {@link Comparator}. This method uses the
   * intro sort algorithm, but falls back to insertion sort for small arrays.
   */
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void introSort(T[] a,
  // java.util.Comparator<? super T> comp)
  static void introSort(std::deque<T> &a,
                        std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given array slice in natural order. This method uses the intro
   * sort algorithm, but falls back to insertion sort for small arrays.
   * @param fromIndex start index (inclusive)
   * @param toIndex end index (exclusive)
   */
  template <typename T>
  static void introSort(std::deque<T> &a, int fromIndex, int toIndex);

  /**
   * Sorts the given array in natural order. This method uses the intro sort
   * algorithm, but falls back to insertion sort for small arrays.
   */
  template <typename T>
  static void introSort(std::deque<T> &a);

  // tim sorts:

  /**
   * Sorts the given array slice using the {@link Comparator}. This method uses
   * the Tim sort algorithm, but falls back to binary sort for small arrays.
   * @param fromIndex start index (inclusive)
   * @param toIndex end index (exclusive)
   */
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void timSort(T[] a, int fromIndex, int
  // toIndex, java.util.Comparator<? super T> comp)
  static void timSort(std::deque<T> &a, int fromIndex, int toIndex,
                      std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given array using the {@link Comparator}. This method uses the
   * Tim sort algorithm, but falls back to binary sort for small arrays.
   */
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void timSort(T[] a, java.util.Comparator<?
  // super T> comp)
  static void timSort(std::deque<T> &a, std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given array slice in natural order. This method uses the Tim sort
   * algorithm, but falls back to binary sort for small arrays.
   * @param fromIndex start index (inclusive)
   * @param toIndex end index (exclusive)
   */
  template <typename T>
  static void timSort(std::deque<T> &a, int fromIndex, int toIndex);

  /**
   * Sorts the given array in natural order. This method uses the Tim sort
   * algorithm, but falls back to binary sort for small arrays.
   */
  template <typename T>
  static void timSort(std::deque<T> &a);

  /** Reorganize {@code arr[from:to[} so that the element at offset k is at the
   *  same position as if {@code arr[from:to[} was sorted, and all elements on
   *  its left are less than or equal to it, and all elements on its right are
   *  greater than or equal to it.
   *  This runs in linear time on average and in {@code n log(n)} time in the
   *  worst case.*/
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void select(T[] arr, int from, int to, int
  // k, java.util.Comparator<? super T> comparator)
  static void select(std::deque<T> &arr, int from, int to, int k,
                     std::shared_ptr<Comparator<T1>> comparator);

private:
  class IntroSelectorAnonymousInnerClass : public IntroSelector
  {
    GET_CLASS_NAME(IntroSelectorAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<T>> arr;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private java.util.Comparator<? super T> comparator;
    std::shared_ptr < Comparator < ? IntroSelector T >> comparator;

  public:
    template <typename T1>
    // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
    // ORIGINAL LINE: public IntroSelectorAnonymousInnerClass(std::deque<T>
    // arr, java.util.Comparator<? super T> comparator)
    IntroSelectorAnonymousInnerClass(
        std::deque<std::shared_ptr<T>> &arr,
        std::shared_ptr<Comparator<T1>> comparator);

    std::shared_ptr<T> pivot;

  protected:
    void swap(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  protected:
    std::shared_ptr<IntroSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSelectorAnonymousInnerClass>(
          IntroSelector::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
