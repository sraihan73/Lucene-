#pragma once
#include "stringhelper.h"
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
 * An {@link IntroSorter} for object arrays.
 * @lucene.internal
 */
template <typename T>
class ArrayIntroSorter final : public IntroSorter
{
  GET_CLASS_NAME(ArrayIntroSorter)

private:
  std::deque<T> const arr;
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final java.util.Comparator<? super T> comparator;
  const std::shared_ptr < Comparator < ? IntroSorter T >> comparator;
  T pivot;

  /** Create a new {@link ArrayInPlaceMergeSorter}. */
public:
  template <typename T1>
  // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
  // ORIGINAL LINE: public ArrayIntroSorter(T[] arr, java.util.Comparator<?
  // super T> comparator)
  ArrayIntroSorter(std::deque<T> &arr,
                   std::shared_ptr<Comparator<T1>> comparator)
      : arr(arr), comparator(comparator)
  {
    pivot = nullptr;
  }

protected:
  int compare(int i, int j) override
  {
    return comparator->compare(arr[i], arr[j]);
  }

  void swap(int i, int j) override { ArrayUtil::swap(arr, i, j); }

  void setPivot(int i) override { pivot = arr[i]; }

  int comparePivot(int i) override
  {
    return comparator->compare(pivot, arr[i]);
  }

protected:
  std::shared_ptr<ArrayIntroSorter> shared_from_this()
  {
    return std::static_pointer_cast<ArrayIntroSorter>(
        IntroSorter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
