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
 * {@link Sorter} implementation based on the
 * <a
 * href="http://svn.python.org/projects/python/trunk/Objects/listsort.txt">TimSort</a>
 * algorithm.
 * <p>This implementation is especially good at sorting partially-sorted
 * arrays and sorts small arrays with binary sort.
 * <p><b>NOTE</b>:There are a few differences with the original
 * implementation:<ul> <li><a name="maxTempSlots"></a>The extra amount of memory
 * to perform merges is configurable. This allows small merges to be very fast
 * while large merges will be performed in-place (slightly slower). You can make
 * sure that the fast merge routine will always be used by having
 * <code>maxTempSlots</code> equal to half of the length of the slice of data to
 * sort. <li>Only the fast merge routine can gallop (the one that doesn't run
 * in-place) and it only gallops on the longest slice.
 * </ul>
 * @lucene.internal
 */
class TimSorter : public Sorter
{
  GET_CLASS_NAME(TimSorter)

public:
  static constexpr int MINRUN = 32;
  static constexpr int THRESHOLD = 64;
  static constexpr int STACKSIZE = 49; // depends on MINRUN
  static constexpr int MIN_GALLOP = 7;

  const int maxTempSlots;
  // C++ NOTE: Fields cannot have the same name as methods:
  int minRun_ = 0;
  int to = 0;
  int stackSize = 0;
  std::deque<int> runEnds;

  /**
   * Create a new {@link TimSorter}.
   * @param maxTempSlots the <a href="#maxTempSlots">maximum amount of extra
   * memory to run merges</a>
   */
protected:
  TimSorter(int maxTempSlots);

  /** Minimum run length for an array of length <code>length</code>. */
public:
  static int minRun(int length);

  virtual int runLen(int i);

  virtual int runBase(int i);

  virtual int runEnd(int i);

  virtual void setRunEnd(int i, int runEnd);

  virtual void pushRunLen(int len);

  /** Compute the length of the next run, make the run sorted and return its
   *  length. */
  virtual int nextRun();

  virtual void ensureInvariants();

  virtual void exhaustStack();

  virtual void reset(int from, int to);

  virtual void mergeAt(int n);

  virtual void merge(int lo, int mid, int hi);

  void sort(int from, int to) override;

  void doRotate(int lo, int mid, int hi) override;

  virtual void mergeLo(int lo, int mid, int hi);

  virtual void mergeHi(int lo, int mid, int hi);

  virtual int lowerSaved(int from, int to, int val);

  virtual int upperSaved(int from, int to, int val);

  // faster than lowerSaved when val is at the beginning of [from:to[
  virtual int lowerSaved3(int from, int to, int val);

  // faster than upperSaved when val is at the end of [from:to[
  virtual int upperSaved3(int from, int to, int val);

  /** Copy data from slot <code>src</code> to slot <code>dest</code>. */
protected:
  virtual void copy(int src, int dest) = 0;

  /** Save all elements between slots <code>i</code> and <code>i+len</code>
   *  into the temporary storage. */
  virtual void save(int i, int len) = 0;

  /** Restore element <code>j</code> from the temporary storage into slot
   * <code>i</code>. */
  virtual void restore(int i, int j) = 0;

  /** Compare element <code>i</code> from the temporary storage with element
   *  <code>j</code> from the slice to sort, similarly to
   *  {@link #compare(int, int)}. */
  virtual int compareSaved(int i, int j) = 0;

protected:
  std::shared_ptr<TimSorter> shared_from_this()
  {
    return std::static_pointer_cast<TimSorter>(Sorter::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
