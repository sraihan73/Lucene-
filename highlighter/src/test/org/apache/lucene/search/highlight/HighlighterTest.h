#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/search/highlight/InvalidTokenOffsetsException.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTerm.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Formatter.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/search/highlight/TextFragment.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/search/highlight/TokenGroup.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Fragmenter.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Highlighter.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedTerm.h"

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
namespace org::apache::lucene::search::highlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;

/**
 * JUnit Test for Highlighter class.
 *
 */
class HighlighterTest : public BaseTokenStreamTestCase, public Formatter
{
  GET_CLASS_NAME(HighlighterTest)

private:
  std::shared_ptr<IndexReader> reader;

public:
  static const std::wstring FIELD_NAME;

private:
  static const std::wstring NUMERIC_FIELD_NAME;
  std::shared_ptr<Query> query;

public:
  std::shared_ptr<Directory> ramDir;
  std::shared_ptr<IndexSearcher> searcher = nullptr;
  int numHighlights = 0;
  std::shared_ptr<MockAnalyzer> analyzer;
  std::shared_ptr<TopDocs> hits;
  std::shared_ptr<FieldType> fieldType; // see doc()

  const std::shared_ptr<FieldType> FIELD_TYPE_TV;

  std::deque<std::wstring> texts = {
      L"Hello this is a piece of text that is very long and contains too much "
      L"preamble and the meat is really here which says kennedy has been shot",
      L"This piece of text refers to Kennedy at the beginning then has a "
      L"longer piece of text that is very long in the middle and finally ends "
      L"with another reference to Kennedy",
      L"JFK has been shot",
      L"John Kennedy has been shot",
      L"This text has a typo in referring to Keneddy",
      L"wordx wordy wordz wordx wordy wordx worda wordb wordy wordc",
      L"y z x y z a b",
      L"lets is a the lets is a the lets is a the lets",
      L"Attribute instances are reused for all tokens of a document. Thus, a "
      L"TokenStream/-Filter needs to update the appropriate Attribute(s) in "
      L"incrementToken(). The consumer, commonly the Lucene indexer, consumes "
      L"the data in the Attributes and then calls incrementToken() again until "
      L"it retuns false, which indicates that the end of the stream was "
      L"reached. This means that in each call of incrementToken() a "
      L"TokenStream/-Filter can safely overwrite the data in the Attribute "
      L"instances. "};

  // Convenience method for succinct tests; doesn't represent "best practice"
private:
  std::shared_ptr<TokenStream> getAnyTokenStream(const std::wstring &fieldName,
                                                 int docId) ;

public:
  virtual void testCustomScoreQueryHighlight() ;

  virtual void testFunctionScoreQuery() ;

  virtual void testQueryScorerHits() ;

  virtual void testHighlightingCommonTermsQuery() ;

  virtual void testHighlightingSynonymQuery() ;

