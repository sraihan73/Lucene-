#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::queryparser::flexible::standard
{
class CommonQueryParserConfiguration;
}
namespace org::apache::lucene::document
{
class DateTools;
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
namespace org::apache::lucene::queryparser::util
{

using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using DateTools = org::apache::lucene::document::DateTools;
// import org.apache.lucene.queryparser.classic.CharStream;
// import org.apache.lucene.queryparser.classic.ParseException;
// import org.apache.lucene.queryparser.classic.QueryParser;
// import org.apache.lucene.queryparser.classic.QueryParserBase;
// import org.apache.lucene.queryparser.classic.QueryParserTokenManager;
using CommonQueryParserConfiguration = org::apache::lucene::queryparser::
    flexible::standard::CommonQueryParserConfiguration;
using namespace org::apache::lucene::search;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base Test class for QueryParser subclasses
 */
// TODO: it would be better to refactor the parts that are specific really
// to the core QP and subclass/use the parts that are not in the flexible QP
class QueryParserTestBase : public LuceneTestCase
{

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
  public:
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<OffsetAttribute> offsetAtt;

    /**
     * Filter which discards the token 'stop' and which expands the
     * token 'phrase' into 'phrase1 phrase2'
     */
    QPTestFilter(std::shared_ptr<TokenStream> in_);

    bool inPhrase = false;
    int savedStart = 0, savedEnd = 0;

    bool incrementToken()  override;

  protected:
    std::shared_ptr<QPTestFilter> shared_from_this()
    {
      return std::static_pointer_cast<QPTestFilter>(
          TokenFilter::shared_from_this());
    }
  };

public:
  class QPTestAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(QPTestAnalyzer)

    /** Filters MockTokenizer with StopFilter. */
  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<QPTestAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<QPTestAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

private:
  int originalMaxClauses = 0;

  std::wstring defaultField = L"field";

protected:
  virtual std::wstring getDefaultField();

  virtual void setDefaultField(const std::wstring &defaultField);

public:
  void setUp()  override;

  virtual std::shared_ptr<CommonQueryParserConfiguration>
  getParserConfig(std::shared_ptr<Analyzer> a) = 0;

  virtual void setDefaultOperatorOR(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) = 0;

  virtual void setDefaultOperatorAND(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) = 0;

  virtual void setAutoGeneratePhraseQueries(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC, bool value) = 0;

  virtual void
  setDateResolution(std::shared_ptr<CommonQueryParserConfiguration> cqpC,
                    std::shared_ptr<std::wstring> field,
                    DateTools::Resolution value) = 0;

  virtual std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<CommonQueryParserConfiguration> cqpC) = 0;

  virtual std::shared_ptr<Query> getQuery(const std::wstring &query,
                                          std::shared_ptr<Analyzer> a) = 0;

  virtual bool isQueryParserException(std::runtime_error exception) = 0;

  virtual std::shared_ptr<Query>
  getQuery(const std::wstring &query) ;

  virtual void
  assertQueryEquals(const std::wstring &query, std::shared_ptr<Analyzer> a,
                    const std::wstring &result) ;

  virtual void
  assertMatchNoDocsQuery(const std::wstring &queryString,
                         std::shared_ptr<Analyzer> a) ;

  virtual void assertMatchNoDocsQuery(std::shared_ptr<Query> query) throw(
      std::runtime_error);

  virtual void
  assertQueryEquals(std::shared_ptr<CommonQueryParserConfiguration> cqpC,
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

  virtual void testCJK() ;

  // individual CJK chars as terms, like StandardAnalyzer
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
          Tokenizer::shared_from_this());
    }
  };

private:
  class SimpleCJKAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(SimpleCJKAnalyzer)
  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<SimpleCJKAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCJKAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testCJKTerm() ;

  virtual void testCJKBoostedTerm() ;

  virtual void testCJKPhrase() ;

  virtual void testCJKBoostedPhrase() ;

  virtual void testCJKSloppyPhrase() ;

  virtual void testAutoGeneratePhraseQueriesOn() ;

  virtual void testSimple() ;

  virtual void testDefaultOperator() = 0;

  virtual void testOperatorVsWhitespace() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<QueryParserTestBase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<QueryParserTestBase> outerInstance);

    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPunct() ;

  virtual void testSlop() ;

  virtual void testNumber() ;

  virtual void testWildcard() ;

  virtual void testLeadingWildcardType() ;

  virtual void testQPA() ;

  virtual void testRange() ;

  virtual void testRangeWithPhrase() ;

  virtual void testRangeQueryEndpointTO() ;

  virtual void testRangeQueryRequiresTO() ;

