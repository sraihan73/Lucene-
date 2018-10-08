#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/uima/UIMABaseAnalyzer.h"

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
namespace org::apache::lucene::analysis::uima
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Testcase for {@link UIMABaseAnalyzer}
 */
class UIMABaseAnalyzerTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(UIMABaseAnalyzerTest)

private:
  std::shared_ptr<UIMABaseAnalyzer> analyzer;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @After public void tearDown() throws Exception
  void tearDown()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void baseUIMAAnalyzerStreamTest() throws
  // Exception
  virtual void baseUIMAAnalyzerStreamTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void baseUIMAAnalyzerIntegrationTest() throws
  // Exception
  virtual void baseUIMAAnalyzerIntegrationTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @AwaitsFix(bugUrl =
  // "https://issues.apache.org/jira/browse/LUCENE-3869") public void
  // testRandomStrings() throws Exception
  virtual void testRandomStrings() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @AwaitsFix(bugUrl =
  // "https://issues.apache.org/jira/browse/LUCENE-3869") public void
  // testRandomStringsWithConfigurationParameters() throws Exception
  virtual void
  testRandomStringsWithConfigurationParameters() ;

protected:
  std::shared_ptr<UIMABaseAnalyzerTest> shared_from_this()
  {
    return std::static_pointer_cast<UIMABaseAnalyzerTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/uima/