  virtual void
  testHighlightUnknownQueryAfterRewrite() throw(IOException,
                                                InvalidTokenOffsetsException);

private:
  class QueryAnonymousInnerClass : public Query
  {
    GET_CLASS_NAME(QueryAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    QueryAnonymousInnerClass(std::shared_ptr<HighlighterTest> outerInstance);

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring
    BaseTokenStreamTestCase::toString(const std::wstring &field) override;

    virtual int hashCode();

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<QueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryAnonymousInnerClass>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

public:
  virtual void testHighlightingWithDefaultField() ;

  /**
   * This method intended for use with
   * <tt>testHighlightingWithDefaultField()</tt>
   */
private:
  std::wstring
  highlightField(std::shared_ptr<Query> query, const std::wstring &fieldName,
                 const std::wstring &text) throw(IOException,
                                                 InvalidTokenOffsetsException);

public:
  virtual void testSimpleSpanHighlighter() ;

  // LUCENE-2229
  virtual void
  testSimpleSpanHighlighterWithStopWordsStraddlingFragmentBoundaries() throw(
      std::runtime_error);

  // LUCENE-1752
  virtual void testRepeatingTermsInMultBooleans() ;

  virtual void
  testSimpleQueryScorerPhraseHighlighting() ;

  virtual void testSpanRegexQuery() ;

  virtual void testRegexQuery() ;

  virtual void testExternalReader() ;

  virtual void testDimensionalRangeQuery() ;

  virtual void testToParentBlockJoinQuery() ;

  virtual void testToChildBlockJoinQuery() ;

  virtual void
  testSimpleQueryScorerPhraseHighlighting2() ;

  virtual void
  testSimpleQueryScorerPhraseHighlighting3() ;

  virtual void testSimpleSpanFragmenter() ;

  // position sensitive query added after position insensitive query
  virtual void testPosTermStdTerm() ;

  virtual void
  testQueryScorerMultiPhraseQueryHighlighting() ;

  virtual void testQueryScorerMultiPhraseQueryHighlightingWithGap() throw(
      std::runtime_error);

  virtual void testNearSpanSimpleQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testSimpleQueryTermScorerHighlighter() ;

  virtual void testSpanHighlighting() ;

private:
  class TestHighlightRunnerAnonymousInnerClass2 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass2)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass2(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass2>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testNotSpanSimpleQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass3 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass3)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass3(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass3>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsSimpleQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass4 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass4)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass4(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass4>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsConstantScore() ;

private:
  class TestHighlightRunnerAnonymousInnerClass5 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass5)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass5(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass5>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetFuzzyFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass6 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass6)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass6(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass6>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetWildCardFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass7 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass7)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass7(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass7>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetMidWildCardFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass8 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass8)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass8(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass8>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetRangeFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass9 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass9)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass9(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass9> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass9>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testConstantScoreMultiTermQuery() ;

  virtual void testGetBestFragmentsPhrase() ;

private:
  class TestHighlightRunnerAnonymousInnerClass10 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass10)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass10(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass10> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass10>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsQueryScorer() ;

private:
  class TestHighlightRunnerAnonymousInnerClass11 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass11)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass11(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass11> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass11>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testOffByOne() ;

private:
  class TestHighlightRunnerAnonymousInnerClass12 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass12)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass12(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass12> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass12>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsFilteredQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass13 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass13)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass13(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass13> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass13>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void
  testGetBestFragmentsFilteredPhraseQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass14 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass14)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass14(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass14> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass14>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsMultiTerm() ;

private:
  class TestHighlightRunnerAnonymousInnerClass15 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass15)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass15(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass15> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass15>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestFragmentsWithOr() ;

private:
  class TestHighlightRunnerAnonymousInnerClass16 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass16)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass16(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass16> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass16>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetBestSingleFragment() ;

private:
  class TestHighlightRunnerAnonymousInnerClass17 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass17)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass17(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass17> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass17>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testNotRewriteMultiTermQuery() ;

private:
  class WeightedSpanTermExtractorAnonymousInnerClass
      : public WeightedSpanTermExtractor
  {
    GET_CLASS_NAME(WeightedSpanTermExtractorAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

    std::shared_ptr<MultiTermQuery> mtq;

  public:
    WeightedSpanTermExtractorAnonymousInnerClass(
        std::shared_ptr<HighlighterTest> outerInstance,
        std::shared_ptr<MultiTermQuery> mtq);

  protected:
    void
    extract(std::shared_ptr<Query> query, float boost,
            std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
                &terms)  override;

  protected:
    std::shared_ptr<WeightedSpanTermExtractorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          WeightedSpanTermExtractorAnonymousInnerClass>(
          WeightedSpanTermExtractor::shared_from_this());
    }
  };

public:
  virtual void testGetBestSingleFragmentWithWeights() ;

private:
  class TestHighlightRunnerAnonymousInnerClass18 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass18)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass18(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass18> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass18>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

  // tests a "complex" analyzer that produces multiple
  // overlapping tokens
public:
  virtual void testOverlapAnalyzer() ;

private:
  class TestHighlightRunnerAnonymousInnerClass19 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass19)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass19(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass19> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass19>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetSimpleHighlight() ;

private:
  class TestHighlightRunnerAnonymousInnerClass20 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass20)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass20(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass20> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass20>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testGetTextFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass21 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass21)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass21(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass21> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass21>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testMaxSizeHighlight() ;

private:
  class TestHighlightRunnerAnonymousInnerClass22 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass22)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass22(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass22> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass22>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testMaxSizeHighlightTruncates() ;

private:
  class TestHighlightRunnerAnonymousInnerClass23 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass23)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass23(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass23> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass23>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testMaxSizeEndHighlight() ;

private:
  class TestHighlightRunnerAnonymousInnerClass24 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass24)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass24(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass24> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass24>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void
  testHighlighterWithPhraseQuery() throw(IOException,
                                         InvalidTokenOffsetsException);

  virtual void
  testHighlighterWithMultiPhraseQuery() throw(IOException,
                                              InvalidTokenOffsetsException);

private:
  void assertHighlighting(
      std::shared_ptr<Query> query, std::shared_ptr<Formatter> formatter,
      const std::wstring &text, const std::wstring &expected,
      const std::wstring &fieldName) throw(IOException,
                                           InvalidTokenOffsetsException);

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

    std::wstring fieldName;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<HighlighterTest> outerInstance,
                                const std::wstring &fieldName);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testUnRewrittenQuery() ;

private:
  class TestHighlightRunnerAnonymousInnerClass25 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass25)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass25(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass25> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass25>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

public:
  virtual void testNoFragments() ;

private:
  class TestHighlightRunnerAnonymousInnerClass26 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass26)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass26(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass26> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass26>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

  /**
   * Demonstrates creation of an XHTML compliant doc using new encoding
   * facilities.
   */
public:
  virtual void testEncoding() ;

private:
  class ScorerAnonymousInnerClass : public Scorer
  {
    GET_CLASS_NAME(ScorerAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    ScorerAnonymousInnerClass(std::shared_ptr<HighlighterTest> outerInstance);

    void startFragment(std::shared_ptr<TextFragment> newFragment) override;

    float getTokenScore() override;

    float getFragmentScore() override;

    std::shared_ptr<TokenStream>
    init(std::shared_ptr<TokenStream> tokenStream) override;

  protected:
    std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerAnonymousInnerClass>(
          Scorer::shared_from_this());
    }
  };

public:
  virtual void testFieldSpecificHighlighting() ;

private:
  class TestHighlightRunnerAnonymousInnerClass27 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass27)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass27(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass27> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass27>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<TokenStream> getTS2();

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<HighlighterTest> outerInstance);

