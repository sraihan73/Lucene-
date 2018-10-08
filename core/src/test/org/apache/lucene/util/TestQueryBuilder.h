#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::util
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

class TestQueryBuilder : public LuceneTestCase
{
  GET_CLASS_NAME(TestQueryBuilder)

public:
  virtual void testTerm();

  virtual void testBoolean();

  virtual void testBooleanMust();

  virtual void testMinShouldMatchNone();

  virtual void testMinShouldMatchAll();

  virtual void testMinShouldMatch();

  virtual void testPhraseQueryPositionIncrements() ;

  virtual void testEmpty();

  /** adds synonym of "dog" for "dogs", and synonym of "cavy" for "guinea pig".
   */
public:
  class MockSynonymAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MockSynonymAnalyzer)
  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MockSynonymAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MockSynonymAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /** simple synonyms test */
public:
  virtual void testSynonyms() ;

  /** forms multiphrase query */
  virtual void testSynonymsPhrase() ;

  /** forms graph query */
  virtual void testMultiWordSynonymsPhrase() ;

  /** forms graph query */
  virtual void testMultiWordSynonymsBoolean() ;

  /** forms graph query */
  virtual void testMultiWordPhraseSynonymsBoolean() ;

protected:
  class SimpleCJKTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(SimpleCJKTokenizer)
  private:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  public:
    SimpleCJKTokenizer();

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<SimpleCJKTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCJKTokenizer>(
          org.apache.lucene.analysis.Tokenizer::shared_from_this());
    }
  };

private:
  class SimpleCJKAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(SimpleCJKAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<SimpleCJKAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCJKAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testCJKTerm() ;

  virtual void testCJKPhrase() ;

  virtual void testCJKSloppyPhrase() ;

  /**
   * adds synonym of "國" for "国".
   */
protected:
  class MockCJKSynonymFilter : public TokenFilter
  {
    GET_CLASS_NAME(MockCJKSynonymFilter)
  public:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    bool addSynonym = false;

    MockCJKSynonymFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<MockCJKSynonymFilter> shared_from_this()
    {
      return std::static_pointer_cast<MockCJKSynonymFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  class MockCJKSynonymAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MockCJKSynonymAnalyzer)
  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MockCJKSynonymAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MockCJKSynonymAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /** simple CJK synonym test */
public:
  virtual void testCJKSynonym() ;

  /** synonyms with default OR operator */
  virtual void testCJKSynonymsOR() ;

  /** more complex synonyms with default OR operator */
  virtual void testCJKSynonymsOR2() ;

  /** synonyms with default AND operator */
  virtual void testCJKSynonymsAND() ;

  /** more complex synonyms with default AND operator */
  virtual void testCJKSynonymsAND2() ;

  /** forms multiphrase query */
  virtual void testCJKSynonymsPhrase() ;

  virtual void testNoTermAttribute();

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestQueryBuilder> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestQueryBuilder> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class TokenizerAnonymousInnerClass : public Tokenizer
    {
      GET_CLASS_NAME(TokenizerAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance;

    public:
      TokenizerAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance);

      bool wasReset = false;
      void reset()  override;

      bool incrementToken()  override;

    protected:
      std::shared_ptr<TokenizerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TokenizerAnonymousInnerClass>(
            org.apache.lucene.analysis.Tokenizer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestQueryBuilder> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryBuilder>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
