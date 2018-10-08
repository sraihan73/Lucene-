#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeywordMarkerFilter.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class Tokenizer;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

using namespace org::apache::lucene::analysis::miscellaneous;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterFilter.*;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterIterator.DEFAULT_WORD_DELIM_TABLE;

/**
 * New WordDelimiterFilter tests... most of the tests are in ConvertedLegacyTest
 * TODO: should explicitly test things like protWords and not rely on
 * the factory tests in Solr.
 */
class TestWordDelimiterFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestWordDelimiterFilter)

  /*
  public void testPerformance() throws IOException {
    std::wstring s = "now is the time-for all good men to come to-the aid of their
  country."; Token tok = new Token(); long start = System.currentTimeMillis();
    int ret=0;
    for (int i=0; i<1000000; i++) {
      StringReader r = new StringReader(s);
      TokenStream ts = new WhitespaceTokenizer(r);
      ts = new WordDelimiterFilter(ts, 1,1,1,1,0);

      while (ts.next(tok) != null) ret++;
    }

    System.out.println("ret="+ret+" time="+(System.currentTimeMillis()-start));
  }
  ***/

public:
  virtual void testOffsets() ;

  virtual void testOffsetChange() ;

  virtual void testOffsetChange2() ;

  virtual void testOffsetChange3() ;

  virtual void testOffsetChange4() ;

  virtual void
  doSplit(const std::wstring &input,
          std::deque<std::wstring> &output) ;

  virtual void testSplits() ;

  virtual void doSplitPossessive(
      int stemPossessive, const std::wstring &input,
      std::deque<std::wstring> &output) ;

  /*
   * Test option that allows disabling the special "'s" stemming, instead
   * treating the single quote like other delimiters.
   */
  virtual void testPossessives() ;

  /*
   * Set a large position increment gap of 10 if the token is "largegap" or "/"
   */
private:
  class LargePosIncTokenFilter final : public TokenFilter
  {
    GET_CLASS_NAME(LargePosIncTokenFilter)
  private:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  protected:
    LargePosIncTokenFilter(std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<LargePosIncTokenFilter> shared_from_this()
    {
      return std::static_pointer_cast<LargePosIncTokenFilter>(
          TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testPositionIncrements() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protWords);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protWords);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protWords);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testKeywordFilter() ;

private:
  std::shared_ptr<Analyzer>
  keywordTestAnalyzer(int flags) ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  private:
    class KeywordMarkerFilterAnonymousInnerClass : public KeywordMarkerFilter
    {
      GET_CLASS_NAME(KeywordMarkerFilterAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance;

    public:
      KeywordMarkerFilterAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance,
          std::shared_ptr<org::apache::lucene::analysis::Tokenizer> tokenizer);

    private:
      const std::shared_ptr<CharTermAttribute> term;

    public:
      bool isKeyword() override;

    protected:
      std::shared_ptr<KeywordMarkerFilterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<KeywordMarkerFilterAnonymousInnerClass>(
            KeywordMarkerFilter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          Analyzer::shared_from_this());
    }
  };

  /** concat numbers + words + all */
public:
  virtual void testLotsOfConcatenating() ;

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          Analyzer::shared_from_this());
    }
  };

  /** concat numbers + words + all + preserve original */
public:
  virtual void testLotsOfConcatenating2() ;

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass6>(
          Analyzer::shared_from_this());
    }
  };

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass7 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass7)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass7(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protectedWords);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass7>(
          Analyzer::shared_from_this());
    }
  };

  /** blast some enormous random strings through the analyzer */
public:
  virtual void testRandomHugeStrings() ;

private:
  class AnalyzerAnonymousInnerClass8 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass8)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass8(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protectedWords);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass8>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass9 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass9)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass9(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protectedWords);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass9> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass9>(
          Analyzer::shared_from_this());
    }
  };

  /*
  public void testToDot() throws Exception {
    int flags = GENERATE_WORD_PARTS | GENERATE_NUMBER_PARTS | CATENATE_ALL |
  SPLIT_ON_CASE_CHANGE | SPLIT_ON_NUMERICS | STEM_ENGLISH_POSSESSIVE |
  PRESERVE_ORIGINAL | CATENATE_WORDS | CATENATE_NUMBERS |
  STEM_ENGLISH_POSSESSIVE; std::wstring text = "PowerSystem2000-5-Shot's";
    WordDelimiterFilter wdf = new WordDelimiterFilter(new CannedTokenStream(new
  Token(text, 0, text.length())), DEFAULT_WORD_DELIM_TABLE, flags, null);
    //StringWriter sw = new StringWriter();
    // TokenStreamToDot toDot = new TokenStreamToDot(text, wdf, new
  PrintWriter(sw)); PrintWriter pw = new PrintWriter("/x/tmp/before.dot");
    TokenStreamToDot toDot = new TokenStreamToDot(text, wdf, pw);
    toDot.toDot();
    pw.close();
    System.out.println("TEST DONE");
    //System.out.println("DOT:\n" + sw.toString());
  }
  */

public:
  virtual void testOnlyNumbers() ;

private:
  class AnalyzerAnonymousInnerClass10 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass10)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass10(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass10> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass10>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testNumberPunct() ;

private:
  class AnalyzerAnonymousInnerClass11 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass11)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass11(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass11> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass11>(
          Analyzer::shared_from_this());
    }
  };

private:
  std::shared_ptr<Analyzer> getAnalyzer(int const flags);

private:
  class AnalyzerAnonymousInnerClass12 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass12)
  private:
    std::shared_ptr<TestWordDelimiterFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass12(
        std::shared_ptr<TestWordDelimiterFilter> outerInstance, int flags);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass12> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass12>(
          Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestWordDelimiterFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestWordDelimiterFilter>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