    std::shared_ptr<Iterator<std::shared_ptr<Token>>> iter;
    std::deque<std::shared_ptr<Token>> lst;

  private:
    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

  public:
    bool incrementToken() override;

    void reset()  override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

  // same token-stream as above, but the bigger token comes first this time
protected:
  virtual std::shared_ptr<TokenStream> getTS2a();

private:
  class TokenStreamAnonymousInnerClass2 : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass2)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TokenStreamAnonymousInnerClass2(
        std::shared_ptr<HighlighterTest> outerInstance);

    std::shared_ptr<Iterator<std::shared_ptr<Token>>> iter;
    std::deque<std::shared_ptr<Token>> lst;

  private:
    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;

  public:
    bool incrementToken() override;

    void reset()  override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass2>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testOverlapAnalyzer2() ;

private:
  class TestHighlightRunnerAnonymousInnerClass28 : public TestHighlightRunner
  {
    GET_CLASS_NAME(TestHighlightRunnerAnonymousInnerClass28)
  private:
    std::shared_ptr<HighlighterTest> outerInstance;

  public:
    TestHighlightRunnerAnonymousInnerClass28(
        std::shared_ptr<HighlighterTest> outerInstance);

    void run()  override;

  protected:
    std::shared_ptr<TestHighlightRunnerAnonymousInnerClass28> shared_from_this()
    {
      return std::static_pointer_cast<TestHighlightRunnerAnonymousInnerClass28>(
          org.apache.lucene.search.highlight.SynonymTokenizer
              .TestHighlightRunner::shared_from_this());
    }
  };

private:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<Analyzer> a;

public:
  virtual void
  testWeightedTermsWithDeletes() throw(IOException,
                                       InvalidTokenOffsetsException);

private:
  void makeIndex() ;

  void deleteDocument() ;

  void searchIndex() ;

  /** We can highlight based on payloads. It's supported both via term vectors
   * and MemoryIndex since Lucene 5. */
public:
  virtual void testPayloadQuery() throw(IOException,
                                        InvalidTokenOffsetsException);

  /*
   *
   * public void testBigramAnalyzer() throws IOException, ParseException {
   * //test to ensure analyzers with none-consecutive start/end offsets //dont
   * double-highlight text //setup index 1 RAMDirectory ramDir = new
   * RAMDirectory(); Analyzer bigramAnalyzer=new CJKAnalyzer(); IndexWriter
   * writer = new IndexWriter(ramDir,bigramAnalyzer , true); Document d = new
   * Document(); Field f = new Field(FIELD_NAME, "java abc def", true, true,
   * true); d.add(f); writer.addDocument(d); writer.close(); IndexReader reader
   * = DirectoryReader.open(ramDir);
   *
   * IndexSearcher searcher=new IndexSearcher(reader); query =
   * QueryParser.parse("abc", FIELD_NAME, bigramAnalyzer);
   * System.out.println("Searching for: " + query.toString(FIELD_NAME)); hits =
   * searcher.search(query);
   *
   * Highlighter highlighter = new Highlighter(this,new
   * QueryFragmentScorer(query));
   *
   * for (int i = 0; i < hits.totalHits; i++) { std::wstring text =
   * searcher.doc2(hits.scoreDocs[i].doc).get(FIELD_NAME); TokenStream
   * tokenStream=bigramAnalyzer.tokenStream(FIELD_NAME,text);
   * std::wstring highlightedText = highlighter.getBestFragment(tokenStream,text);
   * System.out.println(highlightedText); } }
   */

  std::wstring highlightTerm(const std::wstring &originalText,
                             std::shared_ptr<TokenGroup> group) override;

  virtual void doSearching(std::shared_ptr<Query> unReWrittenQuery) throw(
      std::runtime_error);

  virtual void assertExpectedHighlightCount(
      int const maxNumFragmentsRequired,
      int const expectedHighlights) ;

  void setUp()  override;

  void tearDown()  override;

