#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
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
namespace org::apache::lucene::facet::taxonomy
{

using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

using com::carrotsearch::hppc::IntIntScatterMap;

/** Base class for all taxonomy-based facets that aggregate
 *  to a per-ords int[]. */

class IntTaxonomyFacets : public TaxonomyFacets
{
  GET_CLASS_NAME(IntTaxonomyFacets)

  /** Per-ordinal value. */
private:
  std::deque<int> const values;
  const std::shared_ptr<IntIntScatterMap> sparseValues;

  /** Sole constructor. */
protected:
  IntTaxonomyFacets(const std::wstring &indexFieldName,
                    std::shared_ptr<TaxonomyReader> taxoReader,
                    std::shared_ptr<FacetsConfig> config,
                    std::shared_ptr<FacetsCollector> fc) ;

  /** Return true if a sparse hash table should be used for counting, instead of
   * a dense int[]. */
  virtual bool useHashTable(std::shared_ptr<FacetsCollector> fc,
                            std::shared_ptr<TaxonomyReader> taxoReader);

  /** Increment the count for this ordinal by 1. */
  virtual void increment(int ordinal);

  /** Increment the count for this ordinal by {@code amount}.. */
  virtual void increment(int ordinal, int amount);

private:
  int getValue(int ordinal);

  /** Rolls up any single-valued hierarchical dimensions. */
protected:
  virtual void rollup() ;

private:
  int rollup(int ord) ;

public:
  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path)  override;

protected:
  std::shared_ptr<IntTaxonomyFacets> shared_from_this()
  {
    return std::static_pointer_cast<IntTaxonomyFacets>(
        TaxonomyFacets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
