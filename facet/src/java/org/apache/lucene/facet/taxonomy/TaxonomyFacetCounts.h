#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/OrdinalsReader.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
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
namespace org::apache::lucene::facet::taxonomy
{

using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/** Reads from any {@link OrdinalsReader}; use {@link
 *  FastTaxonomyFacetCounts} if you are using the
 *  default encoding from {@link BinaryDocValues}.
 *
 * @lucene.experimental */
class TaxonomyFacetCounts : public IntTaxonomyFacets
{
  GET_CLASS_NAME(TaxonomyFacetCounts)
private:
  const std::shared_ptr<OrdinalsReader> ordinalsReader;

  /** Create {@code TaxonomyFacetCounts}, which also
   *  counts all facet labels.  Use this for a non-default
   *  {@link OrdinalsReader}; otherwise use {@link
   *  FastTaxonomyFacetCounts}. */
public:
  TaxonomyFacetCounts(std::shared_ptr<OrdinalsReader> ordinalsReader,
                      std::shared_ptr<TaxonomyReader> taxoReader,
                      std::shared_ptr<FacetsConfig> config,
                      std::shared_ptr<FacetsCollector> fc) ;

private:
  void count(std::deque<std::shared_ptr<MatchingDocs>> &matchingDocs) throw(
      IOException);

protected:
  std::shared_ptr<TaxonomyFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<TaxonomyFacetCounts>(
        IntTaxonomyFacets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
