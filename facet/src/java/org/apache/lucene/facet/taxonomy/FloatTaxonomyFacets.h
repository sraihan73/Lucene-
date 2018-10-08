#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/** Base class for all taxonomy-based facets that aggregate
 *  to a per-ords float[]. */
class FloatTaxonomyFacets : public TaxonomyFacets
{
  GET_CLASS_NAME(FloatTaxonomyFacets)

  // TODO: also use native hash map_obj for sparse collection, like
  // IntTaxonomyFacets

  /** Per-ordinal value. */
protected:
  std::deque<float> const values;

  /** Sole constructor. */
  FloatTaxonomyFacets(const std::wstring &indexFieldName,
                      std::shared_ptr<TaxonomyReader> taxoReader,
                      std::shared_ptr<FacetsConfig> config) ;

  /** Rolls up any single-valued hierarchical dimensions. */
  virtual void rollup() ;

private:
  float rollup(int ord) ;

public:
  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path)  override;

protected:
  std::shared_ptr<FloatTaxonomyFacets> shared_from_this()
  {
    return std::static_pointer_cast<FloatTaxonomyFacets>(
        TaxonomyFacets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
