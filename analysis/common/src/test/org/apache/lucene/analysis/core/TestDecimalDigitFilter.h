#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::util
{
class SparseFixedBitSet;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::analysis::core
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using SparseFixedBitSet = org::apache::lucene::util::SparseFixedBitSet;

/**
 * Tests for {@link DecimalDigitFilter}
 */
class TestDecimalDigitFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestDecimalDigitFilter)
private:
  std::shared_ptr<Analyzer> tokenized;
  std::shared_ptr<Analyzer> keyword;

  static std::shared_ptr<SparseFixedBitSet> DECIMAL_DIGIT_CODEPOINTS;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void
  // init_DECIMAL_DIGIT_CODEPOINTS()
  static void init_DECIMAL_DIGIT_CODEPOINTS();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void
  // destroy_DECIMAL_DIGIT_CODEPOINTS()
  static void destroy_DECIMAL_DIGIT_CODEPOINTS();

  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestDecimalDigitFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestDecimalDigitFilter> outerInstance);

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

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDecimalDigitFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestDecimalDigitFilter> outerInstance);

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
  void tearDown()  override;

  /**
   * test that digits are normalized
   */
  virtual void testSimple() ;

  /**
   * test that double struck digits are normalized
   */
  virtual void testDoubleStruck() ;

  /**
   * test sequences of digits mixed with other random simple string data
   */
  virtual void testRandomSequences() ;

  /**
   * test each individual digit in different locations of strings.
   */
  virtual void testRandom() ;

  /**
   * check the filter is a no-op for the empty string term
   */
  virtual void testEmptyTerm() ;

  /**
   * blast some random strings through the filter
   */
  virtual void testRandomStrings() ;

  /** returns a psuedo-random codepoint which is a Decimal Digit */
  static int getRandomDecimalDigit(std::shared_ptr<Random> r);

protected:
  std::shared_ptr<TestDecimalDigitFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestDecimalDigitFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
