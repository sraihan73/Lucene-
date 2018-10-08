#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
}

namespace org::apache::lucene::analysis::shingle
{
class ShingleFilter;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis::shingle
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Token = org::apache::lucene::analysis::Token;

class ShingleFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(ShingleFilterTest)

public:
  static std::deque<std::shared_ptr<Token>> const TEST_TOKEN;

  static std::deque<int> const UNIGRAM_ONLY_POSITION_INCREMENTS;

  static std::deque<std::wstring> const UNIGRAM_ONLY_TYPES;

  static std::deque<std::shared_ptr<Token>> testTokenWithHoles;

  static std::deque<std::shared_ptr<Token>> const BI_GRAM_TOKENS;

  static std::deque<int> const BI_GRAM_POSITION_INCREMENTS;

  static std::deque<std::wstring> const BI_GRAM_TYPES;

  static std::deque<std::shared_ptr<Token>> const BI_GRAM_TOKENS_WITH_HOLES;

  static std::deque<int> const BI_GRAM_POSITION_INCREMENTS_WITH_HOLES;

private:
  static std::deque<std::wstring> const BI_GRAM_TYPES_WITH_HOLES;

public:
  static std::deque<std::shared_ptr<Token>> const
      BI_GRAM_TOKENS_WITHOUT_UNIGRAMS;

  static std::deque<int> const BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS;

  static std::deque<std::wstring> const BI_GRAM_TYPES_WITHOUT_UNIGRAMS;

  static std::deque<std::shared_ptr<Token>> const
      BI_GRAM_TOKENS_WITH_HOLES_WITHOUT_UNIGRAMS;

  static std::deque<int> const
      BI_GRAM_POSITION_INCREMENTS_WITH_HOLES_WITHOUT_UNIGRAMS;

  static std::deque<std::shared_ptr<Token>> const TEST_SINGLE_TOKEN;

  static std::deque<std::shared_ptr<Token>> const SINGLE_TOKEN;

  static std::deque<int> const SINGLE_TOKEN_INCREMENTS;

  static std::deque<std::wstring> const SINGLE_TOKEN_TYPES;

  static std::deque<std::shared_ptr<Token>> const EMPTY_TOKEN_ARRAY;

  static std::deque<int> const EMPTY_TOKEN_INCREMENTS_ARRAY;

  static std::deque<std::wstring> const EMPTY_TOKEN_TYPES_ARRAY;

  static std::deque<std::shared_ptr<Token>> const TRI_GRAM_TOKENS;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS;

  static std::deque<std::wstring> const TRI_GRAM_TYPES;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_WITHOUT_UNIGRAMS;

  static std::deque<std::shared_ptr<Token>> const FOUR_GRAM_TOKENS;

  static std::deque<int> const FOUR_GRAM_POSITION_INCREMENTS;

  static std::deque<std::wstring> const FOUR_GRAM_TYPES;

  static std::deque<std::shared_ptr<Token>> const
      FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS;

  static std::deque<int> const FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS;

  static std::deque<std::wstring> const FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS;

  static std::deque<std::shared_ptr<Token>> const TRI_GRAM_TOKENS_MIN_TRI_GRAM;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_MIN_TRI_GRAM;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<std::wstring> const
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<std::shared_ptr<Token>> const
      FOUR_GRAM_TOKENS_MIN_TRI_GRAM;

  static std::deque<int> const FOUR_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM;

  static std::deque<std::wstring> const FOUR_GRAM_TYPES_MIN_TRI_GRAM;

  static std::deque<std::shared_ptr<Token>> const
      FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<int> const
      FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<std::wstring> const
      FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM;

  static std::deque<std::shared_ptr<Token>> const
      FOUR_GRAM_TOKENS_MIN_FOUR_GRAM;

  static std::deque<int> const FOUR_GRAM_POSITION_INCREMENTS_MIN_FOUR_GRAM;

  static std::deque<std::wstring> const FOUR_GRAM_TYPES_MIN_FOUR_GRAM;

  static std::deque<std::shared_ptr<Token>> const
      FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM;

  static std::deque<int> const
      FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM;

  static std::deque<std::wstring> const
      FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM;

  static std::deque<std::shared_ptr<Token>> const BI_GRAM_TOKENS_NO_SEPARATOR;

  static std::deque<int> const BI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR;

  static std::deque<std::wstring> const BI_GRAM_TYPES_NO_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<int> const
      BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<std::wstring> const
      BI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const TRI_GRAM_TOKENS_NO_SEPARATOR;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_NO_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<std::wstring> const
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const BI_GRAM_TOKENS_ALT_SEPARATOR;

  static std::deque<int> const BI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR;

  static std::deque<std::wstring> const BI_GRAM_TYPES_ALT_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<int> const
      BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<std::wstring> const
      BI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_ALT_SEPARATOR;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_ALT_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<std::wstring> const
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_NULL_SEPARATOR;

  static std::deque<int> const TRI_GRAM_POSITION_INCREMENTS_NULL_SEPARATOR;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_NULL_SEPARATOR;

  static std::deque<std::shared_ptr<Token>> const
      TEST_TOKEN_POS_INCR_EQUAL_TO_N;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N;

  static std::deque<std::wstring> const TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS;

  static std::deque<std::wstring> const
      TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS;

  static std::deque<std::shared_ptr<Token>> const
      TEST_TOKEN_POS_INCR_GREATER_THAN_N;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N;
  static std::deque<std::wstring> const TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N;

  static std::deque<std::shared_ptr<Token>> const
      TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS;

  static std::deque<int> const
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS;

  static std::deque<std::wstring> const
      TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS;

  void setUp()  override;

  /*
   * Class under test for void ShingleFilter(TokenStream, int)
   */
  virtual void testBiGramFilter() ;

  virtual void testBiGramFilterWithHoles() ;

  virtual void testBiGramFilterWithoutUnigrams() ;

  virtual void testBiGramFilterWithHolesWithoutUnigrams() ;

  virtual void testBiGramFilterWithSingleToken() ;

  virtual void
  testBiGramFilterWithSingleTokenWithoutUnigrams() ;

  virtual void testBiGramFilterWithEmptyTokenStream() ;

  virtual void
  testBiGramFilterWithEmptyTokenStreamWithoutUnigrams() ;

  virtual void testTriGramFilter() ;

  virtual void testTriGramFilterWithoutUnigrams() ;

  virtual void testFourGramFilter() ;

  virtual void testFourGramFilterWithoutUnigrams() ;

  virtual void testTriGramFilterMinTriGram() ;

  virtual void testTriGramFilterWithoutUnigramsMinTriGram() ;

  virtual void testFourGramFilterMinTriGram() ;

  virtual void testFourGramFilterWithoutUnigramsMinTriGram() ;

  virtual void testFourGramFilterMinFourGram() ;

  virtual void
  testFourGramFilterWithoutUnigramsMinFourGram() ;

  virtual void testBiGramFilterNoSeparator() ;

  virtual void testBiGramFilterWithoutUnigramsNoSeparator() ;
  virtual void testTriGramFilterNoSeparator() ;

  virtual void testTriGramFilterWithoutUnigramsNoSeparator() ;

  virtual void testBiGramFilterAltSeparator() ;

  virtual void testBiGramFilterWithoutUnigramsAltSeparator() ;
  virtual void testTriGramFilterAltSeparator() ;

  virtual void
  testTriGramFilterWithoutUnigramsAltSeparator() ;

  virtual void testTriGramFilterNullSeparator() ;

  virtual void testPositionIncrementEqualToN() ;

  virtual void
  testPositionIncrementEqualToNWithoutUnigrams() ;

  virtual void testPositionIncrementGreaterThanN() ;

  virtual void
  testPositionIncrementGreaterThanNWithoutUnigrams() ;

  virtual void testReset() ;

  virtual void
  testOutputUnigramsIfNoShinglesSingleTokenCase() ;

  virtual void
  testOutputUnigramsIfNoShinglesWithSimpleBigram() ;

  virtual void
  testOutputUnigramsIfNoShinglesWithSimpleUnigramlessBigram() throw(
      IOException);

  virtual void
  testOutputUnigramsIfNoShinglesWithMultipleInputTokens() ;

protected:
  virtual void shingleFilterTest(
      int maxSize, std::deque<std::shared_ptr<Token>> &tokensToShingle,
      std::deque<std::shared_ptr<Token>> &tokensToCompare,
      std::deque<int> &positionIncrements, std::deque<std::wstring> &types,
      bool outputUnigrams) ;

  virtual void
  shingleFilterTest(int minSize, int maxSize,
                    std::deque<std::shared_ptr<Token>> &tokensToShingle,
                    std::deque<std::shared_ptr<Token>> &tokensToCompare,
                    std::deque<int> &positionIncrements,
                    std::deque<std::wstring> &types,
                    bool outputUnigrams) ;

  virtual void
  shingleFilterTest(int minSize, int maxSize,
                    std::deque<std::shared_ptr<Token>> &tokensToShingle,
                    std::deque<std::shared_ptr<Token>> &tokensToCompare,
                    std::deque<int> &positionIncrements,
                    std::deque<std::wstring> &types, bool outputUnigrams,
                    bool outputUnigramsIfNoShingles) ;

  virtual void shingleFilterTest(
      const std::wstring &tokenSeparator, int minSize, int maxSize,
      std::deque<std::shared_ptr<Token>> &tokensToShingle,
      std::deque<std::shared_ptr<Token>> &tokensToCompare,
      std::deque<int> &positionIncrements, std::deque<std::wstring> &types,
      bool outputUnigrams) ;

  virtual void
  shingleFilterTestCommon(std::shared_ptr<ShingleFilter> filter,
                          std::deque<std::shared_ptr<Token>> &tokensToCompare,
                          std::deque<int> &positionIncrements,
                          std::deque<std::wstring> &types) ;

private:
  static std::shared_ptr<Token> createToken(const std::wstring &term, int start,
                                            int offset);

  static std::shared_ptr<Token> createToken(const std::wstring &term, int start,
                                            int offset, int positionIncrement);

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<ShingleFilterTest> outerInstance);

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

  /** blast some random large strings through the analyzer */
public:
  virtual void testRandomHugeStrings() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<ShingleFilterTest> outerInstance);

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
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<ShingleFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testTrailingHole1() ;

  virtual void testTrailingHole2() ;

  virtual void testTwoTrailingHoles() ;

  virtual void testTwoTrailingHolesTriShingle() ;

  virtual void
  testTwoTrailingHolesTriShingleWithTokenFiller() ;

  virtual void testPositionLength() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<ShingleFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<ShingleFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<ShingleFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass6>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass7 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass7)
  private:
    std::shared_ptr<ShingleFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass7(
        std::shared_ptr<ShingleFilterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass7>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ShingleFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<ShingleFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::shingle
