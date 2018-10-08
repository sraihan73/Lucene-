#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/** tests for classicanalyzer */
class TestClassicAnalyzer : public BaseTokenStreamTestCase
{

private:
  std::shared_ptr<Analyzer> a;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testMaxTermLength() ;

  virtual void testMaxTermLength2() ;

  virtual void testMaxTermLength3() ;

  virtual void testAlphanumeric() ;

  virtual void testUnderscores() ;

  virtual void testDelimiters() ;

  virtual void testApostrophes() ;

  virtual void testTSADash() ;

  virtual void testCompanyNames() ;

  virtual void testLucene1140() ;

  virtual void testDomainNames() ;

  virtual void testEMailAddresses() ;

  virtual void testNumeric() ;

  virtual void testTextWithNumbers() ;

  virtual void testVariousText() ;

  virtual void testAcronyms() ;

  virtual void testCPlusPlusHash() ;

  virtual void testKorean() ;

  // Compliance with the "old" JavaCC-based analyzer, see:
  // https://issues.apache.org/jira/browse/LUCENE-966#action_12516752

  virtual void testComplianceFileName() ;

  virtual void testComplianceNumericIncorrect() ;

  virtual void testComplianceNumericLong() ;

  virtual void testComplianceNumericFile() ;

  virtual void testComplianceNumericWithUnderscores() ;

  virtual void testComplianceNumericWithDash() ;

  virtual void testComplianceManyTokens() ;

  virtual void testJava14BWCompatibility() ;

  /**
   * Make sure we skip wicked long terms.
   */
  virtual void testWickedLongTerm() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

protected:
  std::shared_ptr<TestClassicAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestClassicAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
