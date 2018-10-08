#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PassageFormatter.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/Passage.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Some tests that highlight wildcard, fuzzy, etc queries.
 */
class TestUnifiedHighlighterMTQ : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterMTQ)

public:
  const std::shared_ptr<FieldType> fieldType;

  std::shared_ptr<BaseDirectoryWrapper> dir;
  std::shared_ptr<Analyzer> indexAnalyzer;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
  // parameters()
  static std::deque<std::deque<std::any>> parameters();

  TestUnifiedHighlighterMTQ(std::shared_ptr<FieldType> fieldType);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
  virtual void doBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
  virtual void doAfter() ;

  virtual void testWildcards() ;

private:
  std::shared_ptr<UnifiedHighlighter>
  randomUnifiedHighlighter(std::shared_ptr<IndexSearcher> searcher,
                           std::shared_ptr<Analyzer> indexAnalyzer);

public:
  virtual void testOnePrefix() ;

  virtual void testOneRegexp() ;

  virtual void testOneFuzzy() ;

  virtual void testRanges() ;

  virtual void testWildcardInBoolean() ;

  virtual void testWildcardInFiltered() ;

  virtual void testWildcardInConstantScore() ;

  virtual void testWildcardInDisjunctionMax() ;

  virtual void testSpanWildcard() ;

  virtual void testSpanOr() ;

  virtual void testSpanNear() ;

  virtual void testSpanNot() ;

  virtual void testSpanPositionCheck() ;

  /**
   * Runs a query with two MTQs and confirms the formatter
   * can tell which query matched which hit.
   */
  virtual void testWhichMTQMatched() ;

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<Analyzer> indexAnalyzer);

  protected:
    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  private:
    class PassageFormatterAnonymousInnerClass : public PassageFormatter
    {
      GET_CLASS_NAME(PassageFormatterAnonymousInnerClass)
    private:
      std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> outerInstance;

    public:
      PassageFormatterAnonymousInnerClass(
          std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> outerInstance);

      std::any format(std::deque<std::shared_ptr<Passage>> &passages,
                      const std::wstring &content) override;

    protected:
      std::shared_ptr<PassageFormatterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PassageFormatterAnonymousInnerClass>(
            PassageFormatter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  //
  //  All tests below were *not* ported from the PostingsHighlighter; they are
  //  new to the U.H.
  //

public:
  virtual void testWithMaxLen() ;

  virtual void testWithMaxLenAndMultipleWildcardMatches() ;

  virtual void testTokenStreamIsClosed() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance);

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

  /**
   * Not empty but nothing analyzes. Ensures we address null term-vectors.
   */
public:
  virtual void testNothingAnalyzes() ;

  virtual void testMultiSegment() ;

  virtual void
  testPositionSensitiveWithWildcardDoesNotHighlight() ;

  virtual void testCustomSpanQueryHighlighting() ;

private:
  class UnifiedHighlighterAnonymousInnerClass2 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass2(
        std::shared_ptr<TestUnifiedHighlighterMTQ> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<Analyzer> indexAnalyzer);

  protected:
    std::deque<std::shared_ptr<Query>>
    preMultiTermQueryRewrite(std::shared_ptr<Query> query) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass2>(
          UnifiedHighlighter::shared_from_this());
    }
  };

private:
  class MyWrapperSpanQuery : public SpanQuery
  {
    GET_CLASS_NAME(MyWrapperSpanQuery)

  private:
    const std::shared_ptr<SpanQuery> originalQuery;

    MyWrapperSpanQuery(std::shared_ptr<SpanQuery> originalQuery);

  public:
    std::wstring getField() override;

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<SpanWeight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<MyWrapperSpanQuery> shared_from_this()
    {
      return std::static_pointer_cast<MyWrapperSpanQuery>(
          org.apache.lucene.search.spans.SpanQuery::shared_from_this());
    }
  };

  // LUCENE-7717 bug, ordering of MTQ AutomatonQuery detection
public:
  virtual void testRussianPrefixQuery() ;

  // LUCENE-7719
  virtual void testMultiByteMTQ() ;

private:
  void highlightAndAssertMatch(std::shared_ptr<IndexSearcher> searcher,
                               std::shared_ptr<UnifiedHighlighter> highlighter,
                               std::shared_ptr<Query> query,
                               const std::wstring &field,
                               const std::wstring &fieldVal) ;

protected:
  std::shared_ptr<TestUnifiedHighlighterMTQ> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterMTQ>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
