#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::queryparser::flexible::precedence
{
class PrecedenceQueryParser;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::queryparser::flexible::precedence
{

using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using DateTools = org::apache::lucene::document::DateTools;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * <p>
 * This test case tests {@link PrecedenceQueryParser}.
 * </p>
 * <p>
 * It contains all tests from {@link QueryParserTestBase}
 * with some adjusted to fit the precedence requirement, plus some precedence
 * test cases.
 * </p>
 *
 * @see QueryParserTestBase
 */
// TODO: refactor this to actually extend that class (QueryParserTestBase),
// overriding the tests that it adjusts to fit the precedence requirement, adding
// its extra tests.
class TestPrecedenceQueryParser : public LuceneTestCase
{
  GET_CLASS_NAME(TestPrecedenceQueryParser)

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

    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);

    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);

  public:
    bool incrementToken()  override;

    void reset()  override;

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
    createComponents(const std::wstring &fieldName) override final;

  protected:
    std::shared_ptr<QPTestAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<QPTestAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

private:
  int originalMaxClauses = 0;

public:
  void setUp()  override;

  virtual std::shared_ptr<PrecedenceQueryParser>
  getParser(std::shared_ptr<Analyzer> a) ;

  virtual std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<Analyzer> a) ;

  virtual void
  assertQueryEquals(const std::wstring &query, std::shared_ptr<Analyzer> a,
                    const std::wstring &result) ;

  virtual void
  assertMatchNoDocsQuery(const std::wstring &queryString,
                         std::shared_ptr<Analyzer> a) ;

  virtual void assertMatchNoDocsQuery(std::shared_ptr<Query> query) throw(
      std::runtime_error);

  virtual void assertWildcardQueryEquals(
      const std::wstring &query,
      const std::wstring &result) ;

  virtual std::shared_ptr<Query>
  getQueryDOA(const std::wstring &query,
              std::shared_ptr<Analyzer> a) ;

  virtual void
  assertQueryEqualsDOA(const std::wstring &query, std::shared_ptr<Analyzer> a,
                       const std::wstring &result) ;

  virtual void testSimple() ;

  virtual void testPunct() ;

  virtual void testSlop() ;

  virtual void testNumber() ;

  virtual void testWildcard() ;

  virtual void testQPA() ;

  virtual void testRange() ;

private:
  std::wstring escapeDateString(const std::wstring &s);

public:
  virtual std::wstring getDate(const std::wstring &s) ;

private:
  std::wstring getLocalizedDate(int year, int month, int day,
                                bool extendLastDate);

public:
  virtual void testDateRange() ;

  /** for testing DateTools support */
private:
  std::wstring
  getDate(const std::wstring &s,
          DateTools::Resolution resolution) ;

  /** for testing DateTools support */
  std::wstring getDate(Date d, DateTools::Resolution resolution);

public:
  virtual void
  assertQueryEquals(std::shared_ptr<PrecedenceQueryParser> qp,
                    const std::wstring &field, const std::wstring &query,
                    const std::wstring &result) ;

  virtual void assertDateRangeQueryEquals(
      std::shared_ptr<PrecedenceQueryParser> qp, const std::wstring &field,
      const std::wstring &startDate, const std::wstring &endDate,
      Date endDateInclusive,
      DateTools::Resolution resolution) ;

  virtual void testEscaped() ;

  virtual void testTabNewlineCarriageReturn() ;

  virtual void testSimpleDAO() ;

  virtual void testBoost() ;

  virtual void testException() ;

  // ParseException expected due to too many bool clauses
  virtual void testBooleanQuery() ;

  // LUCENE-792
  virtual void testNOT() ;

  /**
   * This test differs from the original QueryParser, showing how the precedence
   * issue has been corrected.
   */
  virtual void testPrecedence() ;

  void tearDown()  override;

protected:
  std::shared_ptr<TestPrecedenceQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<TestPrecedenceQueryParser>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::precedence
