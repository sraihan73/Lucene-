#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class MutablePointValues;
}

namespace org::apache::lucene::util
{
class Sorter;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class Selector;
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
namespace org::apache::lucene::util::bkd
{

using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Utility APIs for sorting and partitioning buffered points.
 *
 * @lucene.internal */
class MutablePointsReaderUtils final
    : public std::enable_shared_from_this<MutablePointsReaderUtils>
{
  GET_CLASS_NAME(MutablePointsReaderUtils)

public:
  MutablePointsReaderUtils();

  /** Sort the given {@link MutablePointValues} based on its packed value then
   * doc ID. */
  static void sort(int maxDoc, int packedBytesLength,
                   std::shared_ptr<MutablePointValues> reader, int from,
                   int to);

private:
  class MSBRadixSorterAnonymousInnerClass : public MSBRadixSorter
  {
    GET_CLASS_NAME(MSBRadixSorterAnonymousInnerClass)
  private:
    int packedBytesLength = 0;
    std::shared_ptr<MutablePointValues> reader;
    int bitsPerDocId = 0;

  public:
    MSBRadixSorterAnonymousInnerClass(
        int packedBytesLength, int packedBytesLength,
        std::shared_ptr<MutablePointValues> reader, int bitsPerDocId);

  protected:
    void swap(int i, int j) override;

    int byteAt(int i, int k) override;

    std::shared_ptr<org::apache::lucene::util::Sorter>
    getFallbackSorter(int k) override;

  private:
    class IntroSorterAnonymousInnerClass : public IntroSorter
    {
      GET_CLASS_NAME(IntroSorterAnonymousInnerClass)
    private:
      std::shared_ptr<MSBRadixSorterAnonymousInnerClass> outerInstance;

      int k = 0;

    public:
      IntroSorterAnonymousInnerClass(
          std::shared_ptr<MSBRadixSorterAnonymousInnerClass> outerInstance,
          int k);

      const std::shared_ptr<BytesRef> pivot;
      const std::shared_ptr<BytesRef> scratch;
      int pivotDoc = 0;

    protected:
      void swap(int i, int j) override;

      void setPivot(int i) override;

      int comparePivot(int j) override;

    protected:
      std::shared_ptr<IntroSorterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<IntroSorterAnonymousInnerClass>(
            org.apache.lucene.util.IntroSorter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MSBRadixSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MSBRadixSorterAnonymousInnerClass>(
          org.apache.lucene.util.MSBRadixSorter::shared_from_this());
    }
  };

  /** Sort points on the given dimension. */
public:
  static void sortByDim(int sortedDim, int bytesPerDim,
                        std::deque<int> &commonPrefixLengths,
                        std::shared_ptr<MutablePointValues> reader, int from,
                        int to, std::shared_ptr<BytesRef> scratch1,
                        std::shared_ptr<BytesRef> scratch2);

private:
  class IntroSorterAnonymousInnerClass2 : public IntroSorter
  {
    GET_CLASS_NAME(IntroSorterAnonymousInnerClass2)
  private:
    std::shared_ptr<MutablePointValues> reader;
    std::shared_ptr<BytesRef> scratch1;
    std::shared_ptr<BytesRef> scratch2;
    int offset = 0;
    int numBytesToCompare = 0;

  public:
    IntroSorterAnonymousInnerClass2(std::shared_ptr<MutablePointValues> reader,
                                    std::shared_ptr<BytesRef> scratch1,
                                    std::shared_ptr<BytesRef> scratch2,
                                    int offset, int numBytesToCompare);

    const std::shared_ptr<BytesRef> pivot = scratch1;
    int pivotDoc = -1;

  protected:
    void swap(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  protected:
    std::shared_ptr<IntroSorterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<IntroSorterAnonymousInnerClass2>(
          org.apache.lucene.util.IntroSorter::shared_from_this());
    }
  };

  /** Partition points around {@code mid}. All values on the left must be less
   *  than or equal to it and all values on the right must be greater than or
   *  equal to it. */
public:
  static void partition(int maxDoc, int splitDim, int bytesPerDim,
                        int commonPrefixLen,
                        std::shared_ptr<MutablePointValues> reader, int from,
                        int to, int mid, std::shared_ptr<BytesRef> scratch1,
                        std::shared_ptr<BytesRef> scratch2);

private:
  class RadixSelectorAnonymousInnerClass : public RadixSelector
  {
    GET_CLASS_NAME(RadixSelectorAnonymousInnerClass)
  private:
    std::shared_ptr<MutablePointValues> reader;
    std::shared_ptr<BytesRef> scratch1;
    std::shared_ptr<BytesRef> scratch2;
    int offset = 0;
    int cmpBytes = 0;
    int bitsPerDocId = 0;

  public:
    RadixSelectorAnonymousInnerClass(int cmpBytes,
                                     std::shared_ptr<MutablePointValues> reader,
                                     std::shared_ptr<BytesRef> scratch1,
                                     std::shared_ptr<BytesRef> scratch2,
                                     int offset, int cmpBytes,
                                     int bitsPerDocId);

  protected:
    std::shared_ptr<Selector> getFallbackSelector(int k) override;

  private:
    class IntroSelectorAnonymousInnerClass : public IntroSelector
    {
      GET_CLASS_NAME(IntroSelectorAnonymousInnerClass)
    private:
      std::shared_ptr<RadixSelectorAnonymousInnerClass> outerInstance;

      int k = 0;

    public:
      IntroSelectorAnonymousInnerClass(
          std::shared_ptr<RadixSelectorAnonymousInnerClass> outerInstance,
          int k);

      const std::shared_ptr<BytesRef> pivot;
      int pivotDoc = 0;

    protected:
      void swap(int i, int j) override;

      void setPivot(int i) override;

      int comparePivot(int j) override;

    protected:
      std::shared_ptr<IntroSelectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<IntroSelectorAnonymousInnerClass>(
            org.apache.lucene.util.IntroSelector::shared_from_this());
      }
    };

  protected:
    void swap(int i, int j) override;

    int byteAt(int i, int k) override;

  protected:
    std::shared_ptr<RadixSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RadixSelectorAnonymousInnerClass>(
          org.apache.lucene.util.RadixSelector::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::util::bkd
