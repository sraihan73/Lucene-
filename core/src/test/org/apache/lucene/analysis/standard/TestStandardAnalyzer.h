#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestStandardAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestStandardAnalyzer)

  // LUCENE-5897: slow tokenization of strings of the form
  // (\p{WB:ExtendNumLet}[\p{WB:Format}\p{WB:Extend}]*)+
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testLargePartiallyMatchingToken() throws
  // Exception
  virtual void testLargePartiallyMatchingToken() ;

  virtual void testHugeDoc() ;

private:
  std::shared_ptr<Analyzer> a;

public:
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestStandardAnalyzer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestStandardAnalyzer> outerInstance);

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

public:
  void tearDown()  override;

  virtual void testArmenian() ;

  virtual void testAmharic() ;

  virtual void testArabic() ;

  virtual void testAramaic() ;

  virtual void testBengali() ;

  virtual void testFarsi() ;

  virtual void testGreek() ;

  virtual void testThai() ;

  virtual void testLao() ;

  virtual void testTibetan() ;

  /*
   * For chinese, tokenize as char (these can later form bigrams or whatever)
   */
  virtual void testChinese() ;

  virtual void testEmpty() ;

  /* test various jira issues this analyzer is related to */

  virtual void testLUCENE1545() ;

  /* Tests from StandardAnalyzer, just to show behavior is similar */
  virtual void testAlphanumericSA() ;

  virtual void testDelimitersSA() ;

  virtual void testApostrophesSA() ;

  virtual void testNumericSA() ;

  virtual void testTextWithNumbersSA() ;

  virtual void testVariousTextSA() ;

  virtual void testKoreanSA() ;

  virtual void testOffsets() ;

  virtual void testTypes() ;

  virtual void testUnicodeWordBreaks() ;

  virtual void testSupplementary() ;

  virtual void testKorean() ;

  virtual void testJapanese() ;

  virtual void testCombiningMarks() ;

  /**
   * Multiple consecutive chars in \p{WB:MidLetter}, \p{WB:MidNumLet},
   * and/or \p{MidNum} should trigger a token split.
   */
  virtual void testMid() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

  // Adds random graph after:
  virtual void testRandomHugeStringsGraphAfter() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestStandardAnalyzer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestStandardAnalyzer> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testNormalize();

  virtual void testMaxTokenLengthDefault() ;

  virtual void testMaxTokenLengthNonDefault() ;

protected:
  std::shared_ptr<TestStandardAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestStandardAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
