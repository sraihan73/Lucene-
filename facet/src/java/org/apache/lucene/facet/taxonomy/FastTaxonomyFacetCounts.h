#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class FacetsCollector;
}

namespace org::apache::lucene::facet
{
class FacetsConfig;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyReader;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::facet::taxonomy
{

using MatchingDocs = org::apache::lucene::facet::FacetsCollector::MatchingDocs;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using IndexReader = org::apache::lucene::index::IndexReader;

/** Computes facets counts, assuming the default encoding
 *  into DocValues was used.
 *
 * @lucene.experimental */
class FastTaxonomyFacetCounts : public IntTaxonomyFacets
{
  GET_CLASS_NAME(FastTaxonomyFacetCounts)

  /** Create {@code FastTaxonomyFacetCounts}, which also
   *  counts all facet labels. */
public:
  FastTaxonomyFacetCounts(
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<FacetsCollector> fc) ;

  /** Create {@code FastTaxonomyFacetCounts}, using the
   *  specified {@code indexFieldName} for ordinals.  Use
   *  this if you had set {@link
   *  FacetsConfig#setIndexFieldName} to change the index
   *  field name for certain dimensions. */
  FastTaxonomyFacetCounts(
      const std::wstring &indexFieldName,
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<FacetsCollector> fc) ;

  /** Create {@code FastTaxonomyFacetCounts}, using the
   *  specified {@code indexFieldName} for ordinals, and
   *  counting all non-deleted documents in the index.  This is
   *  the same result as searching on {@link MatchAllDocsQuery},
   *  but faster */
  FastTaxonomyFacetCounts(
      const std::wstring &indexFieldName, std::shared_ptr<IndexReader> reader,
      std::shared_ptr<TaxonomyReader> taxoReader,
      std::shared_ptr<FacetsConfig> config) ;

private:
  void count(std::deque<std::shared_ptr<MatchingDocs>> &matchingDocs) throw(
      IOException);

  void countAll(std::shared_ptr<IndexReader> reader) ;

protected:
  std::shared_ptr<FastTaxonomyFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<FastTaxonomyFacetCounts>(
        IntTaxonomyFacets::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
