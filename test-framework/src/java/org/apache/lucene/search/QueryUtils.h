#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/index/LeafMetaData.h"
#include  "core/src/java/org/apache/lucene/index/CacheHelper.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static junit.framework.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static junit.framework.Assert.assertFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static junit.framework.Assert.assertTrue;

/**
 * Utility class for sanity-checking queries.
 */
class QueryUtils : public std::enable_shared_from_this<QueryUtils>
{
  GET_CLASS_NAME(QueryUtils)

  /** Check the types of things query objects should be able to do. */
public:
  static void check(std::shared_ptr<Query> q);

  /** check very basic hashCode and equals */
  static void checkHashEquals(std::shared_ptr<Query> q);

private:
  class QueryAnonymousInnerClass : public Query
  {
    GET_CLASS_NAME(QueryAnonymousInnerClass)
  public:
    QueryAnonymousInnerClass();

    std::wstring toString(const std::wstring &field) override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<QueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryAnonymousInnerClass>(
          Query::shared_from_this());
    }
  };

public:
  static void checkEqual(std::shared_ptr<Query> q1, std::shared_ptr<Query> q2);

  static void checkUnequal(std::shared_ptr<Query> q1,
                           std::shared_ptr<Query> q2);

  /** deep check that explanations of a query 'score' correctly */
  static void
  checkExplanations(std::shared_ptr<Query> q,
                    std::shared_ptr<IndexSearcher> s) ;

  /**
   * Various query sanity checks on a searcher, some checks are only done for
   * instanceof IndexSearcher.
   *
   * @see #check(Query)
   * @see #checkFirstSkipTo
   * @see #checkSkipTo
   * @see #checkExplanations
   * @see #checkEqual
   * @see CheckHits#checkMatches(Query, IndexSearcher)
   */
  static void check(std::shared_ptr<Random> random, std::shared_ptr<Query> q1,
                    std::shared_ptr<IndexSearcher> s);
  static void check(std::shared_ptr<Random> random, std::shared_ptr<Query> q1,
                    std::shared_ptr<IndexSearcher> s, bool wrap);

  /**
   * Given an IndexSearcher, returns a new IndexSearcher whose IndexReader
   * is a MultiReader containing the Reader of the original IndexSearcher,
   * as well as several "empty" IndexReaders -- some of which will have
   * deleted documents in them.  This new IndexSearcher should
   * behave exactly the same as the original IndexSearcher.
   * @param s the searcher to wrap
   * @param edge if negative, s will be the first sub; if 0, s will be in the
   * middle, if positive s will be the last sub
   */
  static std::shared_ptr<IndexSearcher>
  wrapUnderlyingReader(std::shared_ptr<Random> random,
                       std::shared_ptr<IndexSearcher> s,
                       int const edge) ;

private:
  static std::shared_ptr<IndexReader> emptyReader(int const maxDoc);

private:
  class LeafReaderAnonymousInnerClass : public LeafReader
  {
    GET_CLASS_NAME(LeafReaderAnonymousInnerClass)
  private:
    int maxDoc = 0;

  public:
    LeafReaderAnonymousInnerClass(int maxDoc);

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    std::shared_ptr<NumericDocValues>
    getNumericDocValues(const std::wstring &field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinaryDocValues(const std::wstring &field)  override;

    std::shared_ptr<SortedDocValues>
    getSortedDocValues(const std::wstring &field)  override;

    std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
        const std::wstring &field)  override;

    std::shared_ptr<SortedSetDocValues> getSortedSetDocValues(
        const std::wstring &field)  override;

    std::shared_ptr<NumericDocValues>
    getNormValues(const std::wstring &field)  override;

    std::shared_ptr<FieldInfos> getFieldInfos() override;

    const std::shared_ptr<Bits> liveDocs =
        std::make_shared<Bits::MatchNoBits>(maxDoc);
    std::shared_ptr<Bits> getLiveDocs() override;

    std::shared_ptr<PointValues>
    getPointValues(const std::wstring &fieldName) override;

    void checkIntegrity()  override;
    std::shared_ptr<Fields>
    getTermVectors(int docID)  override;

    int numDocs() override;

    int maxDoc() override;

    void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
        IOException) override;

  protected:
    void doClose()  override;

  public:
    std::shared_ptr<LeafMetaData> getMetaData() override;

    std::shared_ptr<IndexReader::CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<IndexReader::CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<LeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LeafReaderAnonymousInnerClass>(
          org.apache.lucene.index.LeafReader::shared_from_this());
    }
  };

  /** alternate scorer advance(),advance(),next(),next(),advance(),advance(),
   * etc and ensure a hitcollector receives same docs and scores
   */
public:
  static void checkSkipTo(std::shared_ptr<Query> q,
                          std::shared_ptr<IndexSearcher> s) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::Query> q;
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    std::deque<std::shared_ptr<LeafReaderContext>> readerContextArray;
    int skip_op = 0;
    std::deque<int> order;
    std::deque<int> opidx;
    std::deque<int> lastDoc;
    float maxDiff = 0;
    std::deque<std::shared_ptr<LeafReader>> lastReader;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::Query> q,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        std::deque<std::shared_ptr<LeafReaderContext>> &readerContextArray,
        int skip_op, std::deque<int> &order, std::deque<int> &opidx,
        std::deque<int> &lastDoc, float maxDiff,
        std::deque<std::shared_ptr<LeafReader>> &lastReader);

  private:
    std::shared_ptr<Scorer> sc;
    std::shared_ptr<Scorer> scorer;
    std::shared_ptr<DocIdSetIterator> iterator;
    int leafPtr = 0;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

  /** check that first skip on just created scorers always goes to the right doc
   */
public:
  static void
  checkFirstSkipTo(std::shared_ptr<Query> q,
                   std::shared_ptr<IndexSearcher> s) ;

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    float maxDiff = 0;
    std::deque<int> lastDoc;
    std::deque<std::shared_ptr<LeafReader>> lastReader;
    std::deque<std::shared_ptr<LeafReaderContext>> context;
    std::shared_ptr<org::apache::lucene::search::Query> rewritten;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        float maxDiff, std::deque<int> &lastDoc,
        std::deque<std::shared_ptr<LeafReader>> &lastReader,
        std::deque<std::shared_ptr<LeafReaderContext>> &context,
        std::shared_ptr<org::apache::lucene::search::Query> rewritten);

  private:
    std::shared_ptr<Scorer> scorer;
    int leafPtr = 0;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;
    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          SimpleCollector::shared_from_this());
    }
  };

  /** Check that the scorer and bulk scorer advance consistently. */
public:
  static void checkBulkScorerSkipTo(
      std::shared_ptr<Random> r, std::shared_ptr<Query> query,
      std::shared_ptr<IndexSearcher> searcher) ;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::Scorer> scorer;
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> iterator;
    int min = 0;
    int max = 0;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> iterator,
        int min, int max);

    std::shared_ptr<Scorer> scorer2;
    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

private:
  class LeafCollectorAnonymousInnerClass2
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass2>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<org::apache::lucene::search::Scorer> scorer;

  public:
    LeafCollectorAnonymousInnerClass2(
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
