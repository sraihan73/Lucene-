#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
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
namespace org::apache::lucene::analysis::ngram
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * Tests {@link EdgeNGramTokenFilter} for correctness.
 */
class EdgeNGramTokenFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(EdgeNGramTokenFilterTest)
private:
  std::shared_ptr<TokenStream> input;

public:
  void setUp()  override;

  virtual void testInvalidInput() ;

  virtual void testInvalidInput2() ;

  virtual void testInvalidInput3() ;

  virtual void testFrontUnigram() ;

  virtual void testOversizedNgrams() ;

  virtual void testOversizedNgramsPreserveOriginal() ;

  virtual void testPreserveOriginal() ;

  virtual void testFrontRangeOfNgrams() ;

  virtual void testFilterPositions() ;

private:
  class PositionFilter : public TokenFilter
  {
    GET_CLASS_NAME(PositionFilter)

  private:
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    bool started = false;

  public:
    PositionFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override final;

    void reset()  override;

  protected:
    std::shared_ptr<PositionFilter> shared_from_this()
    {
      return std::static_pointer_cast<PositionFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testFirstTokenPositionIncrement() ;

  virtual void testSmallTokenInStream() ;

  virtual void testReset() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<EdgeNGramTokenFilterTest> outerInstance;

    int min = 0;
    int max = 0;
    bool preserveOriginal = false;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<EdgeNGramTokenFilterTest> outerInstance, int min,
        int max, bool preserveOriginal);

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
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<EdgeNGramTokenFilterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<EdgeNGramTokenFilterTest> outerInstance);

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
  virtual void testGraphs() ;

  virtual void testSupplementaryCharacters() ;

  virtual void testEndPositionIncrement() ;

protected:
  std::shared_ptr<EdgeNGramTokenFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<EdgeNGramTokenFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ngram/
