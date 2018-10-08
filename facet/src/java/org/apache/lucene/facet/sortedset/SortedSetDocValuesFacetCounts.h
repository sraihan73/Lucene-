#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::sortedset
{
class SortedSetDocValuesReaderState;
}

namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::facet
{
class FacetsCollector;
}
namespace org::apache::lucene::facet
{
class FacetResult;
}
namespace org::apache::lucene::facet::sortedset
{
class OrdRange;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class OrdinalMap;
}
namespace org::apache::lucene::facet
{
class MatchingDocs;
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

/** Compute facets counts from previously
 *  indexed {@link SortedSetDocValuesFacetField},
 *  without require a separate taxonomy index.  Faceting is
 *  a bit slower (~25%), and there is added cost on every
 *  {@link IndexReader} open to create a new {@link
 *  SortedSetDocValuesReaderState}.  Furthermore, this does
 *  not support hierarchical facets; only flat (dimension +
 *  label) facets, but it uses quite a bit less RAM to do
 *  so.
 *
 *  <p><b>NOTE</b>: this class should be instantiated and
 *  then used from a single thread, because it holds a
 *  thread-private instance of {@link SortedSetDocValues}.
 *
 * <p><b>NOTE:</b>: tie-break is by unicode sort order
 *
 * @lucene.experimental */
class SortedSetDocValuesFacetCounts : public Facets
{
  GET_CLASS_NAME(SortedSetDocValuesFacetCounts)

public:
  const std::shared_ptr<SortedSetDocValuesReaderState> state;
  const std::shared_ptr<SortedSetDocValues> dv;
  const std::wstring field;
  std::deque<int> const counts;

  /** Returns all facet counts, same result as searching on {@link
   * MatchAllDocsQuery} but faster. */
  SortedSetDocValuesFacetCounts(
      std::shared_ptr<SortedSetDocValuesReaderState> state) ;

  /** Counts all facet dimensions across the provided hits. */
  SortedSetDocValuesFacetCounts(
      std::shared_ptr<SortedSetDocValuesReaderState> state,
      std::shared_ptr<FacetsCollector> hits) ;

  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path)  override;

private:
  std::shared_ptr<FacetResult> getDim(const std::wstring &dim,
                                      std::shared_ptr<OrdRange> ordRange,
                                      int topN) ;

  void countOneSegment(std::shared_ptr<OrdinalMap> ordinalMap,
                       std::shared_ptr<LeafReader> reader, int segOrd,
                       std::shared_ptr<MatchingDocs> hits) ;

  /** Does all the "real work" of tallying up the counts. */
  void count(std::deque<std::shared_ptr<MatchingDocs>> &matchingDocs) throw(
      IOException);

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
    std::shared_ptr<SortedSetDocValuesFacetCounts> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<SortedSetDocValuesFacetCounts> outerInstance);

    int compare(std::shared_ptr<FacetResult> a, std::shared_ptr<FacetResult> b);
  };

protected:
  std::shared_ptr<SortedSetDocValuesFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValuesFacetCounts>(
        org.apache.lucene.facet.Facets::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::sortedset
