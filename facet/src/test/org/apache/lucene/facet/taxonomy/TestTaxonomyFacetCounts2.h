#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::facet
{
class FacetField;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::facet
{
class FacetsConfig;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyWriter;
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

using Document = org::apache::lucene::document::Document;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;

class TestTaxonomyFacetCounts2 : public FacetTestCase
{
  GET_CLASS_NAME(TestTaxonomyFacetCounts2)

private:
  static const std::shared_ptr<Term> A;
  static const std::wstring CP_A, CP_B;
  static const std::wstring CP_C, CP_D; // indexed w/ NO_PARENTS
  static constexpr int NUM_CHILDREN_CP_A = 5, NUM_CHILDREN_CP_B = 3;
  static constexpr int NUM_CHILDREN_CP_C = 5, NUM_CHILDREN_CP_D = 5;
  static std::deque<std::shared_ptr<FacetField>> const CATEGORIES_A,
      CATEGORIES_B;
  static std::deque<std::shared_ptr<FacetField>> const CATEGORIES_C,
      CATEGORIES_D;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestTaxonomyFacetCounts2::StaticConstructor staticConstructor;

  static std::shared_ptr<Directory> indexDir, taxoDir;
  static std::unordered_map<std::wstring, int> allExpectedCounts,
      termExpectedCounts;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // afterClassCountingFacetsAggregatorTest() throws Exception
  static void
  afterClassCountingFacetsAggregatorTest() ;

private:
  static std::deque<std::shared_ptr<FacetField>>
  randomCategories(std::shared_ptr<Random> random);

  static void addField(std::shared_ptr<Document> doc);

  static void addFacets(std::shared_ptr<Document> doc,
                        std::shared_ptr<FacetsConfig> config,
                        bool updateTermExpectedCounts) ;

  static std::shared_ptr<FacetsConfig> getConfig();

  static void indexDocsNoFacets(std::shared_ptr<IndexWriter> indexWriter) throw(
      IOException);

  static void indexDocsWithFacetsNoTerms(
      std::shared_ptr<IndexWriter> indexWriter,
      std::shared_ptr<TaxonomyWriter> taxoWriter,
      std::unordered_map<std::wstring, int> &expectedCounts) ;

  static void indexDocsWithFacetsAndTerms(
      std::shared_ptr<IndexWriter> indexWriter,
      std::shared_ptr<TaxonomyWriter> taxoWriter,
      std::unordered_map<std::wstring, int> &expectedCounts) ;

  static void indexDocsWithFacetsAndSomeTerms(
      std::shared_ptr<IndexWriter> indexWriter,
      std::shared_ptr<TaxonomyWriter> taxoWriter,
      std::unordered_map<std::wstring, int> &expectedCounts) ;

  // initialize expectedCounts w/ 0 for all categories
  static std::unordered_map<std::wstring, int> newCounts();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassCountingFacetsAggregatorTest() throws Exception
  static void
  beforeClassCountingFacetsAggregatorTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDifferentNumResults() throws Exception
  virtual void testDifferentNumResults() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAllCounts() throws Exception
  virtual void testAllCounts() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBigNumResults() throws Exception
  virtual void testBigNumResults() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNoParents() throws Exception
  virtual void testNoParents() ;

protected:
  std::shared_ptr<TestTaxonomyFacetCounts2> shared_from_this()
  {
    return std::static_pointer_cast<TestTaxonomyFacetCounts2>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
