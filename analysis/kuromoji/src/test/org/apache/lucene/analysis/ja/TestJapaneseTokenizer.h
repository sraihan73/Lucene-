#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class UserDictionary;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis::ja
{
class JapaneseTokenizer;
}
namespace org::apache::lucene::analysis
{
class Tokenizer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis::ja
{
class GraphvizFormatter;
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
namespace org::apache::lucene::analysis::ja
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using namespace org::apache::lucene::analysis::ja::tokenattributes;

class TestJapaneseTokenizer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestJapaneseTokenizer)

public:
  static std::shared_ptr<UserDictionary> readDict();

private:
  std::shared_ptr<Analyzer> analyzer, analyzerNormal, analyzerNormalNBest,
      analyzerNoPunct, extendedModeAnalyzerNoPunct;

  std::shared_ptr<JapaneseTokenizer> makeTokenizer(bool discardPunctuation,
                                                   Mode mode);

  std::shared_ptr<Analyzer> makeAnalyzer(std::shared_ptr<Tokenizer> t);

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

    std::shared_ptr<Tokenizer> t;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance,
        std::shared_ptr<Tokenizer> t);

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
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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

public:
  void tearDown()  override;

  virtual void testNormalMode() ;

  virtual void testNormalModeNbest() ;

  virtual void testSearchModeNbest() ;

private:
  std::deque<std::wstring>
  makeTokenList(std::shared_ptr<Analyzer> a,
                const std::wstring &in_) ;

  bool checkToken(std::shared_ptr<Analyzer> a, const std::wstring &in_,
                  const std::wstring &requitedToken) ;

public:
  virtual void testNBestCost() ;

  virtual void testDecomposition1() ;

  virtual void testDecomposition2() ;

  virtual void testDecomposition3() ;

  virtual void testDecomposition4() ;

  /* Note this is really a stupid test just to see if things arent horribly
   * slow. ideally the test would actually fail instead of hanging...
   */
  virtual void testDecomposition5() ;

  /*
    // NOTE: intentionally fails!  Just trying to debug this
    // one input...
  public void testDecomposition6() throws Exception {
    assertAnalyzesTo(analyzer, "奈良先端科学技術大学院大学",
      new std::wstring[] { "これ", "は", "本", "で", "は", "ない" },
      new int[] { 0, 2, 3, 4, 5, 6 },
      new int[] { 2, 3, 4, 5, 6, 8 }
                     );
  }
  */

  /** Tests that sentence offset is incorporated into the resulting offsets */
  virtual void testTwoSentences() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

  virtual void testRandomHugeStringsMockGraphAfter() ;

private:
  class AnalyzerAnonymousInnerClass7 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass7)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass7(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance);

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

public:
  virtual void testLargeDocReliability() ;

  /** simple test for supplementary characters */
  virtual void testSurrogates() ;

  /** random test ensuring we don't ever split supplementaries */
  virtual void testSurrogates2() ;

  virtual void testOnlyPunctuation() ;

  virtual void testOnlyPunctuationExtended() ;

  // note: test is kinda silly since kuromoji emits punctuation tokens.
  // but, when/if we filter these out it will be useful.
  virtual void testEnd() ;

  virtual void testUserDict() ;

  virtual void testUserDict2() ;

  virtual void testUserDict3() ;

  // HMM: fails (segments as a/b/cd/efghij)... because the
  // two paths have exactly equal paths (1 KNOWN + 1
  // UNKNOWN) and we don't seem to favor longer KNOWN /
  // shorter UNKNOWN matches:

  /*
  public void testUserDict4() throws Exception {
    // Test entry that has another entry as prefix
    assertTokenStreamContents(analyzer.tokenStream("foo", "abcdefghij"),
                              new std::wstring[] { "ab", "cd", "efg", "hij"  },
                              new int[] { 0, 2, 4, 7 },
                              new int[] { 2, 4, 7, 10 },
                              new Integer(10)
    );
  }
  */

  virtual void testSegmentation() ;

  virtual void testLatticeToDot() ;

