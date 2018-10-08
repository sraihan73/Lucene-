#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/** Aggregates sum of int values previously indexed with
 *  {@link IntAssociationFacetField}, assuming the default
 *  encoding.
 *
 *  @lucene.experimental */
class TaxonomyFacetSumIntAssociations : public IntTaxonomyFacets
{
  GET_CLASS_NAME(TaxonomyFacetSumIntAssociations)

  /** Create {@code TaxonomyFacetSumIntAssociations} against
   *  the default index field. */
public:
  TaxonomyFacetSumIntAssociations(
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<FacetsCollector> fc) ;

  /** Create {@code TaxonomyFacetSumIntAssociations} against
   *  the specified index field. */
  TaxonomyFacetSumIntAssociations(
      const std::wstring &indexFieldName,
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<FacetsCollector> fc) ;

private:
  void sumValues(std::deque<std::shared_ptr<MatchingDocs>>
                     &matchingDocs) ;

protected:
  std::shared_ptr<TaxonomyFacetSumIntAssociations> shared_from_this()
  {
    return std::static_pointer_cast<TaxonomyFacetSumIntAssociations>(
        IntTaxonomyFacets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
