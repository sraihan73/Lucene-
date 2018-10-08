#pragma once
#include "stringhelper.h"
#include <any>
#include <deque>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/search/grouping/SegmentResult.h"
#include  "core/src/java/org/apache/lucene/search/grouping/GroupedFacetResult.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/grouping/FacetEntry.h"

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
namespace org::apache::lucene::search::grouping
{

using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Base class for computing grouped facets.
 *
 * @lucene.experimental
 */
class GroupFacetCollector : public SimpleCollector
{
  GET_CLASS_NAME(GroupFacetCollector)

protected:
  const std::wstring groupField;
  const std::wstring facetField;
  const std::shared_ptr<BytesRef> facetPrefix;
  const std::deque<std::shared_ptr<SegmentResult>> segmentResults;

  std::deque<int> segmentFacetCounts;
  int segmentTotalCount = 0;
  int startFacetOrd = 0;
  int endFacetOrd = 0;

  GroupFacetCollector(const std::wstring &groupField,
                      const std::wstring &facetField,
                      std::shared_ptr<BytesRef> facetPrefix);

  /**
   * Returns grouped facet results that were computed over zero or more
   * segments. Grouped facet counts are merged from zero or more segment
   * results.
   *
   * @param size The total number of facets to include. This is typically offset
   * + limit
   * @param minCount The minimum count a facet entry should have to be included
   * in the grouped facet result
   * @param orderByCount Whether to sort the facet entries by facet entry count.
   * If <code>false</code> then the facets are sorted lexicographically in
   * ascending order.
   * @return grouped facet results
   * @throws IOException If I/O related errors occur during merging segment
   * grouped facet counts.
   */
public:
  virtual std::shared_ptr<GroupedFacetResult>
  mergeSegmentResults(int size, int minCount,
                      bool orderByCount) ;

protected:
  virtual std::shared_ptr<SegmentResult> createSegmentResult() = 0;

public:
  void setScorer(std::shared_ptr<Scorer> scorer)  override;

  bool needsScores() override;

  /**
   * The grouped facet result. Containing grouped facet entries, total count and
   * total missing count.
   */
public:
  class GroupedFacetResult
      : public std::enable_shared_from_this<GroupedFacetResult>
  {
    GET_CLASS_NAME(GroupedFacetResult)

  private:
    static const std::shared_ptr<Comparator<std::shared_ptr<FacetEntry>>>
        orderByCountAndValue;

  private:
    class ComparatorAnonymousInnerClass
        : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
          public Comparator<std::shared_ptr<FacetEntry>>
    {
      GET_CLASS_NAME(ComparatorAnonymousInnerClass)
    public:
      ComparatorAnonymousInnerClass();

      int compare(std::shared_ptr<FacetEntry> a, std::shared_ptr<FacetEntry> b);
    };

  private:
    static const std::shared_ptr<Comparator<std::shared_ptr<FacetEntry>>>
        orderByValue;

  private:
    class ComparatorAnonymousInnerClass2
        : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
          public Comparator<std::shared_ptr<FacetEntry>>
    {
      GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
    public:
      ComparatorAnonymousInnerClass2();

      int compare(std::shared_ptr<FacetEntry> a, std::shared_ptr<FacetEntry> b);
    };

  private:
    const int maxSize;
    const std::shared_ptr<NavigableSet<std::shared_ptr<FacetEntry>>>
        facetEntries;
    const int totalMissingCount;
    const int totalCount;

    int currentMin = 0;

  public:
    GroupedFacetResult(int size, int minCount, bool orderByCount,
                       int totalCount, int totalMissingCount);

    virtual void addFacetCount(std::shared_ptr<BytesRef> facetValue, int count);

    /**
     * Returns a deque of facet entries to be rendered based on the specified
     * offset and limit. The facet entries are retrieved from the facet entries
     * collected during merging.
     *
     * @param offset The offset in the collected facet entries during merging
     * @param limit The number of facets to return starting from the offset.
     * @return a deque of facet entries to be rendered based on the specified
     * offset and limit
     */
    virtual std::deque<std::shared_ptr<FacetEntry>> getFacetEntries(int offset,
                                                                     int limit);

    /**
     * Returns the sum of all facet entries counts.
     *
     * @return the sum of all facet entries counts
     */
    virtual int getTotalCount();

    /**
     * Returns the number of groups that didn't have a facet value.
     *
     * @return the number of groups that didn't have a facet value
     */
    virtual int getTotalMissingCount();
  };

  /**
   * Represents a facet entry with a value and a count.
   */
public:
  class FacetEntry : public std::enable_shared_from_this<FacetEntry>
  {
    GET_CLASS_NAME(FacetEntry)

  private:
    const std::shared_ptr<BytesRef> value;
    const int count;

  public:
    FacetEntry(std::shared_ptr<BytesRef> value, int count);

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

    /**
     * @return The value of this facet entry
     */
    virtual std::shared_ptr<BytesRef> getValue();

    /**
     * @return The count (number of groups) of this facet entry.
     */
    virtual int getCount();
  };

  /**
   * Contains the local grouped segment counts for a particular segment.
   * Each <code>SegmentResult</code> must be added together.
   */
protected:
  class SegmentResult : public std::enable_shared_from_this<SegmentResult>
  {
    GET_CLASS_NAME(SegmentResult)

  protected:
    std::deque<int> const counts;
    const int total;
    const int missing;
    const int maxTermPos;

    std::shared_ptr<BytesRef> mergeTerm;
    int mergePos = 0;

    SegmentResult(std::deque<int> &counts, int total, int missing,
                  int maxTermPos);

    /**
     * Go to next term in this <code>SegmentResult</code> in order to retrieve
     * the grouped facet counts.
     *
     * @throws IOException If I/O related errors occur
     */
    virtual void nextTerm() = 0;
  };

private:
  class SegmentResultPriorityQueue
      : public PriorityQueue<std::shared_ptr<SegmentResult>>
  {
    GET_CLASS_NAME(SegmentResultPriorityQueue)

  public:
    SegmentResultPriorityQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<SegmentResult> a,
                  std::shared_ptr<SegmentResult> b) override;

  protected:
    std::shared_ptr<SegmentResultPriorityQueue> shared_from_this()
    {
      return std::static_pointer_cast<SegmentResultPriorityQueue>(
          org.apache.lucene.util
              .PriorityQueue<SegmentResult>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<GroupFacetCollector> shared_from_this()
  {
    return std::static_pointer_cast<GroupFacetCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
