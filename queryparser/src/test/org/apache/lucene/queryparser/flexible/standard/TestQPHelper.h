#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/StandardQueryParser.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/document/DateTools.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

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
namespace org::apache::lucene::queryparser::flexible::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using DateTools = org::apache::lucene::document::DateTools;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * This test case is a copy of the core Lucene query parser test, it was adapted
 * to use new QueryParserHelper instead of the old query parser.
 *
 * Tests QueryParser.
 */
// TODO: really this should extend QueryParserTestBase too!
class TestQPHelper : public LuceneTestCase
{
  GET_CLASS_NAME(TestQPHelper)

public:
  static std::shared_ptr<Analyzer> qpAnalyzer;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass()
  static void beforeClass();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass()
  static void afterClass();

public:
  class QPTestFilter final : public TokenFilter
  {
    GET_CLASS_NAME(QPTestFilter)
  private:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);

    /**
     * Filter which discards the token 'stop' and which expands the token
     * 'phrase' into 'phrase1 phrase2'
     */
  public:
    QPTestFilter(std::shared_ptr<TokenStream> in_);

  private:
    bool inPhrase = false;
    int savedStart = 0;
    int savedEnd = 0;

  public:
    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<QPTestFilter> shared_from_this()
    {
      return std::static_pointer_cast<QPTestFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  class QPTestAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(QPTestAnalyzer)

    /** Filters MockTokenizer with StopFilter. */
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override final;

  protected:
    std::shared_ptr<QPTestAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<QPTestAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  class QPTestParser : public StandardQueryParser
  {
    GET_CLASS_NAME(QPTestParser)
  public:
    QPTestParser(std::shared_ptr<Analyzer> a);

  private:
    class QPTestParserQueryNodeProcessor : public QueryNodeProcessorImpl
    {
      GET_CLASS_NAME(QPTestParserQueryNodeProcessor)

    protected:
      std::shared_ptr<QueryNode> postProcessNode(
          std::shared_ptr<QueryNode> node)  override;

      std::shared_ptr<QueryNode> preProcessNode(
          std::shared_ptr<QueryNode> node)  override;

      std::deque<std::shared_ptr<QueryNode>>
      setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
          QueryNodeException) override;

    protected:
      std::shared_ptr<QPTestParserQueryNodeProcessor> shared_from_this()
      {
        return std::static_pointer_cast<QPTestParserQueryNodeProcessor>(
            org.apache.lucene.queryparser.flexible.core.processors
                .QueryNodeProcessorImpl::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<QPTestParser> shared_from_this()
    {
      return std::static_pointer_cast<QPTestParser>(
          StandardQueryParser::shared_from_this());
    }
  };

private:
  int originalMaxClauses = 0;

public:
  void setUp()  override;

  virtual std::shared_ptr<StandardQueryParser>
  getParser(std::shared_ptr<Analyzer> a) ;

  virtual std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<Analyzer> a) ;

  virtual std::shared_ptr<Query> getQueryAllowLeadingWildcard(
      const std::wstring &query,
      std::shared_ptr<Analyzer> a) ;

  virtual void
  assertQueryEquals(const std::wstring &query, std::shared_ptr<Analyzer> a,
                    const std::wstring &result) ;

  virtual void
  assertMatchNoDocsQuery(const std::wstring &queryString,
                         std::shared_ptr<Analyzer> a) ;

  virtual void assertMatchNoDocsQuery(std::shared_ptr<Query> query) throw(
      std::runtime_error);

  virtual void assertQueryEqualsAllowLeadingWildcard(
      const std::wstring &query, std::shared_ptr<Analyzer> a,
      const std::wstring &result) ;

  virtual void
  assertQueryEquals(std::shared_ptr<StandardQueryParser> qp,
                    const std::wstring &field, const std::wstring &query,
                    const std::wstring &result) ;

  virtual void assertEscapedQueryEquals(
      const std::wstring &query, std::shared_ptr<Analyzer> a,
      const std::wstring &result) ;

  virtual void assertWildcardQueryEquals(
      const std::wstring &query, const std::wstring &result,
      bool allowLeadingWildcard) ;

  virtual void assertWildcardQueryEquals(
      const std::wstring &query,
      const std::wstring &result) ;

  virtual std::shared_ptr<Query>
  getQueryDOA(const std::wstring &query,
              std::shared_ptr<Analyzer> a) ;

  virtual void
  assertQueryEqualsDOA(const std::wstring &query, std::shared_ptr<Analyzer> a,
                       const std::wstring &result) ;

  virtual void testConstantScoreAutoRewrite() ;

  virtual void testCJK() ;

  // individual CJK chars as terms, like StandardAnalyzer
private:
  class SimpleCJKTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(SimpleCJKTokenizer)
  private:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  public:
    SimpleCJKTokenizer();

    bool incrementToken()  override;

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

  virtual void testCJKBoostedTerm() ;

  virtual void testCJKPhrase() ;

  virtual void testCJKBoostedPhrase() ;

  virtual void testCJKSloppyPhrase() ;

  virtual void testSimple() ;

  virtual void testPunct() ;

  virtual void testGroup() ;

  virtual void testSlop() ;

  virtual void testNumber() ;

  virtual void testLeadingNegation() ;

  virtual void testNegationInParentheses() ;

  virtual void testWildcard() ;

  virtual void testLeadingWildcardType() ;

  virtual void testQPA() ;

  virtual void testRange() ;

  /** for testing DateTools support */
private:
  std::wstring
  getDate(const std::wstring &s,
          DateTools::Resolution resolution) ;

  /** for testing DateTools support */
  std::wstring getDate(Date d, DateTools::Resolution resolution);

  std::wstring escapeDateString(const std::wstring &s);

  std::wstring getLocalizedDate(int year, int month, int day);

public:
  virtual void testDateRange() ;

  virtual void assertDateRangeQueryEquals(
      std::shared_ptr<StandardQueryParser> qp, const std::wstring &field,
      const std::wstring &startDate, const std::wstring &endDate,
      Date endDateInclusive,
      DateTools::Resolution resolution) ;

  virtual void testEscaped() ;

  virtual void testQueryStringEscaping() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore("flexible queryparser shouldn't escape wildcard
  // terms") public void testEscapedWildcard() throws Exception
  virtual void testEscapedWildcard() ;

  virtual void testTabNewlineCarriageReturn() ;

  virtual void testSimpleDAO() ;

  virtual void testBoost() ;

  virtual void assertQueryNodeException(const std::wstring &queryString) throw(
      std::runtime_error);

  virtual void testException() ;

  // Wildcard queries should not be allowed
  virtual void testCustomQueryParserWildcard();

  // Fuzzy queries should not be allowed"
  virtual void testCustomQueryParserFuzzy() ;

  // too many bool clauses, so ParseException is expected
  virtual void testBooleanQuery() ;

  /**
   * This test differs from TestPrecedenceQueryParser
   */
  virtual void testPrecedence() ;

  // Todo: Convert from DateField to DateUtil
  //  public void testLocalDateFormat() throws IOException, QueryNodeException {
  //    Directory ramDir = newDirectory();
  //    IndexWriter iw = new IndexWriter(ramDir, newIndexWriterConfig(new
  //    MockAnalyzer(random, MockTokenizer.WHITESPACE, false))); addDateDoc("a",
  //    2005, 12, 2, 10, 15, 33, iw); addDateDoc("b", 2005, 12, 4, 22, 15, 00,
  //    iw); iw.close(); IndexSearcher is = new IndexSearcher(ramDir, true);
  //    assertHits(1, "[12/1/2005 TO 12/3/2005]", is);
  //    assertHits(2, "[12/1/2005 TO 12/4/2005]", is);
  //    assertHits(1, "[12/3/2005 TO 12/4/2005]", is);
  //    assertHits(1, "{12/1/2005 TO 12/3/2005}", is);
  //    assertHits(1, "{12/1/2005 TO 12/4/2005}", is);
  //    assertHits(0, "{12/3/2005 TO 12/4/2005}", is);
  //    is.close();
  //    ramDir.close();
  //  }
  //
  //  private void addDateDoc(std::wstring content, int year, int month, int day,
  //                          int hour, int minute, int second, IndexWriter iw)
  //                          throws IOException {
  //    Document d = new Document();
  //    d.add(newField("f", content, Field.Store.YES, Field.Index.ANALYZED));
  //    Calendar cal = Calendar.getInstance(Locale.ENGLISH);
  //    cal.set(year, month - 1, day, hour, minute, second);
  //    d.add(newField("date", DateField.dateToString(cal.getTime()),
  //        Field.Store.YES, Field.Index.NOT_ANALYZED));
  //    iw.addDocument(d);
  //  }

  virtual void testStarParsing() ;

  virtual void testRegexps() ;

  virtual void testStopwords() ;

  virtual void testPositionIncrement() ;

  virtual void testMatchAllDocs() ;

private:
  void assertHits(int expected, const std::wstring &query,
                  std::shared_ptr<IndexSearcher> is) throw(IOException,
                                                           QueryNodeException);

public:
  void tearDown()  override;

private:
  class CannedTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(CannedTokenizer)
  private:
    int upto = 0;
    const std::shared_ptr<PositionIncrementAttribute> posIncr =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<CharTermAttribute> term =
        addAttribute(CharTermAttribute::typeid);

  public:
    CannedTokenizer();

    bool incrementToken() override;

    void reset()  override;

  protected:
    std::shared_ptr<CannedTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<CannedTokenizer>(
          org.apache.lucene.analysis.Tokenizer::shared_from_this());
    }
  };

private:
  class CannedAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(CannedAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &ignored) override;

  protected:
    std::shared_ptr<CannedAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<CannedAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMultiPhraseQuery() ;

  virtual void testRegexQueryParsing() ;

protected:
  std::shared_ptr<TestQPHelper> shared_from_this()
  {
    return std::static_pointer_cast<TestQPHelper>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/
