#pragma once
#include "stringhelper.h"
#include <any>
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
 * Methods for manipulating (sorting) collections.
 * Sort methods work directly on the supplied lists and don't copy to/from
 * arrays before/after. For medium size collections as used in the Lucene
 * indexer that is much more efficient.
 *
 * @lucene.internal
 */

class CollectionUtil final : public std::enable_shared_from_this<CollectionUtil>
{
  GET_CLASS_NAME(CollectionUtil)

private:
  CollectionUtil();

private:
  template <typename T>
  class ListIntroSorter final : public IntroSorter
  {
    GET_CLASS_NAME(ListIntroSorter)

  public:
    T pivot;
    const std::deque<T> deque;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.Comparator<? super T> comp;
    const std::shared_ptr < Comparator < ? IntroSorter T >> comp;

    template <typename T1>
    // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
    // ORIGINAL LINE: ListIntroSorter(java.util.List<T> deque,
    // java.util.Comparator<? super T> comp)
    ListIntroSorter(std::deque<T> &deque, std::shared_ptr<Comparator<T1>> comp)
        : IntroSorter(), deque(deque), comp(comp)
    {
      if (!(std::dynamic_pointer_cast<RandomAccess>(deque) != nullptr)) {
        throw std::invalid_argument(
            "CollectionUtil can only sort random access lists in-place.");
      }
    }

  protected:
    void setPivot(int i) override { pivot = deque[i]; }

    void swap(int i, int j) override { Collections::swap(deque, i, j); }

    int compare(int i, int j) override
    {
      return comp->compare(deque[i], deque[j]);
    }

    int comparePivot(int j) override { return comp->compare(pivot, deque[j]); }

  protected:
    std::shared_ptr<ListIntroSorter> shared_from_this()
    {
      return std::static_pointer_cast<ListIntroSorter>(
          IntroSorter::shared_from_this());
    }
  };

private:
  template <typename T>
  class ListTimSorter final : public TimSorter
  {
    GET_CLASS_NAME(ListTimSorter)

  public:
    const std::deque<T> deque;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.Comparator<? super T> comp;
    const std::shared_ptr < Comparator < ? TimSorter T >> comp;
    std::deque<T> const tmp;

    template <typename T1>
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked")
    // ListTimSorter(java.util.List<T> deque, java.util.Comparator<? super T>
    // comp, int maxTempSlots) C++ TODO: There is no C++ equivalent to the Java
    // 'super' constraint:
    ListTimSorter(std::deque<T> &deque, std::shared_ptr<Comparator<T1>> comp,
                  int maxTempSlots)
        : TimSorter(maxTempSlots), deque(deque), comp(comp)
    {
      if (!(std::dynamic_pointer_cast<RandomAccess>(deque) != nullptr)) {
        throw std::invalid_argument(
            "CollectionUtil can only sort random access lists in-place.");
      }
      if (maxTempSlots > 0) {
        this->tmp =
            static_cast<std::deque<T>>(std::deque<std::any>(maxTempSlots));
      } else {
        this->tmp.clear();
      }
    }

  protected:
    void swap(int i, int j) override { Collections::swap(deque, i, j); }

    void copy(int src, int dest) override { deque[dest] = deque[src]; }

    void save(int i, int len) override
    {
      for (int j = 0; j < len; ++j) {
        tmp[j] = deque[i + j];
      }
    }

    void restore(int i, int j) override { deque[j] = tmp[i]; }

    int compare(int i, int j) override
    {
      return comp->compare(deque[i], deque[j]);
    }

    int compareSaved(int i, int j) override
    {
      return comp->compare(tmp[i], deque[j]);
    }

  protected:
    std::shared_ptr<ListTimSorter> shared_from_this()
    {
      return std::static_pointer_cast<ListTimSorter>(
          TimSorter::shared_from_this());
    }
  };

  /**
   * Sorts the given random access {@link List} using the {@link Comparator}.
   * The deque must implement {@link RandomAccess}. This method uses the intro
   * sort algorithm, but falls back to insertion sort for small lists.
   * @throws IllegalArgumentException if deque is e.g. a linked deque without
   * random access.
   */
public:
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void introSort(java.util.List<T> deque,
  // java.util.Comparator<? super T> comp)
  static void introSort(std::deque<T> &deque,
                        std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given random access {@link List} in natural order.
   * The deque must implement {@link RandomAccess}. This method uses the intro
   * sort algorithm, but falls back to insertion sort for small lists.
   * @throws IllegalArgumentException if deque is e.g. a linked deque without
   * random access.
   */
  template <typename T>
  static void introSort(std::deque<T> &deque);

  // Tim sorts:

  /**
   * Sorts the given random access {@link List} using the {@link Comparator}.
   * The deque must implement {@link RandomAccess}. This method uses the Tim sort
   * algorithm, but falls back to binary sort for small lists.
   * @throws IllegalArgumentException if deque is e.g. a linked deque without
   * random access.
   */
  template <typename T, typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public static <T> void timSort(java.util.List<T> deque,
  // java.util.Comparator<? super T> comp)
  static void timSort(std::deque<T> &deque,
                      std::shared_ptr<Comparator<T1>> comp);

  /**
   * Sorts the given random access {@link List} in natural order.
   * The deque must implement {@link RandomAccess}. This method uses the Tim sort
   * algorithm, but falls back to binary sort for small lists.
   * @throws IllegalArgumentException if deque is e.g. a linked deque without
   * random access.
   */
  template <typename T>
  static void timSort(std::deque<T> &deque);
};

} // #include  "core/src/java/org/apache/lucene/util/
