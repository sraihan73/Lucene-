#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
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
 * A {@link TimSorter} for object arrays.
 * @lucene.internal
 */
template <typename T>
class ArrayTimSorter final : public TimSorter
{
  GET_CLASS_NAME(ArrayTimSorter)

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final java.util.Comparator<? super T> comparator;
  const std::shared_ptr < Comparator < ? TimSorter T >> comparator;
  std::deque<T> const arr;
  std::deque<T> const tmp;

  /** Create a new {@link ArrayTimSorter}. */
public:
  template <typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public ArrayTimSorter(T[] arr, java.util.Comparator<? super
  // T> comparator, int maxTempSlots)
  ArrayTimSorter(std::deque<T> &arr,
                 std::shared_ptr<Comparator<T1>> comparator, int maxTempSlots)
      : TimSorter(maxTempSlots), comparator(comparator), arr(arr)
  {
    if (maxTempSlots > 0) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked") final T[] tmp = (T[]) new
      // Object[maxTempSlots];
      const std::deque<T> tmp =
          static_cast<std::deque<T>>(std::deque<std::any>(maxTempSlots));
      this->tmp = tmp;
    } else {
      this->tmp.clear();
    }
  }

protected:
  int compare(int i, int j) override
  {
    return comparator->compare(arr[i], arr[j]);
  }

  void swap(int i, int j) override { ArrayUtil::swap(arr, i, j); }

  void copy(int src, int dest) override { arr[dest] = arr[src]; }

  void save(int start, int len) override
  {
    System::arraycopy(arr, start, tmp, 0, len);
  }

  void restore(int src, int dest) override { arr[dest] = tmp[src]; }

  int compareSaved(int i, int j) override
  {
    return comparator->compare(tmp[i], arr[j]);
  }

protected:
  std::shared_ptr<ArrayTimSorter> shared_from_this()
  {
    return std::static_pointer_cast<ArrayTimSorter>(
        TimSorter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
