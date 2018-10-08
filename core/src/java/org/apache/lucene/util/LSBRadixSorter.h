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
 * A LSB Radix sorter for unsigned int values.
 * @lucene.internal
 */
class LSBRadixSorter final : public std::enable_shared_from_this<LSBRadixSorter>
{
  GET_CLASS_NAME(LSBRadixSorter)

private:
  static constexpr int INSERTION_SORT_THRESHOLD = 30;
  static constexpr int HISTOGRAM_SIZE = 256;

  std::deque<int> const histogram = std::deque<int>(HISTOGRAM_SIZE);
  std::deque<int> buffer = std::deque<int>(0);

  static void buildHistogram(std::deque<int> &array_, int len,
                             std::deque<int> &histogram, int shift);

  static void sumHistogram(std::deque<int> &histogram);

  static void reorder(std::deque<int> &array_, int len,
                      std::deque<int> &histogram, int shift,
                      std::deque<int> &dest);

  static bool sort(std::deque<int> &array_, int len,
                   std::deque<int> &histogram, int shift,
                   std::deque<int> &dest);

  static void insertionSort(std::deque<int> &array_, int off, int len);

  /** Sort {@code array[0:len]} in place.
   * @param numBits how many bits are required to store any of the values in
   *                {@code array[0:len]}. Pass {@code 32} if unknown. */
public:
  void sort(int numBits, std::deque<int> &array_, int len);
};

} // namespace org::apache::lucene::util
