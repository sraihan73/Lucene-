#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/analysis/CannedBinaryTokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/BinaryToken.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Input.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
using CannedBinaryTokenStream =
    org::apache::lucene::analysis::CannedBinaryTokenStream;
using BinaryToken =
    org::apache::lucene::analysis::CannedBinaryTokenStream::BinaryToken;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Input = org::apache::lucene::search::suggest::Input;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AnalyzingSuggesterTest : public LuceneTestCase
{
  GET_CLASS_NAME(AnalyzingSuggesterTest)

  /** this is basically the WFST test ported to KeywordAnalyzer. so it acts the
   * same */
public:
  virtual void testKeyword() ;

  virtual void testKeywordWithPayloads() ;

  virtual void testRandomRealisticKeys() ;

  // TODO: more tests
  /**
   * basic "standardanalyzer" test with stopword removal
   */
  virtual void testStandard() ;

  virtual void testEmpty() ;

  virtual void testNoSeps() ;

  virtual void testGraphDups() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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
  virtual void testInputPathRequired() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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

  static std::shared_ptr<CannedBinaryTokenStream::BinaryToken>
  token(std::shared_ptr<BytesRef> term);

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
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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
  class TermFreq2 : public std::enable_shared_from_this<TermFreq2>,
                    public Comparable<std::shared_ptr<TermFreq2>>
  {
    GET_CLASS_NAME(TermFreq2)
  public:
    const std::wstring surfaceForm;
    const std::wstring analyzedForm;
    const int64_t weight;
    const std::shared_ptr<BytesRef> payload;

    TermFreq2(const std::wstring &surfaceForm, const std::wstring &analyzedForm,
              int64_t weight, std::shared_ptr<BytesRef> payload);

    int compareTo(std::shared_ptr<TermFreq2> other) override;

    virtual std::wstring toString();
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

private:
  static wchar_t SEP;

public:
  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<TermFreq2>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

    int compare(std::shared_ptr<TermFreq2> left,
                std::shared_ptr<TermFreq2> right);
  };

public:
  virtual void testMaxSurfaceFormsPerAnalyzedForm() ;

  virtual void testQueueExhaustion() ;

  virtual void testExactFirstMissingResult() ;

  virtual void testDupSurfaceFormsMissingResults() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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
  virtual void testDupSurfaceFormsMissingResults2() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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

      int count = 0;

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

  /**
   * Adds 50 random keys, that all analyze to the same thing (dog), with the
   * same cost, and checks that they come back in surface-form order.
   */
public:
  virtual void testTieBreakOnSurfaceForm() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

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
  virtual void test0ByteKeys() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenStreamComponentsAnonymousInnerClass4
        : public TokenStreamComponents
    {
      GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass4)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance;

    public:
      TokenStreamComponentsAnonymousInnerClass4(
          std::shared_ptr<AnalyzerAnonymousInnerClass4> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      int tokenStreamCounter = 0;
      std::deque<std::shared_ptr<TokenStream>> const tokenStreams;

      std::shared_ptr<TokenStream> getTokenStream() override;

    protected:
      void setReader(std::shared_ptr<Reader> reader) override;

    protected:
      std::shared_ptr<TokenStreamComponentsAnonymousInnerClass4>
      shared_from_this()
      {
        return std::static_pointer_cast<
            TokenStreamComponentsAnonymousInnerClass4>(
            TokenStreamComponents::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testDupSurfaceFormsMissingResults3() ;

  virtual void testEndingSpace() ;

  virtual void testTooManyExpansions() ;

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<AnalyzingSuggesterTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<AnalyzingSuggesterTest> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenStreamComponentsAnonymousInnerClass5
        : public TokenStreamComponents
    {
      GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass5)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass5> outerInstance;

    public:
      TokenStreamComponentsAnonymousInnerClass5(
          std::shared_ptr<AnalyzerAnonymousInnerClass5> outerInstance,
          std::shared_ptr<Tokenizer> tokenizer);

      std::shared_ptr<TokenStream> getTokenStream() override;

    protected:
      void setReader(std::shared_ptr<Reader> reader) override;

    protected:
      std::shared_ptr<TokenStreamComponentsAnonymousInnerClass5>
      shared_from_this()
      {
        return std::static_pointer_cast<
            TokenStreamComponentsAnonymousInnerClass5>(
            TokenStreamComponents::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testIllegalLookupArgument() ;

  static std::deque<std::shared_ptr<Input>>
  shuffle(std::deque<Input> &values);

  // TODO: we need BaseSuggesterTestCase?
  virtual void testTooLongSuggestion() ;

private:
  std::shared_ptr<Directory> getDirectory();

protected:
  std::shared_ptr<AnalyzingSuggesterTest> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzingSuggesterTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/analyzing/
