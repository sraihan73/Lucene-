#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"

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

using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;

class TestFacetQuery : public FacetTestCase
{
  GET_CLASS_NAME(TestFacetQuery)

private:
  static std::shared_ptr<Directory> indexDirectory;
  static std::shared_ptr<RandomIndexWriter> indexWriter;
  static std::shared_ptr<IndexReader> indexReader;
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<FacetsConfig> config;

  static std::deque<std::shared_ptr<IndexableField>> const DOC_SINGLEVALUED;

  static std::deque<std::shared_ptr<IndexableField>> const DOC_MULTIVALUED;

  static std::deque<std::shared_ptr<IndexableField>> const DOC_NOFACET;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void createTestIndex() throws
  // java.io.IOException
  static void createTestIndex() ;

private:
  static void
  indexDocuments(std::deque<IndexableField> &docs) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void closeTestIndex() throws
  // java.io.IOException
  static void closeTestIndex() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSingleValued() throws Exception
  virtual void testSingleValued() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiValued() throws Exception
  virtual void testMultiValued() ;

protected:
  std::shared_ptr<TestFacetQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestFacetQuery>(
        FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
