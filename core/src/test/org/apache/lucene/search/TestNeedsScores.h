#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestNeedsScores : public LuceneTestCase
{
  GET_CLASS_NAME(TestNeedsScores)
public:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

  void setUp()  override;

  void tearDown()  override;

  /** prohibited clauses in booleanquery don't need scoring */
  virtual void testProhibitedClause() ;

  /** nested inside constant score query */
  virtual void testConstantScoreQuery() ;

  /** when not sorting by score */
  virtual void testSortByField() ;

  /** when sorting by score */
  virtual void testSortByScore() ;

  /**
   * Wraps a query, checking that the needsScores param
   * passed to Weight.scorer is the expected value.
   */
public:
  class AssertNeedsScores : public Query
  {
    GET_CLASS_NAME(AssertNeedsScores)
  public:
    const std::shared_ptr<Query> in_;
    const bool value;

    AssertNeedsScores(std::shared_ptr<Query> in_, bool value);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class FilterWeightAnonymousInnerClass : public FilterWeight
    {
      GET_CLASS_NAME(FilterWeightAnonymousInnerClass)
    private:
      std::shared_ptr<AssertNeedsScores> outerInstance;

      bool needsScores = false;
      std::shared_ptr<org::apache::lucene::search::Weight> w;

    public:
      FilterWeightAnonymousInnerClass(
          std::shared_ptr<AssertNeedsScores> outerInstance, bool needsScores,
          std::shared_ptr<org::apache::lucene::search::Weight> w);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    protected:
      std::shared_ptr<FilterWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterWeightAnonymousInnerClass>(
            FilterWeight::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    virtual int hashCode();

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<AssertNeedsScores> other);

  public:
    std::wstring toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertNeedsScores> shared_from_this()
    {
      return std::static_pointer_cast<AssertNeedsScores>(
          Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestNeedsScores> shared_from_this()
  {
    return std::static_pointer_cast<TestNeedsScores>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
