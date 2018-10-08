#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermsEnum;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class CacheKey;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::util::packed
{
class PackedInts;
}
namespace org::apache::lucene::util::packed
{
class Mutable;
}
namespace org::apache::lucene::util
{
class Accountable;
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

namespace org::apache::lucene::index
{

using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

/** Maps per-segment ordinals to/from global ordinal space, using a compact
 * packed-ints representation.
 *
 *  <p><b>NOTE</b>: this is a costly operation, as it must merge sort all terms,
 * and may require non-trivial RAM once done.  It's better to operate in
 *  segment-private ordinal space instead when possible.
 *
 * @lucene.internal */
class OrdinalMap : public std::enable_shared_from_this<OrdinalMap>,
                   public Accountable
{
  GET_CLASS_NAME(OrdinalMap)
  // TODO: we could also have a utility method to merge Terms[] and use size()
  // as a weight when we need it
  // TODO: use more efficient packed ints structures?

private:
  class TermsEnumIndex : public std::enable_shared_from_this<TermsEnumIndex>
  {
    GET_CLASS_NAME(TermsEnumIndex)
  public:
    static std::deque<std::shared_ptr<TermsEnumIndex>> const EMPTY_ARRAY;
    const int subIndex;
    const std::shared_ptr<TermsEnum> termsEnum;
    std::shared_ptr<BytesRef> currentTerm;

    TermsEnumIndex(std::shared_ptr<TermsEnum> termsEnum, int subIndex);

    virtual std::shared_ptr<BytesRef> next() ;
  };

private:
  class SegmentMap : public std::enable_shared_from_this<SegmentMap>,
                     public Accountable
  {
    GET_CLASS_NAME(SegmentMap)
  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(SegmentMap::typeid);

    /** Build a map_obj from an index into a sorted view of `weights` to an index
     * into `weights`. */
    static std::deque<int> map_obj(std::deque<int64_t> &weights);

  private:
    class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
    {
      GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
    private:
      std::deque<int64_t> weights;
      std::deque<int> newToOld;

    public:
      InPlaceMergeSorterAnonymousInnerClass(std::deque<int64_t> &weights,
                                            std::deque<int> &newToOld);

    protected:
      void swap(int i, int j) override;
      int compare(int i, int j) override;

    protected:
      std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
            org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
      }
    };

    /** Inverse the map_obj. */
  private:
    static std::deque<int> inverse(std::deque<int> &map_obj);

    // C++ NOTE: Fields cannot have the same name as methods:
    std::deque<int> const newToOld_, oldToNew_;

  public:
    SegmentMap(std::deque<int64_t> &weights);

    virtual int newToOld(int segment);

    virtual int oldToNew(int segment);

    int64_t ramBytesUsed() override;
  };

  /**
   * Create an ordinal map_obj that uses the number of unique values of each
   * {@link SortedDocValues} instance as a weight.
   * @see #build(IndexReader.CacheKey, TermsEnum[], long[], float)
   */
public:
  static std::shared_ptr<OrdinalMap>
  build(std::shared_ptr<IndexReader::CacheKey> owner,
        std::deque<std::shared_ptr<SortedDocValues>> &values,
        float acceptableOverheadRatio) ;

  /**
   * Create an ordinal map_obj that uses the number of unique values of each
   * {@link SortedSetDocValues} instance as a weight.
   * @see #build(IndexReader.CacheKey, TermsEnum[], long[], float)
   */
  static std::shared_ptr<OrdinalMap>
  build(std::shared_ptr<IndexReader::CacheKey> owner,
        std::deque<std::shared_ptr<SortedSetDocValues>> &values,
        float acceptableOverheadRatio) ;

  /**
   * Creates an ordinal map_obj that allows mapping ords to/from a merged
   * space from <code>subs</code>.
   * @param owner a cache key
   * @param subs TermsEnums that support {@link TermsEnum#ord()}. They need
   *             not be dense (e.g. can be FilteredTermsEnums}.
   * @param weights a weight for each sub. This is ideally correlated with
   *             the number of unique terms that each sub introduces compared
   *             to the other subs
   * @throws IOException if an I/O error occurred.
   */
  static std::shared_ptr<OrdinalMap>
  build(std::shared_ptr<IndexReader::CacheKey> owner,
        std::deque<std::shared_ptr<TermsEnum>> &subs,
        std::deque<int64_t> &weights,
        float acceptableOverheadRatio) ;

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(OrdinalMap::typeid);

  /** Cache key of whoever asked for this awful thing */
public:
  const std::shared_ptr<IndexReader::CacheKey> owner;
  // globalOrd -> (globalOrd - segmentOrd) where segmentOrd is the the ordinal
  // in the first segment that contains this term
  const std::shared_ptr<PackedLongValues> globalOrdDeltas;
  // globalOrd -> first segment container
  const std::shared_ptr<PackedLongValues> firstSegments;
  // for every segment, segmentOrd -> globalOrd
  std::deque<std::shared_ptr<LongValues>> const segmentToGlobalOrds;
  // the map_obj from/to segment ids
  const std::shared_ptr<SegmentMap> segmentMap;
  // ram usage
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t ramBytesUsed_;

  OrdinalMap(std::shared_ptr<IndexReader::CacheKey> owner,
             std::deque<std::shared_ptr<TermsEnum>> &subs,
             std::shared_ptr<SegmentMap> segmentMap,
             float acceptableOverheadRatio) ;

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<TermsEnumIndex>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  private:
    std::shared_ptr<OrdinalMap> outerInstance;

  public:
    PriorityQueueAnonymousInnerClass(std::shared_ptr<OrdinalMap> outerInstance,
                                     std::shared_ptr<UnknownType> length);

  protected:
    bool lessThan(std::shared_ptr<TermsEnumIndex> a,
                  std::shared_ptr<TermsEnumIndex> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util
              .PriorityQueue<TermsEnumIndex>::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<OrdinalMap> outerInstance;

    std::shared_ptr<PackedInts::Mutable> newDeltas;

  public:
    LongValuesAnonymousInnerClass(
        std::shared_ptr<OrdinalMap> outerInstance,
        std::shared_ptr<PackedInts::Mutable> newDeltas);

    int64_t get(int64_t ord) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass2 : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<OrdinalMap> outerInstance;

    std::shared_ptr<PackedLongValues> deltas;

  public:
    LongValuesAnonymousInnerClass2(std::shared_ptr<OrdinalMap> outerInstance,
                                   std::shared_ptr<PackedLongValues> deltas);

    int64_t get(int64_t ord) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass2>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

  /**
   * Given a segment number, return a {@link LongValues} instance that maps
   * segment ordinals to global ordinals.
   */
public:
  virtual std::shared_ptr<LongValues> getGlobalOrds(int segmentIndex);

  /**
   * Given global ordinal, returns the ordinal of the first segment which
   * contains this ordinal (the corresponding to the segment return {@link
   * #getFirstSegmentNumber}).
   */
  virtual int64_t getFirstSegmentOrd(int64_t globalOrd);

  /**
   * Given a global ordinal, returns the index of the first
   * segment that contains this term.
   */
  virtual int getFirstSegmentNumber(int64_t globalOrd);

  /**
   * Returns the total number of unique terms in global ord space.
   */
  virtual int64_t getValueCount();

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;
};

} // namespace org::apache::lucene::index