private:
  std::shared_ptr<Document> doc(const std::wstring &name,
                                const std::wstring &value);

  static std::shared_ptr<Token> createToken(const std::wstring &term, int start,
                                            int offset);

protected:
  std::shared_ptr<HighlighterTest> shared_from_this()
  {
    return std::static_pointer_cast<HighlighterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }

private:
  void InitializeInstanceFields();

public:
  HighlighterTest() { InitializeInstanceFields(); }
};

// ===================================================================
// ========== BEGIN TEST SUPPORTING CLASSES
// ========== THESE LOOK LIKE, WITH SOME MORE EFFORT THESE COULD BE
// ========== MADE MORE GENERALLY USEFUL.
// TODO - make synonyms all interchangeable with each other and produce
// a version that does hyponyms - the "is a specialised type of ...."
// so that car = audi, bmw and volkswagen but bmw != audi so different
// behaviour to synonyms
// ===================================================================

class SynonymAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(SynonymAnalyzer)
private:
  std::unordered_map<std::wstring, std::wstring> synonyms;

public:
  SynonymAnalyzer(std::unordered_map<std::wstring, std::wstring> &synonyms);

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.analysis.Analyzer#tokenStream(java.lang.std::wstring,
   *      java.io.Reader)
   */
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &arg0) override;

protected:
  std::shared_ptr<SynonymAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<SynonymAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

/**
 * Expands a token stream with synonyms (TODO - make the synonyms analyzed by
 * choice of analyzer)
 *
 */
class SynonymTokenizer final : public TokenStream
{
  GET_CLASS_NAME(SynonymTokenizer)
private:
  const std::shared_ptr<TokenStream> realStream;
  std::shared_ptr<Token> currentRealToken = nullptr;
  const std::unordered_map<std::wstring, std::wstring> synonyms;
  std::shared_ptr<StringTokenizer> st = nullptr;
  const std::shared_ptr<CharTermAttribute> realTermAtt;
  const std::shared_ptr<PositionIncrementAttribute> realPosIncrAtt;
  const std::shared_ptr<OffsetAttribute> realOffsetAtt;
  const std::shared_ptr<CharTermAttribute> termAtt;
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
  const std::shared_ptr<OffsetAttribute> offsetAtt;

public:
  SynonymTokenizer(std::shared_ptr<TokenStream> realStream,
                   std::unordered_map<std::wstring, std::wstring> &synonyms);

  bool incrementToken()  override;

  void reset()  override;

  void end()  override;

  virtual ~SynonymTokenizer();

public:
  class TestHighlightRunner
      : public std::enable_shared_from_this<TestHighlightRunner>
  {
    GET_CLASS_NAME(TestHighlightRunner)
  public:
    static constexpr int QUERY = 0;
    static constexpr int QUERY_TERM = 1;

    int mode = QUERY;
    std::shared_ptr<Fragmenter> frag = std::make_shared<SimpleFragmenter>(20);

    virtual std::shared_ptr<Highlighter>
    getHighlighter(std::shared_ptr<Query> query, const std::wstring &fieldName,
                   std::shared_ptr<Formatter> formatter);

    virtual std::shared_ptr<Highlighter>
    getHighlighter(std::shared_ptr<Query> query, const std::wstring &fieldName,
                   std::shared_ptr<Formatter> formatter, bool expanMultiTerm);

    virtual std::shared_ptr<Highlighter>
    getHighlighter(std::deque<std::shared_ptr<WeightedTerm>> &weightedTerms,
                   std::shared_ptr<Formatter> formatter);

    virtual void doStandardHighlights(
        std::shared_ptr<Analyzer> analyzer,
        std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<TopDocs> hits,
        std::shared_ptr<Query> query,
        std::shared_ptr<Formatter> formatter) ;

    virtual void doStandardHighlights(std::shared_ptr<Analyzer> analyzer,
                                      std::shared_ptr<IndexSearcher> searcher,
                                      std::shared_ptr<TopDocs> hits,
                                      std::shared_ptr<Query> query,
                                      std::shared_ptr<Formatter> formatter,
                                      bool expandMT) ;

    virtual void run() = 0;

    virtual void start() ;
  };

protected:
  std::shared_ptr<SynonymTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<SynonymTokenizer>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
