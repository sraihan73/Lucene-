#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::ckb
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test the Sorani analyzer
 */
class TestSoraniAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSoraniAnalyzer)

  /**
   * This test fails with NPE when the stopwords file is missing in classpath
   */
public:
  virtual void testResourcesAvailable();

  virtual void testStopwords() ;

  virtual void testCustomStopwords() ;

  virtual void testReusableTokenStream() ;

  virtual void testWithStemExclusionSet() ;

  /**
   * test we fold digits to latin-1
   * (these are somewhat rare, but generally a few % of digits still)
   */
  virtual void testDigits() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestSoraniAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestSoraniAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ckb/
