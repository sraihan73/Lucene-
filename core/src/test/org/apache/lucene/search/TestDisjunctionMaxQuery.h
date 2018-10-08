#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/TestSimilarity.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test of the DisjunctionMaxQuery.
 *
 */
class TestDisjunctionMaxQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestDisjunctionMaxQuery)

  /** threshold for comparing floats */
public:
  static constexpr float SCORE_COMP_THRESH = 0.0000f;

  /**
   * Similarity to eliminate tf, idf and lengthNorm effects to isolate test
   * case.
   *
   * <p>
   * same as TestRankingSimilarity in TestRanking.zip from
   * http://issues.apache.org/jira/browse/LUCENE-323
   * </p>
   */
private:
  class TestSimilarity : public ClassicSimilarity
  {
    GET_CLASS_NAME(TestSimilarity)

  public:
    TestSimilarity();

    float tf(float freq) override;

    float lengthNorm(int length) override;

    float idf(int64_t docFreq, int64_t docCount) override;

  protected:
    std::shared_ptr<TestSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<TestSimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

public:
  std::shared_ptr<Similarity> sim = std::make_shared<TestSimilarity>();
  std::shared_ptr<Directory> index;
  std::shared_ptr<IndexReader> r;
  std::shared_ptr<IndexSearcher> s;

private:
  static const std::shared_ptr<FieldType> nonAnalyzedType;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestDisjunctionMaxQuery::StaticConstructor staticConstructor;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testSkipToFirsttimeMiss() ;

  virtual void testSkipToFirsttimeHit() ;

  virtual void testSimpleEqualScores1() ;

  virtual void testSimpleEqualScores2() ;

  virtual void testSimpleEqualScores3() ;

  virtual void testSimpleTiebreaker() ;

  virtual void testBooleanRequiredEqualScores() ;

  virtual void testBooleanOptionalNoTiebreaker() ;

  virtual void testBooleanOptionalWithTiebreaker() ;

  virtual void
  testBooleanOptionalWithTiebreakerAndBoost() ;

  // LUCENE-4477 / LUCENE-4401:
  virtual void testBooleanSpanQuery() ;

  virtual void testNegativeScore() ;

  virtual void testRewriteBoolean() ;

  /** macro */
protected:
  virtual std::shared_ptr<Query> tq(const std::wstring &f,
                                    const std::wstring &t);

  /** macro */
  virtual std::shared_ptr<Query> tq(const std::wstring &f,
                                    const std::wstring &t, float b);

  virtual void
  printHits(const std::wstring &test, std::deque<std::shared_ptr<ScoreDoc>> &h,
            std::shared_ptr<IndexSearcher> searcher) ;

protected:
  std::shared_ptr<TestDisjunctionMaxQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestDisjunctionMaxQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
