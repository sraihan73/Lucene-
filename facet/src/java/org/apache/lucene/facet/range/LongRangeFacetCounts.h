#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"

#include  "core/src/java/org/apache/lucene/facet/range/LongRange.h"
#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/facet/MatchingDocs.h"

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

using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Query = org::apache::lucene::search::Query;

/** {@link Facets} implementation that computes counts for
 *  dynamic long ranges from a provided {@link LongValuesSource}.  Use
 *  this for dimensions that change in real-time (e.g. a
 *  relative time based dimension like "Past day", "Past 2
 *  days", etc.) or that change for each request (e.g.
 *  distance from the user's location, "&lt; 1 km", "&lt; 2 km",
 *  etc.).
 *
 *  @lucene.experimental */
class LongRangeFacetCounts : public RangeFacetCounts
{
  GET_CLASS_NAME(LongRangeFacetCounts)

  /** Create {@code LongRangeFacetCounts}, using {@link
   *  LongValuesSource} from the specified field. */
public:
  LongRangeFacetCounts(const std::wstring &field,
                       std::shared_ptr<FacetsCollector> hits,
                       std::deque<LongRange> &ranges) ;

  /** Create {@code LongRangeFacetCounts}, using the provided
   *  {@link LongValuesSource}. */
  LongRangeFacetCounts(const std::wstring &field,
                       std::shared_ptr<LongValuesSource> valueSource,
                       std::shared_ptr<FacetsCollector> hits,
                       std::deque<LongRange> &ranges) ;

  /** Create {@code LongRangeFacetCounts}, using the provided
   *  {@link LongValuesSource}, and using the provided Filter as
   *  a fastmatch: only documents passing the filter are
   *  checked for the matching ranges, which is helpful when
   *  the provided {@link LongValuesSource} is costly per-document,
   *  such as a geo distance.  The filter must be
   *  random access (implement {@link DocIdSet#bits}). */
  LongRangeFacetCounts(const std::wstring &field,
                       std::shared_ptr<LongValuesSource> valueSource,
                       std::shared_ptr<FacetsCollector> hits,
                       std::shared_ptr<Query> fastMatchQuery,
                       std::deque<LongRange> &ranges) ;

private:
  void count(std::shared_ptr<LongValuesSource> valueSource,
             std::deque<std::shared_ptr<FacetsCollector::MatchingDocs>>
                 &matchingDocs) ;

protected:
  std::shared_ptr<LongRangeFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<LongRangeFacetCounts>(
        RangeFacetCounts::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/range/
