#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class Mutable;
}

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

using PackedInts = org::apache::lucene::util::packed::PackedInts;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public class TestTimSorterWorstCase extends
// LuceneTestCase
class TestTimSorterWorstCase : public LuceneTestCase
{

public:
  virtual void testWorstCaseStackSize();

private:
  class TimSorterAnonymousInnerClass : public TimSorter
  {
    GET_CLASS_NAME(TimSorterAnonymousInnerClass)
  private:
    std::shared_ptr<TestTimSorterWorstCase> outerInstance;

    std::shared_ptr<PackedInts::Mutable> arr;

  public:
    TimSorterAnonymousInnerClass(
        std::shared_ptr<TestTimSorterWorstCase> outerInstance,
        std::shared_ptr<PackedInts::Mutable> arr);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

    void save(int i, int len) override;

    void restore(int i, int j) override;

    void copy(int src, int dest) override;

    int compareSaved(int i, int j) override;

  protected:
    std::shared_ptr<TimSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TimSorterAnonymousInnerClass>(
          TimSorter::shared_from_this());
    }
  };

  /** Create an array for the given deque of runs. */
private:
  static std::shared_ptr<PackedInts::Mutable>
  createArray(int length, std::deque<int> &runs);

  /** Create an array that triggers a worst-case sequence of run lens. */
public:
  static std::shared_ptr<PackedInts::Mutable>
  generateWorstCaseArray(int length);

  //
  // Code below is borrowed from
  // https://github.com/abstools/java-timsort-bug/blob/master/TestTimSort.java
  //

  /**
   * Fills <code>runs</code> with a sequence of run lengths of the form<br>
   * Y_n     x_{n,1}   x_{n,2}   ... x_{n,l_n} <br>
   * Y_{n-1} x_{n-1,1} x_{n-1,2} ... x_{n-1,l_{n-1}} <br>
   * ... <br>
   * Y_1     x_{1,1}   x_{1,2}   ... x_{1,l_1}<br>
   * The Y_i's are chosen to satisfy the invariant throughout execution,
   * but the x_{i,j}'s are merged (by <code>TimSort.mergeCollapse</code>)
   * into an X_i that violates the invariant.
   */
private:
  static std::deque<int> runsWorstCase(int length, int minRun);

  /**
   * Adds a sequence x_1, ..., x_n of run lengths to <code>runs</code> such
   * that:<br>
   * 1. X = x_1 + ... + x_n <br>
   * 2. x_j >= minRun for all j <br>
   * 3. x_1 + ... + x_{j-2}  <  x_j  <  x_1 + ... + x_{j-1} for all j <br>
   * These conditions guarantee that TimSort merges all x_j's one by one
   * (resulting in X) using only merges on the second-to-last element.
   * @param X  The sum of the sequence that should be added to runs.
   */
  static void generateWrongElem(int X, int minRun, std::deque<int> &runs);

protected:
  std::shared_ptr<TestTimSorterWorstCase> shared_from_this()
  {
    return std::static_pointer_cast<TestTimSorterWorstCase>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
