#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeywordMarkerFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <iostream>
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
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

using namespace org::apache::lucene::analysis::miscellaneous;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterGraphFilter.*;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterIterator.DEFAULT_WORD_DELIM_TABLE;

/**
 * New WordDelimiterGraphFilter tests... most of the tests are in
 * ConvertedLegacyTest
 * TODO: should explicitly test things like protWords and not rely on
 * the factory tests in Solr.
 */
class TestWordDelimiterGraphFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestWordDelimiterGraphFilter)

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

  virtual void testTokenType() ;

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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protWords;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags);

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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags);

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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags);

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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass7(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass8(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass9(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
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

private:
  std::shared_ptr<Analyzer> getAnalyzer(int flags);

  std::shared_ptr<Analyzer>
  getAnalyzer(int flags, std::shared_ptr<CharArraySet> protectedWords);

private:
  class AnalyzerAnonymousInnerClass10 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass10)
  private:
    std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance;

    int flags = 0;
    std::shared_ptr<CharArraySet> protectedWords;

  public:
    AnalyzerAnonymousInnerClass10(
        std::shared_ptr<TestWordDelimiterGraphFilter> outerInstance, int flags,
        std::shared_ptr<CharArraySet> protectedWords);

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

private:
  static bool has(int flags, int flag);

  static bool isEnglishPossessive(const std::wstring &text, int pos);

private:
  class WordPart : public std::enable_shared_from_this<WordPart>
  {
    GET_CLASS_NAME(WordPart)
  public:
    const std::wstring part;
    const int startOffset;
    const int endOffset;
    const int type;

    WordPart(const std::wstring &text, int startOffset, int endOffset);

    virtual std::wstring toString();
  };

private:
  static constexpr int NUMBER = 0;
  static constexpr int LETTER = 1;
  static constexpr int DELIM = 2;

  static int toType(wchar_t ch);

  /** Does (hopefully) the same thing as WordDelimiterGraphFilter, according to
   * the flags, but more slowly, returning all string paths combinations. */
  std::shared_ptr<Set<std::wstring>> slowWDF(const std::wstring &text,
                                             int flags);

  void add(std::shared_ptr<StringBuilder> path, const std::wstring &part);

  void add(std::shared_ptr<StringBuilder> path,
           std::deque<std::shared_ptr<WordPart>> &wordParts, int from, int to);

  void addWithSpaces(std::shared_ptr<StringBuilder> path,
                     std::deque<std::shared_ptr<WordPart>> &wordParts,
                     int from, int to);

  /** Finds the end (exclusive) of the series of part with the same type */
  int endOfRun(std::deque<std::shared_ptr<WordPart>> &wordParts, int start);

  /** Recursively enumerates all paths through the word parts */
  void enumerate(int flags, int upto, const std::wstring &text,
                 std::deque<std::shared_ptr<WordPart>> &wordParts,
                 std::shared_ptr<Set<std::wstring>> paths,
                 std::shared_ptr<StringBuilder> path);

public:
  virtual void testBasicGraphSplits() ;

  /*
  public void testToDot() throws Exception {
    int flags = GENERATE_WORD_PARTS | GENERATE_NUMBER_PARTS | CATENATE_ALL |
  SPLIT_ON_CASE_CHANGE | SPLIT_ON_NUMERICS | STEM_ENGLISH_POSSESSIVE |
  PRESERVE_ORIGINAL | CATENATE_WORDS | CATENATE_NUMBERS |
  STEM_ENGLISH_POSSESSIVE; std::wstring text = "PowerSystem2000-5-Shot's";
    WordDelimiterGraphFilter wdf = new WordDelimiterGraphFilter(new
  CannedTokenStream(new Token(text, 0, text.length())),
  DEFAULT_WORD_DELIM_TABLE, flags, null);
    //StringWriter sw = new StringWriter();
    // TokenStreamToDot toDot = new TokenStreamToDot(text, wdf, new
  PrintWriter(sw)); PrintWriter pw = new PrintWriter("/tmp/foo2.dot");
    TokenStreamToDot toDot = new TokenStreamToDot(text, wdf, pw);
    toDot.toDot();
    pw.close();
    //System.out.println("DOT:\n" + sw.toString());
  }
  */

private:
  std::wstring randomWDFText();

public:
  virtual void testInvalidFlag() ;

  virtual void testRandomPaths() ;

  /** Runs normal and slow WDGF and compares results */
private:
  void verify(const std::wstring &text, int flags) ;

public:
  virtual void testOnlyNumbers() ;

  virtual void testNoCatenate() ;

  virtual void testCuriousCase1() ;

  virtual void testCuriousCase2() ;

  virtual void testOriginalPosLength() ;

  virtual void testCuriousCase3() ;

  virtual void testEmptyString() ;

  virtual void testProtectedWords() ;

protected:
  std::shared_ptr<TestWordDelimiterGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestWordDelimiterGraphFilter>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
