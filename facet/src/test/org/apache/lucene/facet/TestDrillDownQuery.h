#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::facet::taxonomy::directory
{
class DirectoryTaxonomyReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::facet
{
class FacetsConfig;
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
namespace org::apache::lucene::facet
{

using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;

class TestDrillDownQuery : public FacetTestCase
{
  GET_CLASS_NAME(TestDrillDownQuery)

private:
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<DirectoryTaxonomyReader> taxo;
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<Directory> taxoDir;
  static std::shared_ptr<FacetsConfig> config;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // afterClassDrillDownQueryTest() throws Exception
  static void afterClassDrillDownQueryTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // beforeClassDrillDownQueryTest() throws Exception
  static void beforeClassDrillDownQueryTest() ;

  virtual void testAndOrs() ;

  virtual void testQuery() ;

  virtual void testQueryImplicitDefaultParams() ;

  virtual void testZeroLimit() ;

  virtual void testScoring() ;

  virtual void testScoringNoBaseQuery() ;

  virtual void testTermNonDefault();

  virtual void testClone() ;

  virtual void testNoDrillDown() ;

protected:
  std::shared_ptr<TestDrillDownQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestDrillDownQuery>(
        FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet
