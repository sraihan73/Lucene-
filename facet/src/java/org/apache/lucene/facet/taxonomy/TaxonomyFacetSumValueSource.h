#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/OrdinalsReader.h"

#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
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

using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;

/** Aggregates sum of values from {@link
 *  DoubleValues#doubleValue()}, for each facet label.
 *
 *  @lucene.experimental */
class TaxonomyFacetSumValueSource : public FloatTaxonomyFacets
{
  GET_CLASS_NAME(TaxonomyFacetSumValueSource)
private:
  const std::shared_ptr<OrdinalsReader> ordinalsReader;

  /**
   * Aggreggates double facet values from the provided
   * {@link DoubleValuesSource}, pulling ordinals using {@link
   * DocValuesOrdinalsReader} against the default indexed
   * facet field {@link FacetsConfig#DEFAULT_INDEX_FIELD_NAME}.
   */
public:
  TaxonomyFacetSumValueSource(
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config, std::shared_ptr<FacetsCollector> fc,
      std::shared_ptr<DoubleValuesSource> valueSource) ;

  /**
   * Aggreggates float facet values from the provided
   *  {@link DoubleValuesSource}, and pulls ordinals from the
   *  provided {@link OrdinalsReader}.
   */
  TaxonomyFacetSumValueSource(
      std::shared_ptr<OrdinalsReader> ordinalsReader,
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config, std::shared_ptr<FacetsCollector> fc,
      std::shared_ptr<DoubleValuesSource> vs) ;

private:
  static std::shared_ptr<DoubleValues>
  scores(std::shared_ptr<FacetsCollector::MatchingDocs> hits);

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<FacetsCollector::MatchingDocs> hits;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<FacetsCollector::MatchingDocs> hits);

    int index = -1;

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

private:
  void sumValues(
      std::deque<std::shared_ptr<FacetsCollector::MatchingDocs>> &matchingDocs,
      bool keepScores,
      std::shared_ptr<DoubleValuesSource> valueSource) ;

protected:
  std::shared_ptr<TaxonomyFacetSumValueSource> shared_from_this()
  {
    return std::static_pointer_cast<TaxonomyFacetSumValueSource>(
        FloatTaxonomyFacets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
