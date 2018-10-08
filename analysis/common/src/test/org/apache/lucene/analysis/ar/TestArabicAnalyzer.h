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
namespace org::apache::lucene::analysis::ar
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test the Arabic Analyzer
 *
 */
class TestArabicAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestArabicAnalyzer)

  /** This test fails with NPE when the
   * stopwords file is missing in classpath */
public:
  virtual void testResourcesAvailable();

  /**
   * Some simple tests showing some features of the analyzer, how some regular
   * forms will conflate
   */
  virtual void testBasicFeatures() ;

  /**
   * Simple tests to show things are getting reset correctly, etc.
   */
  virtual void testReusableTokenStream() ;

  /**
   * Non-arabic text gets treated in a similar way as SimpleAnalyzer.
   */
  virtual void testEnglishInput() ;

  /**
   * Test that custom stopwords work, and are not case-sensitive.
   */
  virtual void testCustomStopwords() ;

  virtual void testWithStemExclusionSet() ;

  /**
   * test we fold digits to latin-1
   */
  virtual void testDigits() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestArabicAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestArabicAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ar
