#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
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
namespace org::apache::lucene::analysis::icu::segmentation
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestICUTokenizer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestICUTokenizer)

public:
  virtual void testHugeDoc() ;

  virtual void testHugeTerm2() ;

private:
  std::shared_ptr<Analyzer> a;

public:
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestICUTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestICUTokenizer> outerInstance);

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

  virtual void testKhmer() ;
  virtual void testLao() ;

  virtual void testMyanmar() ;

  virtual void testThai() ;

  virtual void testTibetan() ;

  /*
   * For chinese, tokenize as char (these can later form bigrams or whatever)
   */
  virtual void testChinese() ;

  virtual void testHebrew() ;

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

  virtual void testReusableTokenStream() ;

  virtual void testOffsets() ;

  virtual void testTypes() ;

  virtual void testKorean() ;

  virtual void testJapanese() ;

  /** simple emoji */
  virtual void testEmoji() ;

  /** emoji zwj sequence */
  virtual void testEmojiSequence() ;

  /** emoji zwj sequence with fitzpatrick modifier */
  virtual void testEmojiSequenceWithModifier() ;

  /** regional indicator */
  virtual void testEmojiRegionalIndicator() ;

  /** variation sequence */
  virtual void testEmojiVariationSequence() ;

  virtual void testEmojiTagSequence() ;

  virtual void testEmojiTokenization() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

  virtual void testTokenAttributes() ;

  /** test for bugs like http://bugs.icu-project.org/trac/ticket/10767 */
  virtual void testICUConcurrency() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestICUTokenizer> outerInstance;

    std::shared_ptr<CountDownLatch> startingGun;
    int i = 0;

  public:
    ThreadAnonymousInnerClass(std::shared_ptr<TestICUTokenizer> outerInstance,
                              std::shared_ptr<CountDownLatch> startingGun,
                              int i);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestICUTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<TestICUTokenizer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
