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

/** {@link Sorter} implementation based on the merge-sort algorithm that merges
 *  in place (no extra memory will be allocated). Small arrays are sorted with
 *  insertion sort.
 *  @lucene.internal */
class InPlaceMergeSorter : public Sorter
{
  GET_CLASS_NAME(InPlaceMergeSorter)

  /** Create a new {@link InPlaceMergeSorter} */
public:
  InPlaceMergeSorter();

  void sort(int from, int to) override final;

  virtual void mergeSort(int from, int to);

protected:
  std::shared_ptr<InPlaceMergeSorter> shared_from_this()
  {
    return std::static_pointer_cast<InPlaceMergeSorter>(
        Sorter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
