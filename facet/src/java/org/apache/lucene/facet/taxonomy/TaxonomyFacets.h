#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/DimConfig.h"

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
using Facets = org::apache::lucene::facet::Facets;
using DimConfig =
    org::apache::lucene::facet::FacetsConfig::DimConfig; // javadocs
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/** Base class for all taxonomy-based facets impls. */
class TaxonomyFacets : public Facets
{

private:
  static const std::shared_ptr<Comparator<std::shared_ptr<FacetResult>>>
      BY_VALUE_THEN_DIM;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<FacetResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<FacetResult> a,
                std::shared_ptr<FacetResult> b) override;
  };

  /** Index field name provided to the constructor. */
protected:
  const std::wstring indexFieldName;

  /** {@code TaxonomyReader} provided to the constructor. */
  const std::shared_ptr<TaxonomyReader> taxoReader;

  /** {@code FacetsConfig} provided to the constructor. */
  const std::shared_ptr<FacetsConfig> config;

  /** Maps parent ordinal to its child, or -1 if the parent
   *  is childless. */
private:
  std::deque<int> children;

  /** Maps an ordinal to its sibling, or -1 if there is no
   *  sibling. */
  std::deque<int> siblings;

  /** Maps an ordinal to its parent, or -1 if there is no
   *  parent (root node). */
protected:
  std::deque<int> const parents;

  /** Sole constructor. */
  TaxonomyFacets(const std::wstring &indexFieldName,
                 std::shared_ptr<TaxonomyReader> taxoReader,
                 std::shared_ptr<FacetsConfig> config) ;

  /** Returns int[] mapping each ordinal to its first child; this is a large
   * array and is computed (and then saved) the first time this method is
   * invoked. */
  virtual std::deque<int> getChildren() ;

  /** Returns int[] mapping each ordinal to its next sibling; this is a large
   * array and is computed (and then saved) the first time this method is
   * invoked. */
  virtual std::deque<int> getSiblings() ;

  /** Returns true if the (costly, and lazily initialized) children int[] was
   * initialized.
   *
   * @lucene.experimental */
public:
  virtual bool childrenLoaded();

  /** Returns true if the (costly, and lazily initialized) sibling int[] was
   * initialized.
   *
   * @lucene.experimental */
  virtual bool siblingsLoaded();

  /** Throws {@code IllegalArgumentException} if the
   *  dimension is not recognized.  Otherwise, returns the
   *  {@link DimConfig} for this dimension. */
protected:
  virtual std::shared_ptr<DimConfig> verifyDim(const std::wstring &dim);

public:
  std::deque<std::shared_ptr<FacetResult>>
  getAllDims(int topN)  override;

protected:
  std::shared_ptr<TaxonomyFacets> shared_from_this()
  {
    return std::static_pointer_cast<TaxonomyFacets>(
        org.apache.lucene.facet.Facets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
