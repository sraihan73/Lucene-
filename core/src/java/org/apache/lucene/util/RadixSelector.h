#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Selector.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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

/** Radix selector.
 *  <p>This implementation works similarly to a MSB radix sort except that it
 *  only recurses into the sub partition that contains the desired value.
 *  @lucene.internal */
class RadixSelector : public Selector
{
  GET_CLASS_NAME(RadixSelector)

  // after that many levels of recursion we fall back to introselect anyway
  // this is used as a protection against the fact that radix sort performs
  // worse when there are long common prefixes (probably because of cache
  // locality)
private:
  static constexpr int LEVEL_THRESHOLD = 8;
  // size of histograms: 256 + 1 to indicate that the string is finished
  static constexpr int HISTOGRAM_SIZE = 257;
  // buckets below this size will be sorted with introselect
  static constexpr int LENGTH_THRESHOLD = 100;

  // we store one histogram per recursion level
  std::deque<int> const histogram = std::deque<int>(HISTOGRAM_SIZE);
  std::deque<int> const commonPrefix;

  const int maxLength;

  /**
   * Sole constructor.
   * @param maxLength the maximum length of keys, pass {@link Integer#MAX_VALUE}
   * if unknown.
   */
protected:
  RadixSelector(int maxLength);

  /** Return the k-th byte of the entry at index {@code i}, or {@code -1} if
   * its length is less than or equal to {@code k}. This may only be called
   * with a value of {@code i} between {@code 0} included and
   * {@code maxLength} excluded. */
  virtual int byteAt(int i, int k) = 0;

  /** Get a fall-back selector which may assume that the first {@code d} bytes
   *  of all compared strings are equal. This fallback selector is used when
   *  the range becomes narrow or when the maximum level of recursion has
   *  been exceeded. */
  virtual std::shared_ptr<Selector> getFallbackSelector(int d);

private:
  class IntroSelectorAnonymousInnerClass : public IntroSelector
  {
    GET_CLASS_NAME(IntroSelectorAnonymousInnerClass)
  private:
    std::shared_ptr<RadixSelector> outerInstance;

    int d = 0;

  public:
    IntroSelectorAnonymousInnerClass(
        std::shared_ptr<RadixSelector> outerInstance, int d);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  private:
    const std::shared_ptr<BytesRefBuilder> pivot;

  protected:
    std::shared_ptr<IntroSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSelectorAnonymousInnerClass>(
          IntroSelector::shared_from_this());
    }
  };

public:
  void select(int from, int to, int k) override;

private:
  void select(int from, int to, int k, int d, int l);

  /**
   * @param d the character number to compare
   * @param l the level of recursion
   */
  void radixSelect(int from, int to, int k, int d, int l);

  // only used from assert
  bool assertHistogram(int commonPrefixLength, std::deque<int> &histogram);

  /** Return a number for the k-th character between 0 and {@link
   * #HISTOGRAM_SIZE}. */
  int getBucket(int i, int k);

  /** Build a histogram of the number of values per {@link #getBucket(int, int)
   * bucket} and return a common prefix length for all visited values.
   *  @see #buildHistogram */
  int computeCommonPrefixLengthAndBuildHistogram(int from, int to, int k,
                                                 std::deque<int> &histogram);

  /** Build an histogram of the k-th characters of values occurring between
   *  offsets {@code from} and {@code to}, using {@link #getBucket}. */
  void buildHistogram(int from, int to, int k, std::deque<int> &histogram);

  /** Reorder elements so that all of them that fall into {@code bucket} are
   *  between offsets {@code bucketFrom} and {@code bucketTo}. */
  void partition(int from, int to, int bucket, int bucketFrom, int bucketTo,
                 int d);

protected:
  std::shared_ptr<RadixSelector> shared_from_this()
  {
    return std::static_pointer_cast<RadixSelector>(
        Selector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
