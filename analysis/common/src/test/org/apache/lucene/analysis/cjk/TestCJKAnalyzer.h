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
namespace org::apache::lucene::analysis::charfilter
{
class NormalizeCharMap;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::cjk
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/**
 * Most tests adopted from TestCJKTokenizer
 */
class TestCJKAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCJKAnalyzer)
private:
  std::shared_ptr<Analyzer> analyzer;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testJa1() ;

  virtual void testJa2() ;

  virtual void testC() ;

  /**
   * LUCENE-2207: wrong offset calculated by end()
   */
  virtual void testFinalOffset() ;

  virtual void testMix() ;

  virtual void testMix2() ;

  /**
   * Non-english text (outside of CJK) is treated normally, according to unicode
   * rules
   */
  virtual void testNonIdeographic() ;

  /**
   * Same as the above, except with a nonspacing mark to show correctness.
   */
  virtual void testNonIdeographicNonLetter() ;

  virtual void testSurrogates() ;

  virtual void testReusableTokenStream() ;

  virtual void testSingleChar() ;

  virtual void testTokenStream() ;

  /** test that offsets are correct when mappingcharfilter is previously applied
   */
  virtual void testChangedOffsets() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCJKAnalyzer> outerInstance;

    std::shared_ptr<NormalizeCharMap> norm;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestCJKAnalyzer> outerInstance,
                                std::shared_ptr<NormalizeCharMap> norm);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class FakeStandardTokenizer : public TokenFilter
  {
    GET_CLASS_NAME(FakeStandardTokenizer)
  public:
    const std::shared_ptr<TypeAttribute> typeAtt =
        addAttribute(TypeAttribute::typeid);

    FakeStandardTokenizer(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<FakeStandardTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<FakeStandardTokenizer>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testSingleChar2() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCJKAnalyzer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestCJKAnalyzer> outerInstance);

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

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomHugeStrings() ;

  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestCJKAnalyzer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestCJKAnalyzer> outerInstance);

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

protected:
  std::shared_ptr<TestCJKAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestCJKAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::cjk
