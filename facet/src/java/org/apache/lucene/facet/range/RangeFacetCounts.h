#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/range/Range.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"

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
namespace org::apache::lucene::facet::range
{

using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using Query = org::apache::lucene::search::Query;

/** Base class for range faceting.
 *
 *  @lucene.experimental */
class RangeFacetCounts : public Facets
{
  GET_CLASS_NAME(RangeFacetCounts)
  /** Ranges passed to constructor. */
protected:
  std::deque<std::shared_ptr<Range>> const ranges;

  /** Counts, initialized in by subclass. */
  std::deque<int> const counts;

  /** Optional: if specified, we first test this Query to
   *  see whether the document should be checked for
   *  matching ranges.  If this is null, all documents are
   *  checked. */
  const std::shared_ptr<Query> fastMatchQuery;

  /** Our field name. */
  const std::wstring field;

  /** Total number of hits. */
  int totCount = 0;

  /** Create {@code RangeFacetCounts} */
  RangeFacetCounts(const std::wstring &field,
                   std::deque<std::shared_ptr<Range>> &ranges,
                   std::shared_ptr<Query> fastMatchQuery) ;

public:
  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path) override;

  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::deque<std::shared_ptr<FacetResult>>
  getAllDims(int topN)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<RangeFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<RangeFacetCounts>(
        org.apache.lucene.facet.Facets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/range/
