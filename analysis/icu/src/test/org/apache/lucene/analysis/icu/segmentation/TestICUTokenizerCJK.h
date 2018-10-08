#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
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
namespace org::apache::lucene::analysis::icu::segmentation
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * test ICUTokenizer with dictionary-based CJ segmentation
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE:
// @AwaitsFix(bugUrl="https://issues.apache.org/jira/browse/LUCENE-8222") public
// class TestICUTokenizerCJK extends
// org.apache.lucene.analysis.BaseTokenStreamTestCase
class TestICUTokenizerCJK : public BaseTokenStreamTestCase
{
public:
  std::shared_ptr<Analyzer> a;

  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestICUTokenizerCJK> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestICUTokenizerCJK> outerInstance);

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

  /**
   * test stolen from smartcn
   */
  virtual void testSimpleChinese() ;

  virtual void testTraditionalChinese() ;

  virtual void testChineseNumerics() ;

  /**
   * test stolen from kuromoji
   */
  virtual void testSimpleJapanese() ;

  /**
   * dictionary segmentation with emoji
   */
  virtual void testSimpleJapaneseWithEmoji() ;

  virtual void testJapaneseTypes() ;

  virtual void testKorean() ;

  /** make sure that we still tag korean as HANGUL (for further
   * decomposition/ngram/whatever) */
  virtual void testKoreanTypes() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

protected:
  std::shared_ptr<TestICUTokenizerCJK> shared_from_this()
  {
    return std::static_pointer_cast<TestICUTokenizerCJK>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu::segmentation
