#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
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
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDoubleValuesSource : public LuceneTestCase
{
  GET_CLASS_NAME(TestDoubleValuesSource)

private:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testSortMissing() ;

  virtual void testSimpleFieldEquivalences() ;

  virtual void testHashCodeAndEquals();

  virtual void testSimpleFieldSortables() ;

  virtual std::shared_ptr<Sort> randomSort() ;

  // Take a Sort, and replace any field sorts with Sortables
  virtual std::shared_ptr<Sort>
  convertSortToSortable(std::shared_ptr<Sort> sort);

  virtual void checkSorts(std::shared_ptr<Query> query,
                          std::shared_ptr<Sort> sort) ;

  static std::deque<std::shared_ptr<Query>> const testQueries;

  virtual void testExplanations() ;

private:
  void
  testExplanations(std::shared_ptr<Query> q,
                   std::shared_ptr<DoubleValuesSource> vs) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestDoubleValuesSource> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Query> q;
    std::shared_ptr<org::apache::lucene::search::DoubleValuesSource> rewritten;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestDoubleValuesSource> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Query> q,
        std::shared_ptr<org::apache::lucene::search::DoubleValuesSource>
            rewritten);

    std::shared_ptr<DoubleValues> v;
    std::shared_ptr<LeafReaderContext> ctx;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testQueryDoubleValuesSource() ;

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDoubleValuesSource> outerInstance;

    std::shared_ptr<org::apache::lucene::search::DoubleValuesSource> vs;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<TestDoubleValuesSource> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DoubleValuesSource> vs);

    std::shared_ptr<DoubleValues> v;
    std::shared_ptr<Scorer> scorer;
    std::shared_ptr<LeafReaderContext> ctx;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDoubleValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<TestDoubleValuesSource>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
