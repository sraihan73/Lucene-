#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using Directory = org::apache::lucene::store::Directory;

class TestOrdinalMappingLeafReader : public FacetTestCase
{
  GET_CLASS_NAME(TestOrdinalMappingLeafReader)

private:
  static constexpr int NUM_DOCS = 100;
  const std::shared_ptr<FacetsConfig> facetConfig =
      std::make_shared<FacetsConfig>();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before @Override public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTaxonomyMergeUtils() throws Exception
  virtual void testTaxonomyMergeUtils() ;

private:
  void verifyResults(std::shared_ptr<Directory> indexDir,
                     std::shared_ptr<Directory> taxoDir) ;

  void buildIndexWithFacets(std::shared_ptr<Directory> indexDir,
                            std::shared_ptr<Directory> taxoDir,
                            bool asc) ;

protected:
  std::shared_ptr<TestOrdinalMappingLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TestOrdinalMappingLeafReader>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
