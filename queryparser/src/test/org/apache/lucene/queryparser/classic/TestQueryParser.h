#pragma once
#include "exceptionhelper.h"
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

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::classic
{
class ParseException;
}
namespace org::apache::lucene::queryparser::classic
{
class QueryParser;
}
namespace org::apache::lucene::queryparser::flexible::standard
{
class CommonQueryParserConfiguration;
}
namespace org::apache::lucene::document
{
class DateTools;
}
namespace org::apache::lucene::analysis
{
class MockAnalyzer;
}
namespace org::apache::lucene::queryparser::classic
{
class Token;
}
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
namespace org::apache::lucene::queryparser::classic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using QueryParserTestBase =
    org::apache::lucene::queryparser::util::QueryParserTestBase;
using Query = org::apache::lucene::search::Query;

/**
 * Tests QueryParser.
 */
class TestQueryParser : public QueryParserTestBase
{
  GET_CLASS_NAME(TestQueryParser)

protected:
  bool splitOnWhitespace = QueryParser::DEFAULT_SPLIT_ON_WHITESPACE;

private:
  static const std::wstring FIELD;

public:
  class QPTestParser : public QueryParser
  {
    GET_CLASS_NAME(QPTestParser)
  public:
    QPTestParser(const std::wstring &f, std::shared_ptr<Analyzer> a);

  protected:
    std::shared_ptr<Query>
    getFuzzyQuery(const std::wstring &field, const std::wstring &termStr,
                  float minSimilarity)  override;

    std::shared_ptr<Query> getWildcardQuery(
        const std::wstring &field,
        const std::wstring &termStr)  override;

  protected:
    std::shared_ptr<QPTestParser> shared_from_this()
    {
      return std::static_pointer_cast<QPTestParser>(
          QueryParser::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<QueryParser>
  getParser(std::shared_ptr<Analyzer> a) ;

  std::shared_ptr<CommonQueryParserConfiguration> getParserConfig(
      std::shared_ptr<Analyzer> a)  override;

  std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<CommonQueryParserConfiguration>
               cqpC)  override;

  std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<Analyzer> a)  override;

  bool isQueryParserException(std::runtime_error exception) override;

  void setDefaultOperatorOR(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) override;

  void setDefaultOperatorAND(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) override;

  void setAutoGeneratePhraseQueries(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC,
      bool value) override;

  void setDateResolution(std::shared_ptr<CommonQueryParserConfiguration> cqpC,
                         std::shared_ptr<std::wstring> field,
                         Resolution value) override;

  void testDefaultOperator()  override;

  // LUCENE-2002: when we run javacc to regen QueryParser,
  // we also run a replaceregexp step to fix 2 of the public
  // ctors (change them to protected):
  //
  // protected QueryParser(CharStream stream)
  //
  // protected QueryParser(QueryParserTokenManager tm)
  //
  // This test is here as a safety, in case that ant step
  // doesn't work for some reason.
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("rawtype") public void
  // testProtectedCtors() throws Exception
  virtual void testProtectedCtors() ;

  virtual void testFuzzySlopeExtendability() ;

private:
  class QueryParserAnonymousInnerClass : public QueryParser
  {
    GET_CLASS_NAME(QueryParserAnonymousInnerClass)
  private:
    std::shared_ptr<TestQueryParser> outerInstance;

  public:
    QueryParserAnonymousInnerClass(
        std::shared_ptr<TestQueryParser> outerInstance,
        std::shared_ptr<MockAnalyzer> org);

    std::shared_ptr<Query> handleBareFuzzy(
        const std::wstring &qfield, std::shared_ptr<Token> fuzzySlop,
        const std::wstring &termImage)  override;

  protected:
    std::shared_ptr<QueryParserAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryParserAnonymousInnerClass>(
          QueryParser::shared_from_this());
    }
  };

public:
  void testStarParsing()  override;

private:
  class QueryParserAnonymousInnerClass2 : public QueryParser
  {
    GET_CLASS_NAME(QueryParserAnonymousInnerClass2)
  private:
    std::shared_ptr<TestQueryParser> outerInstance;

    std::deque<int> type;