private:
  std::wstring escapeDateString(const std::wstring &s);

  /** for testing DateTools support */
  std::wstring
  getDate(const std::wstring &s,
          DateTools::Resolution resolution) ;

  /** for testing DateTools support */
  std::wstring getDate(Date d, DateTools::Resolution resolution);

  std::wstring getLocalizedDate(int year, int month, int day);

public:
  virtual void testDateRange() ;

  virtual void assertDateRangeQueryEquals(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC,
      const std::wstring &field, const std::wstring &startDate,
      const std::wstring &endDate, Date endDateInclusive,
      DateTools::Resolution resolution) ;

  virtual void testEscaped() ;

  virtual void testEscapedVsQuestionMarkAsWildcard() ;

  virtual void testQueryStringEscaping() ;

  virtual void testTabNewlineCarriageReturn() ;

  virtual void testSimpleDAO() ;

  virtual void testBoost() ;

  virtual void assertParseException(const std::wstring &queryString) throw(
      std::runtime_error);

  virtual void
  assertParseException(const std::wstring &queryString,
                       std::shared_ptr<Analyzer> a) ;

  virtual void testException() ;

  virtual void testBooleanQuery() ;

  /**
   * This test differs from TestPrecedenceQueryParser
   */
  virtual void testPrecedence() ;

  // Todo: convert this from DateField to DateUtil
  //  public void testLocalDateFormat() throws IOException, ParseException {
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
  //  private void addDateDoc(std::wstring content, int year, int month,
  //                          int day, int hour, int minute, int second,
  //                          IndexWriter iw) throws IOException {
  //    Document d = new Document();
  //    d.add(newField("f", content, Field.Store.YES, Field.Index.ANALYZED));
  //    Calendar cal = Calendar.getInstance(Locale.ENGLISH);
  //    cal.set(year, month - 1, day, hour, minute, second);
  //    d.add(newField("date", DateField.dateToString(cal.getTime()),
  //    Field.Store.YES, Field.Index.NOT_ANALYZED)); iw.addDocument(d);
  //  }

  virtual void testStarParsing() = 0;

  virtual void testEscapedWildcard() ;

  virtual void testRegexps() ;

  virtual void testStopwords() ;

  virtual void testPositionIncrement() ;

  virtual void testMatchAllDocs() ;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private void assertHits(int
  // expected, std::wstring query, IndexSearcher is) throws Exception
  void assertHits(int expected, const std::wstring &query,
                  std::shared_ptr<IndexSearcher> is) ;

public:
  void tearDown()  override;

  // LUCENE-2002: make sure defaults for StandardAnalyzer's
  // enableStopPositionIncr & QueryParser's enablePosIncr
  // "match"
  virtual void testPositionIncrements() ;

  /** whitespace+lowercase analyzer with synonyms */
protected:
  class Analyzer1 : public Analyzer
  {
    GET_CLASS_NAME(Analyzer1)
  public:
    Analyzer1();
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<Analyzer1> shared_from_this()
    {
      return std::static_pointer_cast<Analyzer1>(Analyzer::shared_from_this());
    }
  };

  /** whitespace+lowercase analyzer without synonyms */
protected:
  class Analyzer2 : public Analyzer
  {
    GET_CLASS_NAME(Analyzer2)
  public:
    Analyzer2();
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<Analyzer2> shared_from_this()
    {
      return std::static_pointer_cast<Analyzer2>(Analyzer::shared_from_this());
    }
  };

public:
  virtual void testNewFieldQuery() = 0;

  /**
   * Mock collation analyzer: indexes terms as "collated" + term
   */
private:
  class MockCollationFilter : public TokenFilter
  {
    GET_CLASS_NAME(MockCollationFilter)
  private:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

  protected:
    MockCollationFilter(std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<MockCollationFilter> shared_from_this()
    {
      return std::static_pointer_cast<MockCollationFilter>(
          TokenFilter::shared_from_this());
    }
  };

private:
  class MockCollationAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MockCollationAnalyzer)
  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<TokenStream>
    normalize(const std::wstring &fieldName,
              std::shared_ptr<TokenStream> in_) override;

  protected:
    std::shared_ptr<MockCollationAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MockCollationAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testCollatedRange() ;

  virtual void testDistanceAsEditsParsing() ;

  virtual void testPhraseQueryToString() ;

  virtual void testParseWildcardAndPhraseQueries() ;

  virtual void testPhraseQueryPositionIncrements() ;

  virtual void testMatchAllQueryParsing() ;

  virtual void testNestedAndClausesFoo() ;

protected:
  std::shared_ptr<QueryParserTestBase> shared_from_this()
  {
    return std::static_pointer_cast<QueryParserTestBase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::util
