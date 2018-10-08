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

/** Implementation of the quick select algorithm.
 *  <p>It uses the median of the first, middle and last values as a pivot and
 *  falls back to a heap sort when the number of recursion levels exceeds
 *  {@code 2 lg(n)}, as a consequence it runs in linear time on average and in
 *  {@code n log(n)} time in the worst case.</p>
 *  @lucene.internal */
class IntroSelector : public Selector
{
  GET_CLASS_NAME(IntroSelector)

public:
  void select(int from, int to, int k) override final;

  // heap sort
  // TODO: use median of median instead to have linear worst-case rather than
  // n*log(n)
  virtual void slowSelect(int from, int to, int k);

private:
  class SorterAnonymousInnerClass : public Sorter
  {
    GET_CLASS_NAME(SorterAnonymousInnerClass)
  private:
    std::shared_ptr<IntroSelector> outerInstance;

    int from = 0;
    int to = 0;

  public:
    SorterAnonymousInnerClass(std::shared_ptr<IntroSelector> outerInstance,
                              int from, int to);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  public:
    void sort(int from, int to) override;

  protected:
    std::shared_ptr<SorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SorterAnonymousInnerClass>(
          Sorter::shared_from_this());
    }
  };

private:
  void quickSelect(int from, int to, int k, int maxDepth);

  /** Compare entries found in slots <code>i</code> and <code>j</code>.
   *  The contract for the returned value is the same as
   *  {@link Comparator#compare(Object, Object)}. */
protected:
  virtual int compare(int i, int j);

  /** Save the value at slot <code>i</code> so that it can later be used as a
   * pivot, see {@link #comparePivot(int)}. */
  virtual void setPivot(int i) = 0;

  /** Compare the pivot with the slot at <code>j</code>, similarly to
   *  {@link #compare(int, int) compare(i, j)}. */
  virtual int comparePivot(int j) = 0;

protected:
  std::shared_ptr<IntroSelector> shared_from_this()
  {
    return std::static_pointer_cast<IntroSelector>(
        Selector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
