#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetField.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"

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
namespace org::apache::lucene::facet
{

using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

class TestMultipleIndexFields : public FacetTestCase
{
  GET_CLASS_NAME(TestMultipleIndexFields)

private:
  static std::deque<std::shared_ptr<FacetField>> const CATEGORIES;

  std::shared_ptr<FacetsConfig> getConfig();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDefault() throws Exception
  virtual void testDefault() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCustom() throws Exception
  virtual void testCustom() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTwoCustomsSameField() throws Exception
  virtual void testTwoCustomsSameField() ;

private:
  void assertOrdinalsExist(const std::wstring &field,
                           std::shared_ptr<IndexReader> ir) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDifferentFieldsAndText() throws
  // Exception
  virtual void testDifferentFieldsAndText() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSomeSameSomeDifferent() throws
  // Exception
  virtual void testSomeSameSomeDifferent() ;

private:
  void assertCorrectResults(std::shared_ptr<Facets> facets) ;

  std::shared_ptr<FacetsCollector>
  performSearch(std::shared_ptr<TaxonomyReader> tr,
                std::shared_ptr<IndexReader> ir,
                std::shared_ptr<IndexSearcher> searcher) ;

  void seedIndex(std::shared_ptr<TaxonomyWriter> tw,
                 std::shared_ptr<RandomIndexWriter> iw,
                 std::shared_ptr<FacetsConfig> config) ;

protected:
  std::shared_ptr<TestMultipleIndexFields> shared_from_this()
  {
    return std::static_pointer_cast<TestMultipleIndexFields>(
        FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
