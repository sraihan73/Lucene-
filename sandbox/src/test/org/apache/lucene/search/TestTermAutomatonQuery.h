#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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
namespace org::apache::lucene::search
{

using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTermAutomatonQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermAutomatonQuery)
  // "comes * sun"
public:
  virtual void testBasic1() ;

  // "comes * (sun|moon)"
  virtual void testBasicSynonym() ;

  // "comes sun" or "comes * sun"
  virtual void testBasicSlop() ;

  // Verify posLength is "respected" at query time: index "speedy wifi
  // network", search on "fast wi fi network" using (simulated!)
  // query-time syn filter to add "wifi" over "wi fi" with posLength=2.
  // To make this real we need a version of TS2A that operates on whole
  // terms, not characters.
  virtual void testPosLengthAtQueryTimeMock() ;

  virtual void testPosLengthAtQueryTimeTrueish() ;

  virtual void testSegsMissingTerms() ;

  virtual void testInvalidLeadWithAny() ;

  virtual void testInvalidTrailWithAny() ;

  virtual void testAnyFromTokenStream() ;

private:
  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength);

private:
  class RandomSynonymFilter : public TokenFilter
  {
    GET_CLASS_NAME(RandomSynonymFilter)
  private:
    bool synNext = false;
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  public:
    RandomSynonymFilter(std::shared_ptr<TokenFilter> in_);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<RandomSynonymFilter> shared_from_this()
    {
      return std::static_pointer_cast<RandomSynonymFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testRandom() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestTermAutomatonQuery> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestTermAutomatonQuery> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  std::shared_ptr<Set<std::wstring>>
  toDocIDs(std::shared_ptr<IndexSearcher> s,
           std::shared_ptr<TopDocs> hits) ;

private:
  class RandomQuery : public Query
  {
    GET_CLASS_NAME(RandomQuery)
  private:
    const int64_t seed;
    float density = 0;

    // density should be 0.0 ... 1.0
  public:
    RandomQuery(int64_t seed, float density);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<RandomQuery> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<RandomQuery> outerInstance, float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<RandomQuery> other);

  public:
    virtual int hashCode();

  protected:
    std::shared_ptr<RandomQuery> shared_from_this()
    {
      return std::static_pointer_cast<RandomQuery>(Query::shared_from_this());
    }
  };

  /** See if we can create a TAQ with cycles */
public:
  virtual void testWithCycles1() ;

  /** See if we can create a TAQ with cycles */
  virtual void testWithCycles2() ;

  virtual void testTermDoesNotExist() ;

  virtual void testOneTermDoesNotExist() ;

  virtual void testEmptyString() ;

  virtual void testRewriteNoMatch() ;

  virtual void testRewriteTerm() ;

  virtual void testRewriteSimplePhrase() ;

  virtual void testRewritePhraseWithAny() ;

  virtual void testRewriteSimpleMultiPhrase() ;

  virtual void testRewriteMultiPhraseWithAny() ;

protected:
  std::shared_ptr<TestTermAutomatonQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestTermAutomatonQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