  public:
    QueryParserAnonymousInnerClass2(
        std::shared_ptr<TestQueryParser> outerInstance,
        const std::wstring &FIELD, std::shared_ptr<MockAnalyzer> org,
        std::deque<int> &type);

  protected:
    std::shared_ptr<Query>
    getWildcardQuery(const std::wstring &field,
                     const std::wstring &termStr) override;

    std::shared_ptr<Query> getPrefixQuery(const std::wstring &field,
                                          const std::wstring &termStr) override;

    std::shared_ptr<Query>
    getFieldQuery(const std::wstring &field, const std::wstring &queryText,
                  bool quoted)  override;

  protected:
    std::shared_ptr<QueryParserAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<QueryParserAnonymousInnerClass2>(
          QueryParser::shared_from_this());
    }
  };

  // Wildcard queries should not be allowed
public:
  virtual void testCustomQueryParserWildcard();

  // Fuzzy queries should not be allowed
  virtual void testCustomQueryParserFuzzy() ;

  /** query parser that doesn't expand synonyms when users use double quotes */
private:
  class SmartQueryParser : public QueryParser
  {
    GET_CLASS_NAME(SmartQueryParser)
  private:
    std::shared_ptr<TestQueryParser> outerInstance;

  public:
    std::shared_ptr<Analyzer> morePrecise =
        std::make_shared<QueryParserTestBase::Analyzer2>();

    SmartQueryParser(std::shared_ptr<TestQueryParser> outerInstance);

  protected:
    std::shared_ptr<Query>
    getFieldQuery(const std::wstring &field, const std::wstring &queryText,
                  bool quoted)  override;

  protected:
    std::shared_ptr<SmartQueryParser> shared_from_this()
    {
      return std::static_pointer_cast<SmartQueryParser>(
          QueryParser::shared_from_this());
    }
  };

public:
  void testNewFieldQuery()  override;

  /** simple synonyms test */
  virtual void testSynonyms() ;

  /** forms multiphrase query */
  virtual void testSynonymsPhrase() ;

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

  /** LUCENE-6677: make sure wildcard query respects maxDeterminizedStates. */
  virtual void testWildcardMaxDeterminizedStates() ;

  // TODO: Remove this specialization once the flexible standard parser gets
  // multi-word synonym support
  void testQPA()  override;

  // TODO: Move to QueryParserTestBase once standard flexible parser gets this
  // capability
  virtual void testMultiWordSynonyms() ;

  virtual void testEnableGraphQueries() ;

  // TODO: Move to QueryParserTestBase once standard flexible parser gets this
  // capability
  virtual void testOperatorsAndMultiWordSynonyms() ;

  virtual void testOperatorsAndMultiWordSynonymsSplitOnWhitespace() throw(
      std::runtime_error);

  virtual void testDefaultSplitOnWhitespace() ;

  virtual void testWildcardAlone() ;

  virtual void testWildCardEscapes() ;

  virtual void
  testWildcardDoesNotNormalizeEscapedChars() ;

  virtual void testWildCardQuery() ;

  virtual void testPrefixQuery() ;

  virtual void testRangeQuery() ;

  virtual void testFuzzyQuery() ;

public:
  class FoldingFilter final : public TokenFilter
  {
    GET_CLASS_NAME(FoldingFilter)
  public:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

    FoldingFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<FoldingFilter> shared_from_this()
    {
      return std::static_pointer_cast<FoldingFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  class ASCIIAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(ASCIIAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<TokenStream>
    normalize(const std::wstring &fieldName,
              std::shared_ptr<TokenStream> in_) override;

  protected:
    std::shared_ptr<ASCIIAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<ASCIIAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-4176
public:
  virtual void testByteTerms() ;

  // LUCENE-7533
  virtual void test_splitOnWhitespace_with_autoGeneratePhraseQueries();

private:
  bool isAHit(std::shared_ptr<Query> q, const std::wstring &content,
              std::shared_ptr<Analyzer> analyzer) ;

protected:
  std::shared_ptr<TestQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryParser>(
        org.apache.lucene.queryparser.util
            .QueryParserTestBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::classic
