#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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

/** An implementation of a selection algorithm, ie. computing the k-th greatest
 *  value from a collection. */
class Selector : public std::enable_shared_from_this<Selector>
{
  GET_CLASS_NAME(Selector)

  /** Reorder elements so that the element at position {@code k} is the same
   *  as if all elements were sorted and all other elements are partitioned
   *  around it: {@code [from, k)} only contains elements that are less than
   *  or equal to {@code k} and {@code (k, to)} only contains elements that
   *  are greater than or equal to {@code k}. */
public:
  virtual void select(int from, int to, int k) = 0;

  virtual void checkArgs(int from, int to, int k);

  /** Swap values at slots <code>i</code> and <code>j</code>. */
protected:
  virtual void swap(int i, int j) = 0;
};

} // namespace org::apache::lucene::util
