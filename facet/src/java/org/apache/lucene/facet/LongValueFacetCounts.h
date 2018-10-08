#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>


// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class FacetsCollector;
}

namespace org::apache::lucene::search
{
class LongValuesSource;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::facet
{
class MatchingDocs;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::facet
{
class FacetResult;
}
namespace org::apache::lucene::facet
{
class LabelAndValue;
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

namespace org::apache::lucene::facet
{

using com::carrotsearch::hppc::LongIntScatterMap;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using IndexReader = org::apache::lucene::index::IndexReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;

/** {@link Facets} implementation that computes counts for
 *  all uniqute long values, more efficiently counting small values (0-1023)
 * using an int array, and switching to a <code>HashMap</code> for values above
 * 1023. Retrieve all facet counts, in value order, with {@link
 * #getAllChildrenSortByValue}, or get the topN values sorted by count with
 * {@link #getTopChildrenSortByCount}.
 *
 *  @lucene.experimental */
class LongValueFacetCounts : public Facets
{
  GET_CLASS_NAME(LongValueFacetCounts)

  /** Used for all values that are < 1K. */
private:
  std::deque<int> const counts = std::deque<int>(1024);

  /** Used for all values that are >= 1K. */
  const std::shared_ptr<LongIntScatterMap> hashCounts =
      std::make_shared<LongIntScatterMap>();

  const std::wstring field;

  /** Total number of values counted, which is the subset of hits that had a
   * value for this field. */
  int totCount = 0;

  /** Create {@code LongValueFacetCounts}, using either single-valued {@link
   *  NumericDocValues} or multi-valued {@link SortedNumericDocValues} from the
   *  specified field. */
public:
  LongValueFacetCounts(const std::wstring &field,
                       std::shared_ptr<FacetsCollector> hits,
                       bool multiValued) ;

  /** Create {@code LongValueFacetCounts}, using the provided
   *  {@link LongValuesSource}.  If hits is
   *  null then all facets are counted. */
  LongValueFacetCounts(
      const std::wstring &field, std::shared_ptr<LongValuesSource> valueSource,
      std::shared_ptr<FacetsCollector> hits) ;

  /** Create {@code LongValueFacetCounts}, using the provided
   *  {@link LongValuesSource}.
   *  random access (implement {@link org.apache.lucene.search.DocIdSet#bits}).
   */
  LongValueFacetCounts(const std::wstring &field,
                       std::shared_ptr<LongValuesSource> valueSource,
                       std::shared_ptr<FacetsCollector> hits,
                       bool multiValued) ;

  /** Counts all facet values for this reader.  This produces the same result as
   * computing facets on a {@link org.apache.lucene.search.MatchAllDocsQuery},
   * but is more efficient. */
  LongValueFacetCounts(const std::wstring &field,
                       std::shared_ptr<IndexReader> reader,
                       bool multiValued) ;

  /** Counts all facet values for the provided {@link LongValuesSource}.  This
   * produces the same result as computing facets on a {@link
   * org.apache.lucene.search.MatchAllDocsQuery}, but is more efficient. */
  LongValueFacetCounts(const std::wstring &field,
                       std::shared_ptr<LongValuesSource> valueSource,
                       std::shared_ptr<IndexReader> reader) ;

private:
  void count(std::shared_ptr<LongValuesSource> valueSource,
             std::deque<std::shared_ptr<MatchingDocs>>
                 &matchingDocs) ;

  void count(const std::wstring &field,
             std::deque<std::shared_ptr<MatchingDocs>>
                 &matchingDocs) ;

  void countOneSegment(std::shared_ptr<NumericDocValues> values,
                       std::shared_ptr<MatchingDocs> hits) ;

  /** Counts directly from SortedNumericDocValues. */
  void countMultiValued(const std::wstring &field,
                        std::deque<std::shared_ptr<MatchingDocs>>
                            &matchingDocs) ;

  /** Optimized version that directly counts all doc values. */
  void countAll(std::shared_ptr<IndexReader> reader,
                const std::wstring &field) ;

  void countAllOneSegment(std::shared_ptr<NumericDocValues> values) throw(
      IOException);

  void countAll(std::shared_ptr<LongValuesSource> valueSource,
                const std::wstring &field,
                std::shared_ptr<IndexReader> reader) ;

  void countAllMultiValued(std::shared_ptr<IndexReader> reader,
                           const std::wstring &field) ;

  void increment(int64_t value);

public:
  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path) override;

  /** Reusable hash entry to hold long facet value and int count. */
private:
  class Entry : public std::enable_shared_from_this<Entry>
  {
    GET_CLASS_NAME(Entry)
  public:
    int count = 0;
    int64_t value = 0;
  };

  /** Returns the specified top number of facets, sorted by count. */
public:
  virtual std::shared_ptr<FacetResult> getTopChildrenSortByCount(int topN);

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<Entry>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  private:
    std::shared_ptr<LongValueFacetCounts> outerInstance;

  public:
    PriorityQueueAnonymousInnerClass(
        std::shared_ptr<LongValueFacetCounts> outerInstance, int min);

  protected:
    bool lessThan(std::shared_ptr<Entry> a, std::shared_ptr<Entry> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<Entry>::shared_from_this());
    }
  };

  /** Returns all unique values seen, sorted by value.  */
public:
  virtual std::shared_ptr<FacetResult> getAllChildrenSortByValue();

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  private:
    std::shared_ptr<LongValueFacetCounts> outerInstance;

    std::deque<int> hashCounts;
    std::deque<int64_t> hashValues;

  public:
    InPlaceMergeSorterAnonymousInnerClass(
        std::shared_ptr<LongValueFacetCounts> outerInstance,
        std::deque<int> &hashCounts, std::deque<int64_t> &hashValues);

    int compare(int i, int j) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

private:
  void appendCounts(std::deque<std::shared_ptr<LabelAndValue>> &labelValues);

public:
  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::deque<std::shared_ptr<FacetResult>>
  getAllDims(int topN)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<LongValueFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<LongValueFacetCounts>(
        Facets::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet
