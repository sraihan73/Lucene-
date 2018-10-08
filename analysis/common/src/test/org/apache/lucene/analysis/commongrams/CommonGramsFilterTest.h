#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

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
namespace org::apache::lucene::analysis::commongrams
{

using namespace org::apache::lucene::analysis;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * Tests CommonGrams(Query)Filter
 */
class CommonGramsFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(CommonGramsFilterTest)
private:
  static const std::shared_ptr<CharArraySet> commonWords;

public:
  virtual void testReset() ;

  virtual void testQueryReset() ;

  /**
   * This is for testing CommonGramsQueryFilter which outputs a set of tokens
   * optimized for querying with only one token at each position, either a
   * unigram or a bigram It also will not return a token for the final position
   * if the final word is already in the preceding bigram Example:(three
   * tokens/positions in)
   * "foo bar the"=&gt;"foo:1|bar:2,bar-the:2|the:3=&gt; "foo" "bar-the" (2
   * tokens out)
   *
   */
  virtual void testCommonGramsQueryFilter() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<CommonGramsFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<CommonGramsFilterTest> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testCommonGramsFilter() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<CommonGramsFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<CommonGramsFilterTest> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

  /**
   * Test that CommonGramsFilter works correctly in case-insensitive mode
   */
public:
  virtual void testCaseSensitive() ;

  /**
   * Test CommonGramsQueryFilter in the case that the last word is a stopword
   */
  virtual void testLastWordisStopWord() ;

  /**
   * Test CommonGramsQueryFilter in the case that the first word is a stopword
   */
  virtual void testFirstWordisStopWord() ;

  /**
   * Test CommonGramsQueryFilter in the case of a single (stop)word query
   */
  virtual void testOneWordQueryStopWord() ;

  /**
   * Test CommonGramsQueryFilter in the case of a single word query
   */
  virtual void testOneWordQuery() ;

  /**
   * Test CommonGramsQueryFilter when first and last words are stopwords.
   */
  virtual void TestFirstAndLastStopWord() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<CommonGramsFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<CommonGramsFilterTest> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<CommonGramsFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<CommonGramsFilterTest> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CommonGramsFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<CommonGramsFilterTest>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/commongrams/
