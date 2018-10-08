#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
 * Test the Arabic Normalization Filter
 *
 */
class TestArabicStemFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestArabicStemFilter)

public:
  virtual void testAlPrefix() ;

  virtual void testWalPrefix() ;

  virtual void testBalPrefix() ;

  virtual void testKalPrefix() ;

  virtual void testFalPrefix() ;

  virtual void testLlPrefix() ;

  virtual void testWaPrefix() ;

  virtual void testAhSuffix() ;

  virtual void testAnSuffix() ;

  virtual void testAtSuffix() ;

  virtual void testWnSuffix() ;

  virtual void testYnSuffix() ;

  virtual void testYhSuffix() ;

  virtual void testYpSuffix() ;

  virtual void testHSuffix() ;

  virtual void testPSuffix() ;

  virtual void testYSuffix() ;

  virtual void testComboPrefSuf() ;

  virtual void testComboSuf() ;

  virtual void testShouldntStem() ;

  virtual void testNonArabic() ;

  virtual void testWithKeywordAttribute() ;

private:
  void check(const std::wstring &input,
             const std::wstring &expected) ;

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestArabicStemFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestArabicStemFilter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestArabicStemFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestArabicStemFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ar/
