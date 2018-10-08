#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
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
namespace org::apache::lucene::analysis
{
class Tokenizer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis
{
class Token;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::search::suggest
{
class Lookup;
}
namespace org::apache::lucene::search::suggest
{
class LookupResult;
}
namespace org::apache::lucene::search::suggest
{
class Input;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Input = org::apache::lucene::search::suggest::Input;
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using Directory = org::apache::lucene::store::Directory;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class FuzzySuggesterTest : public LuceneTestCase
{
  GET_CLASS_NAME(FuzzySuggesterTest)

public:
  virtual void testRandomEdits() ;

  virtual void testNonLatinRandomEdits() ;

  /** this is basically the WFST test ported to KeywordAnalyzer. so it acts the
   * same */
  virtual void testKeyword() ;

  /**
   * basic "standardanalyzer" test with stopword removal
   */
  virtual void testStandard() ;

  virtual void testNoSeps() ;

  virtual void testGraphDups() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<FuzzySuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<FuzzySuggesterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenStreamComponentsAnonymousInnerClass
        : public TokenStreamComponents
    {
      GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance;

    public:
      TokenStreamComponentsAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      int tokenStreamCounter = 0;
      std::deque<std::shared_ptr<TokenStream>> const tokenStreams;

      std::shared_ptr<TokenStream> getTokenStream() override;

    protected:
      void setReader(std::shared_ptr<Reader> reader) override;

    protected:
      std::shared_ptr<TokenStreamComponentsAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            TokenStreamComponentsAnonymousInnerClass>(
            TokenStreamComponents::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmpty() ;

  virtual void testInputPathRequired() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<FuzzySuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<FuzzySuggesterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenStreamComponentsAnonymousInnerClass2
        : public TokenStreamComponents
    {
      GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass2)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance;

    public:
      TokenStreamComponentsAnonymousInnerClass2(
          std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      int tokenStreamCounter = 0;
      std::deque<std::shared_ptr<TokenStream>> const tokenStreams;

      std::shared_ptr<TokenStream> getTokenStream() override;

    protected:
      void setReader(std::shared_ptr<Reader> reader) override;

    protected:
      std::shared_ptr<TokenStreamComponentsAnonymousInnerClass2>
      shared_from_this()
      {
        return std::static_pointer_cast<
            TokenStreamComponentsAnonymousInnerClass2>(
            TokenStreamComponents::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength);

  /*
  private void printTokens(final Analyzer analyzer, std::wstring input) throws
  IOException { System.out.println("Tokens for " + input); TokenStream ts =
  analyzer.tokenStream("", new StringReader(input)); ts.reset(); final
  TermToBytesRefAttribute termBytesAtt =
  ts.addAttribute(TermToBytesRefAttribute.class); final
  PositionIncrementAttribute posIncAtt =
  ts.addAttribute(PositionIncrementAttribute.class); final
  PositionLengthAttribute posLengthAtt =
  ts.addAttribute(PositionLengthAttribute.class);

    while(ts.incrementToken()) {
      termBytesAtt.fillBytesRef();
      System.out.println(std::wstring.format("%s,%s,%s",
  termBytesAtt.getBytesRef().utf8ToString(), posIncAtt.getPositionIncrement(),
  posLengthAtt.getPositionLength()));
    }
    ts.end();
    ts.close();
  }
  */

  std::shared_ptr<Analyzer> getUnusualAnalyzer();

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<FuzzySuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<FuzzySuggesterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenStreamComponentsAnonymousInnerClass3
        : public TokenStreamComponents
    {
      GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass3)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass3> outerInstance;

    public:
      TokenStreamComponentsAnonymousInnerClass3(
          std::shared_ptr<AnalyzerAnonymousInnerClass3> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      int count = 0;

      std::shared_ptr<TokenStream> getTokenStream() override;

    protected:
      void setReader(std::shared_ptr<Reader> reader) override;

    protected:
      std::shared_ptr<TokenStreamComponentsAnonymousInnerClass3>
      shared_from_this()
      {
        return std::static_pointer_cast<
            TokenStreamComponentsAnonymousInnerClass3>(
            TokenStreamComponents::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testExactFirst() ;

  virtual void testNonExactFirst() ;

  // Holds surface form separately:
private:
  class TermFreqPayload2
      : public std::enable_shared_from_this<TermFreqPayload2>,
        public Comparable<std::shared_ptr<TermFreqPayload2>>
  {
    GET_CLASS_NAME(TermFreqPayload2)
  public:
    const std::wstring surfaceForm;
    const std::wstring analyzedForm;
    const int64_t weight;

    TermFreqPayload2(const std::wstring &surfaceForm,
                     const std::wstring &analyzedForm, int64_t weight);

    int compareTo(std::shared_ptr<TermFreqPayload2> other) override;
  };

public:
  static bool isStopChar(wchar_t ch, int numStopChars);

  // Like StopFilter:
private:
  class TokenEater : public TokenFilter
  {
    GET_CLASS_NAME(TokenEater)
  private:
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const int numStopChars;
    const bool preserveHoles;
    bool first = false;

  public:
    TokenEater(bool preserveHoles, std::shared_ptr<TokenStream> in_,
               int numStopChars);

    void reset()  override;

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<TokenEater> shared_from_this()
    {
      return std::static_pointer_cast<TokenEater>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

private:
  class MockTokenEatingAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MockTokenEatingAnalyzer)
  private:
    int numStopChars = 0;
    bool preserveHoles = false;

  public:
    MockTokenEatingAnalyzer(int numStopChars, bool preserveHoles);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MockTokenEatingAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MockTokenEatingAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<FuzzySuggesterTest> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<FuzzySuggesterTest> outerInstance);

    int compare(std::shared_ptr<LookupResult> left,
                std::shared_ptr<LookupResult> right);
  };

public:
  virtual void testMaxSurfaceFormsPerAnalyzedForm() ;

  virtual void testEditSeps() ;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private static std::wstring
  // addRandomEdit(std::wstring string, int prefixLength)
  static std::wstring addRandomEdit(const std::wstring &string,
                                    int prefixLength);

  std::wstring randomSimpleString(int maxLen);

public:
  virtual void testRandom2() ;

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<std::shared_ptr<Input>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<FuzzySuggesterTest> outerInstance;

  public:
    ComparatorAnonymousInnerClass2(
        std::shared_ptr<FuzzySuggesterTest> outerInstance);

    int compare(std::shared_ptr<Input> a, std::shared_ptr<Input> b);
  };

private:
  std::deque<std::shared_ptr<LookupResult>>
  slowFuzzyMatch(int prefixLen, int maxEdits, bool allowTransposition,
                 std::deque<std::shared_ptr<Input>> &answers,
                 const std::wstring &frag);

private:
  class CharSequenceComparator
      : public std::enable_shared_from_this<CharSequenceComparator>,
        public Comparator<std::shared_ptr<std::wstring>>
  {
    GET_CLASS_NAME(CharSequenceComparator)

  public:
    int compare(std::shared_ptr<std::wstring> o1,
                std::shared_ptr<std::wstring> o2) override;
  };

private:
  static const std::shared_ptr<Comparator<std::shared_ptr<std::wstring>>>
      CHARSEQUENCE_COMPARATOR;

public:
  class CompareByCostThenAlpha
      : public std::enable_shared_from_this<CompareByCostThenAlpha>,
        public Comparator<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(CompareByCostThenAlpha)
  public:
    int compare(std::shared_ptr<LookupResult> a,
                std::shared_ptr<LookupResult> b) override;
  };

  // NOTE: copied from
  // modules/suggest/src/java/org/apache/lucene/search/spell/LuceneLevenshteinDistance.java
  // and tweaked to return the edit distance not the float
  // lucene measure

  /* Finds unicode (code point) Levenstein (edit) distance
   * between two strings, including transpositions. */
public:
  virtual int getDistance(const std::wstring &target, const std::wstring &other,
                          bool allowTransposition);

private:
  static std::shared_ptr<IntsRef> toIntsRef(const std::wstring &s);

  std::shared_ptr<Directory> getDirectory();

protected:
  std::shared_ptr<FuzzySuggesterTest> shared_from_this()
  {
    return std::static_pointer_cast<FuzzySuggesterTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
