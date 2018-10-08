#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

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
namespace org::apache::lucene::analysis::core
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;

class TestKeywordAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestKeywordAnalyzer)

private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Analyzer> analyzer;

public:
  void setUp()  override;

  void tearDown()  override;

  /*
  public void testPerFieldAnalyzer() throws Exception {
    PerFieldAnalyzerWrapper analyzer = new PerFieldAnalyzerWrapper(new
  SimpleAnalyzer()); analyzer.addAnalyzer("partnum", new KeywordAnalyzer());

    QueryParser queryParser = new QueryParser("description", analyzer);
    Query query = queryParser.parse("partnum:Q36 AND SPACE");

    ScoreDoc[] hits = searcher.search(query, null, 1000).scoreDocs;
    assertEquals("Q36 kept as-is",
              "+partnum:Q36 +space", query.toString("description"));
    assertEquals("doc found!", 1, hits.length);
  }
  */

  virtual void testMutipleDocument() ;

  // LUCENE-1441
  virtual void testOffsets() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestKeywordAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestKeywordAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
