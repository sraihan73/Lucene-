#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"

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

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

class TestTaxonomyFacetCounts : public FacetTestCase
{
  GET_CLASS_NAME(TestTaxonomyFacetCounts)

public:
  virtual void testBasic() ;

  // LUCENE-5333
  virtual void testSparseFacets() ;

  virtual void testWrongIndexFieldName() ;

  virtual void testReallyNoNormsForDrillDown() ;

private:
  class PerFieldSimilarityWrapperAnonymousInnerClass
      : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(PerFieldSimilarityWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestTaxonomyFacetCounts> outerInstance;

  public:
    PerFieldSimilarityWrapperAnonymousInnerClass(
        std::shared_ptr<TestTaxonomyFacetCounts> outerInstance);

    const std::shared_ptr<Similarity> sim;

    std::shared_ptr<Similarity> get(const std::wstring &name) override;

  protected:
    std::shared_ptr<PerFieldSimilarityWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PerFieldSimilarityWrapperAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .PerFieldSimilarityWrapper::shared_from_this());
    }
  };

public:
  virtual void testMultiValuedHierarchy() ;

  virtual void testLabelWithDelimiter() ;

  virtual void testRequireDimCount() ;

  // LUCENE-4583: make sure if we require > 32 KB for one
  // document, we don't hit exc when using Facet42DocValuesFormat
  virtual void testManyFacetsInOneDocument() ;

  // Make sure we catch when app didn't declare field as
  // hierarchical but it was:
  virtual void testDetectHierarchicalField() ;

  // Make sure we catch when app didn't declare field as
  // multi-valued but it was:
  virtual void testDetectMultiValuedField() ;

  virtual void testSeparateIndexedFields() ;

  virtual void testCountRoot() ;

  virtual void testGetFacetResultsTwice() ;

  virtual void testChildCount() ;

private:
  void indexTwoDocs(std::shared_ptr<TaxonomyWriter> taxoWriter,
                    std::shared_ptr<IndexWriter> indexWriter,
                    std::shared_ptr<FacetsConfig> config,
                    bool withContent) ;

public:
  virtual void
  testSegmentsWithoutCategoriesOrResults() ;

  virtual void testRandom() ;

private:
  static std::shared_ptr<Facets>
  getAllFacets(const std::wstring &indexFieldName,
               std::shared_ptr<IndexSearcher> searcher,
               std::shared_ptr<TaxonomyReader> taxoReader,
               std::shared_ptr<FacetsConfig> config) ;

protected:
  std::shared_ptr<TestTaxonomyFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<TestTaxonomyFacetCounts>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
