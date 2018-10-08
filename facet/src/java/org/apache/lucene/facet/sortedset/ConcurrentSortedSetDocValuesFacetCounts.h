#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/sortedset/SortedSetDocValuesReaderState.h"

#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"
#include  "core/src/java/org/apache/lucene/facet/sortedset/OrdRange.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/facet/MatchingDocs.h"
#include  "core/src/java/org/apache/lucene/index/OrdinalMap.h"

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
namespace org::apache::lucene::facet::sortedset
{

using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using OrdRange = org::apache::lucene::facet::sortedset::
    SortedSetDocValuesReaderState::OrdRange;
using LeafReader = org::apache::lucene::index::LeafReader;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;

/** Like {@link SortedSetDocValuesFacetCounts}, but aggregates counts
 * concurrently across segments.
 *
 * @lucene.experimental */
class ConcurrentSortedSetDocValuesFacetCounts : public Facets
{
  GET_CLASS_NAME(ConcurrentSortedSetDocValuesFacetCounts)

public:
  const std::shared_ptr<ExecutorService> exec;
  const std::shared_ptr<SortedSetDocValuesReaderState> state;
  const std::shared_ptr<SortedSetDocValues> dv;
  const std::wstring field;
  const std::shared_ptr<AtomicIntegerArray> counts;

  /** Returns all facet counts, same result as searching on {@link
   * MatchAllDocsQuery} but faster. */
  ConcurrentSortedSetDocValuesFacetCounts(
      std::shared_ptr<SortedSetDocValuesReaderState> state,
      std::shared_ptr<ExecutorService> exec) throw(IOException,
                                                   InterruptedException);

  /** Counts all facet dimensions across the provided hits. */
  ConcurrentSortedSetDocValuesFacetCounts(
      std::shared_ptr<SortedSetDocValuesReaderState> state,
      std::shared_ptr<FacetsCollector> hits,
      std::shared_ptr<ExecutorService> exec) throw(IOException,
                                                   InterruptedException);

  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path)  override;

private:
  std::shared_ptr<FacetResult> getDim(const std::wstring &dim,
                                      std::shared_ptr<OrdRange> ordRange,
                                      int topN) ;

private:
  class CountOneSegment : public std::enable_shared_from_this<CountOneSegment>,
                          public Callable<std::shared_ptr<Void>>
  {
    GET_CLASS_NAME(CountOneSegment)
  private:
    std::shared_ptr<ConcurrentSortedSetDocValuesFacetCounts> outerInstance;

  public:
    const std::shared_ptr<LeafReader> leafReader;
    const std::shared_ptr<MatchingDocs> hits;
    const std::shared_ptr<OrdinalMap> ordinalMap;
    const int segOrd;

    CountOneSegment(
        std::shared_ptr<ConcurrentSortedSetDocValuesFacetCounts> outerInstance,
        std::shared_ptr<LeafReader> leafReader,
        std::shared_ptr<MatchingDocs> hits,
        std::shared_ptr<OrdinalMap> ordinalMap, int segOrd);

    std::shared_ptr<Void> call()  override;
  };

  /** Does all the "real work" of tallying up the counts. */
private:
  void count(std::deque<std::shared_ptr<MatchingDocs>> &matchingDocs) throw(
      IOException, InterruptedException);

  /** Does all the "real work" of tallying up the counts. */
  void countAll() ;

public:
  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::deque<std::shared_ptr<FacetResult>>
  getAllDims(int topN)  override;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<FacetResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ConcurrentSortedSetDocValuesFacetCounts> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<ConcurrentSortedSetDocValuesFacetCounts> outerInstance);

    int compare(std::shared_ptr<FacetResult> a, std::shared_ptr<FacetResult> b);
  };

protected:
  std::shared_ptr<ConcurrentSortedSetDocValuesFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<ConcurrentSortedSetDocValuesFacetCounts>(
        org.apache.lucene.facet.Facets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/sortedset/
