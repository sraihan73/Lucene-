#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"

#include  "core/src/java/org/apache/lucene/facet/range/DoubleRange.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
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
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;

/** {@link Facets} implementation that computes counts for
 *  dynamic double ranges from a provided {@link
 *  DoubleValuesSource}.  Use this for dimensions that change in real-time (e.g.
 * a relative time based dimension like "Past day", "Past 2 days", etc.) or that
 * change for each request (e.g. distance from the user's location, "&lt; 1 km",
 * "&lt; 2 km", etc.).
 *
 *  If you have indexed your field using {@link
 *  FloatDocValuesField}, then you should use a DoubleValuesSource
 *  generated from {@link DoubleValuesSource#fromFloatField(std::wstring)}.
 *
 *  @lucene.experimental */
class DoubleRangeFacetCounts : public RangeFacetCounts
{
  GET_CLASS_NAME(DoubleRangeFacetCounts)

  /**
   * Create {@code RangeFacetCounts}, using {@link DoubleValues} from the
   * specified field.
   *
   * N.B This assumes that the field was indexed with {@link
   * org.apache.lucene.document.DoubleDocValuesField}. For float-valued fields,
   * use {@link #DoubleRangeFacetCounts(std::wstring, DoubleValuesSource,
   * FacetsCollector, DoubleRange...)}
   */
public:
  DoubleRangeFacetCounts(const std::wstring &field,
                         std::shared_ptr<FacetsCollector> hits,
                         std::deque<DoubleRange> &ranges) ;

  /**
   * Create {@code RangeFacetCounts} using the provided {@link
   * DoubleValuesSource}
   */
  DoubleRangeFacetCounts(const std::wstring &field,
                         std::shared_ptr<DoubleValuesSource> valueSource,
                         std::shared_ptr<FacetsCollector> hits,
                         std::deque<DoubleRange> &ranges) ;

  /**
   * Create {@code RangeFacetCounts}, using the provided
   * {@link DoubleValuesSource}, and using the provided Query as
   * a fastmatch: only documents matching the query are
   * checked for the matching ranges.
   */
  DoubleRangeFacetCounts(const std::wstring &field,
                         std::shared_ptr<DoubleValuesSource> valueSource,
                         std::shared_ptr<FacetsCollector> hits,
                         std::shared_ptr<Query> fastMatchQuery,
                         std::deque<DoubleRange> &ranges) ;

private:
  void count(std::shared_ptr<DoubleValuesSource> valueSource,
             std::deque<std::shared_ptr<FacetsCollector::MatchingDocs>>
                 &matchingDocs) ;

protected:
  std::shared_ptr<DoubleRangeFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<DoubleRangeFacetCounts>(
        RangeFacetCounts::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/range/
