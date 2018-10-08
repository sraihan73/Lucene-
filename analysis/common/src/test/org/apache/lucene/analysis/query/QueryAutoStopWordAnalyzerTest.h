#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/RAMDirectory.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/query/QueryAutoStopWordAnalyzer.h"

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
namespace org::apache::lucene::analysis::query
{

using namespace org::apache::lucene::analysis;
using IndexReader = org::apache::lucene::index::IndexReader;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

class QueryAutoStopWordAnalyzerTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(QueryAutoStopWordAnalyzerTest)
public:
  std::deque<std::wstring> variedFieldValues = {
      L"the",  L"quick", L"brown", L"fox",    L"jumped",
      L"over", L"the",   L"lazy",  L"boring", L"dog"};
  std::deque<std::wstring> repetitiveFieldValues = {L"boring", L"boring",
                                                     L"vaguelyboring"};
  std::shared_ptr<RAMDirectory> dir;
  std::shared_ptr<Analyzer> appAnalyzer;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<QueryAutoStopWordAnalyzer> protectedAnalyzer;

  void setUp()  override;

  void tearDown()  override;

  virtual void testNoStopwords() ;

  virtual void testDefaultStopwordsAllFields() ;

  virtual void testStopwordsAllFieldsMaxPercentDocs() ;

  virtual void testStopwordsPerFieldMaxPercentDocs() ;

  virtual void testStopwordsPerFieldMaxDocFreq() ;

  virtual void testNoFieldNamePollution() ;

  virtual void testTokenStream() ;

protected:
  std::shared_ptr<QueryAutoStopWordAnalyzerTest> shared_from_this()
  {
    return std::static_pointer_cast<QueryAutoStopWordAnalyzerTest>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/query/