private:
  class AnalyzerAnonymousInnerClass8 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass8)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::ja::GraphvizFormatter> gv2;

  public:
    AnalyzerAnonymousInnerClass8(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::ja::GraphvizFormatter>
            gv2);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass8>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  void assertReadings(const std::wstring &input,
                      std::deque<std::wstring> &readings) ;

  void assertPronunciations(
      const std::wstring &input,
      std::deque<std::wstring> &pronunciations) ;

  void assertBaseForms(const std::wstring &input,
                       std::deque<std::wstring> &baseForms) ;

  void assertInflectionTypes(
      const std::wstring &input,
      std::deque<std::wstring> &inflectionTypes) ;

  void assertInflectionForms(
      const std::wstring &input,
      std::deque<std::wstring> &inflectionForms) ;

  void assertPartsOfSpeech(
      const std::wstring &input,
      std::deque<std::wstring> &partsOfSpeech) ;

public:
  virtual void testReadings() ;

  virtual void testReadings2() ;

  virtual void testPronunciations() ;

  virtual void testPronunciations2() ;

  virtual void testBasicForms() ;

  virtual void testInflectionTypes() ;

  virtual void testInflectionForms() ;

  virtual void testPartOfSpeech() ;

  // TODO: the next 2 tests are no longer using the first/last word ids, maybe
  // lookup the words and fix? do we have a possibility to actually lookup the
  // first and last word from dictionary?
  virtual void testYabottai() ;

  virtual void testTsukitosha() ;

  virtual void testBocchan() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testBocchanBig() throws Exception
  virtual void testBocchanBig() ;

  /*
  public void testWikipedia() throws Exception {
    final FileInputStream fis = new
  FileInputStream("/q/lucene/jawiki-20120220-pages-articles.xml"); final Reader
  r = new BufferedReader(new InputStreamReader(fis, StandardCharsets.UTF_8));

    final long startTimeNS = System.nanoTime();
    bool done = false;
    long compoundCount = 0;
    long nonCompoundCount = 0;
    long netOffset = 0;
    while (!done) {
      final TokenStream ts = analyzer.tokenStream("ignored", r);
      ts.reset();
      final PositionIncrementAttribute posIncAtt =
  ts.addAttribute(PositionIncrementAttribute.class); final OffsetAttribute
  offsetAtt = ts.addAttribute(OffsetAttribute.class); int count = 0; while
  (true) { if (!ts.incrementToken()) { done = true; break;
        }
        count++;
        if (posIncAtt.getPositionIncrement() == 0) {
          compoundCount++;
        } else {
          nonCompoundCount++;
          if (nonCompoundCount % 1000000 == 0) {
            System.out.println(std::wstring.format("%.2f msec [pos=%d, %d, %d]",
                                             (System.nanoTime()-startTimeNS)/1000000.0,
                                             netOffset +
  offsetAtt.startOffset(), nonCompoundCount, compoundCount));
          }
        }
        if (count == 100000000) {
          System.out.println("  again...");
          break;
        }
      }
      ts.end();
      netOffset += offsetAtt.endOffset();
    }
    System.out.println("compoundCount=" + compoundCount + " nonCompoundCount=" +
  nonCompoundCount); r.close();
  }
  */

private:
  void doTestBocchan(int numIterations) ;

public:
  virtual void testWithPunctuation() ;

  virtual void testCompoundOverPunctuation() ;

  virtual void testEmptyUserDict() ;

private:
  class AnalyzerAnonymousInnerClass9 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass9)
  private:
    std::shared_ptr<TestJapaneseTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> emptyDict;

  public:
    AnalyzerAnonymousInnerClass9(
        std::shared_ptr<TestJapaneseTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> emptyDict);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass9> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass9>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testBigDocument() ;

protected:
  std::shared_ptr<TestJapaneseTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<TestJapaneseTokenizer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
