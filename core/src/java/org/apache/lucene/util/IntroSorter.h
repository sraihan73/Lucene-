#pragma once
#include "stringhelper.h"
#include <memory>

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
 * {@link Sorter} implementation based on a variant of the quicksort algorithm
 * called <a href="http://en.wikipedia.org/wiki/Introsort">introsort</a>: when
 * the recursion level exceeds the log of the length of the array to sort, it
 * falls back to heapsort. This prevents quicksort from running into its
 * worst-case quadratic runtime. Small arrays are sorted with
 * insertion sort.
 * @lucene.internal
 */
class IntroSorter : public Sorter
{
  GET_CLASS_NAME(IntroSorter)

  /** Create a new {@link IntroSorter}. */
public:
  IntroSorter();

  void sort(int from, int to) override final;

  virtual void quicksort(int from, int to, int maxDepth);

  // Don't rely on the slow default impl of setPivot/comparePivot since
  // quicksort relies on these methods to be fast for good performance

protected:
  void setPivot(int i) = 0;
  override override;

  int comparePivot(int j) = 0;
  override override;

  int compare(int i, int j) override;

protected:
  std::shared_ptr<IntroSorter> shared_from_this()
  {
    return std::static_pointer_cast<IntroSorter>(Sorter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
