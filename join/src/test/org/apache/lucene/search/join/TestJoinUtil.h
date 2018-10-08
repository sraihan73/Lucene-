#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::util
{
class BitSet;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search::join
{
class IndexIterationContext;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search::join
{
class JoinScore;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::join
{
class RandomDoc;
}
namespace org::apache::lucene::index
{
class OrdinalMap;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::search::join
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using namespace org::apache::lucene::search;
using Directory = org::apache::lucene::store::Directory;
using BitSet = org::apache::lucene::util::BitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestJoinUtil : public LuceneTestCase
{
  GET_CLASS_NAME(TestJoinUtil)

public:
  virtual void testSimple() ;

  virtual void testSimpleOrdinalsJoin() ;

  virtual void testOrdinalsJoinExplainNoMatches() ;

  virtual void testRandomOrdinalsJoin() ;

  virtual void testMinMaxScore() ;

  // FunctionQuery would be helpful, but join module doesn't depend on queries
  // module.
  static std::shared_ptr<Query>
  numericDocValuesScoreQuery(const std::wstring &field);

private:
  class QueryAnonymousInnerClass : public Query
  {
    GET_CLASS_NAME(QueryAnonymousInnerClass)
  private:
    std::wstring field;

  public:
    QueryAnonymousInnerClass(const std::wstring &field);

  private:
    const std::shared_ptr<Query> fieldQuery =
        std::make_shared<DocValuesFieldExistsQuery>(field);

  public:
    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class WeightAnonymousInnerClass : public Weight
    {
      GET_CLASS_NAME(WeightAnonymousInnerClass)
    private:
      std::shared_ptr<QueryAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::search::Weight> fieldWeight;

    public:
      WeightAnonymousInnerClass(
          std::shared_ptr<QueryAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::search::Weight> fieldWeight);

      void
      extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

      std::shared_ptr<Explanation>
      explain(std::shared_ptr<LeafReaderContext> context,
              int doc)  override;

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class FilterScorerAnonymousInnerClass : public FilterScorer
      {
        GET_CLASS_NAME(FilterScorerAnonymousInnerClass)
      private:
        std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<NumericDocValues> price;

      public:
        FilterScorerAnonymousInnerClass(
            std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
            std::shared_ptr<org::apache::lucene::search::Scorer> fieldScorer,
            std::shared_ptr<NumericDocValues> price);

        float score()  override;

      protected:
        std::shared_ptr<FilterScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterScorerAnonymousInnerClass>(
              FilterScorer::shared_from_this());
        }
      };

    public:
      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<WeightAnonymousInnerClass>(
            Weight::shared_from_this());
      }
    };

  public:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring Term::toString(const std::wstring &field) override;

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
  virtual void testMinMaxDocs() ;

  virtual void testRewrite() ;

  // TermsWithScoreCollector.MV.Avg forgets to grow beyond
  // TermsWithScoreCollector.INITIAL_ARRAY_SIZE
  virtual void testOverflowTermsWithScoreCollector() ;

  virtual void
  testOverflowTermsWithScoreCollectorRandom() ;

  virtual void test300spartans(bool multipleValues,
                               ScoreMode scoreMode) ;

  /** LUCENE-5487: verify a join query inside a SHOULD BQ
   *  will still use the join query's optimized BulkScorers */
  virtual void testInsideBooleanQuery() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestJoinUtil> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestJoinUtil> outerInstance);

    bool sawFive = false;
    void collect(int docID) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testSimpleWithScoring() ;

  virtual void testEquals() ;

  virtual void testEquals_globalOrdinalsJoin() ;

  virtual void testEquals_numericJoin() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Slow public void testSingleValueRandomJoin() throws
  // Exception
  virtual void testSingleValueRandomJoin() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Slow public void testMultiValueRandomJoin() throws
  // Exception
  virtual void testMultiValueRandomJoin() ;

private:
  void
  executeRandomJoin(bool multipleValuesPerDocument, int maxIndexIter,
                    int maxSearchIter,
                    int numberOfDocumentsToIndex) ;

  void
  assertBitSet(std::shared_ptr<BitSet> expectedResult,
               std::shared_ptr<BitSet> actualResult,
               std::shared_ptr<IndexSearcher> indexSearcher) ;

  void assertTopDocs(std::shared_ptr<TopDocs> expectedTopDocs,
                     std::shared_ptr<TopDocs> actualTopDocs,
                     ScoreMode scoreMode,
                     std::shared_ptr<IndexSearcher> indexSearcher,
                     std::shared_ptr<Query> joinQuery) ;

  std::shared_ptr<IndexIterationContext>
  createContext(int nDocs, bool multipleValuesPerDocument,
                bool globalOrdinalJoin) ;

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestJoinUtil> outerInstance;

    std::shared_ptr<
        org::apache::lucene::search::join::TestJoinUtil::IndexIterationContext>
        context;
    std::wstring fromField;
    std::unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
        joinValueToJoinScores;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<TestJoinUtil> outerInstance,
        std::shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                            IndexIterationContext>
            context,
        const std::wstring &fromField,
        std::unordered_map<std::shared_ptr<BytesRef>,
                           std::shared_ptr<JoinScore>> &joinValueToJoinScores);

  private:
    std::shared_ptr<Scorer> scorer;
    std::shared_ptr<SortedSetDocValues> docTermOrds;

  public:
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass3 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestJoinUtil> outerInstance;

    std::shared_ptr<
        org::apache::lucene::search::join::TestJoinUtil::IndexIterationContext>
        context;
    std::wstring fromField;
    std::unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
        joinValueToJoinScores;

  public:
    SimpleCollectorAnonymousInnerClass3(
        std::shared_ptr<TestJoinUtil> outerInstance,
        std::shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                            IndexIterationContext>
            context,
        const std::wstring &fromField,
        std::unordered_map<std::shared_ptr<BytesRef>,
                           std::shared_ptr<JoinScore>> &joinValueToJoinScores);

  private:
    std::shared_ptr<Scorer> scorer;
    std::shared_ptr<BinaryDocValues> terms;

  public:
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass3>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass4 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestJoinUtil> outerInstance;

    std::shared_ptr<
        org::apache::lucene::search::join::TestJoinUtil::IndexIterationContext>
        context;
    std::wstring toField;
    std::unordered_map<std::shared_ptr<BytesRef>, std::shared_ptr<JoinScore>>
        joinValueToJoinScores;
    std::unordered_map<int, std::shared_ptr<JoinScore>> docToJoinScore;

  public:
    SimpleCollectorAnonymousInnerClass4(
        std::shared_ptr<TestJoinUtil> outerInstance,
        std::shared_ptr<org::apache::lucene::search::join::TestJoinUtil::
                            IndexIterationContext>
            context,
        const std::wstring &toField,
        std::unordered_map<std::shared_ptr<BytesRef>,
                           std::shared_ptr<JoinScore>> &joinValueToJoinScores,
        std::unordered_map<int, std::shared_ptr<JoinScore>> &docToJoinScore);

  private:
    std::shared_ptr<BinaryDocValues> terms;
    int docBase = 0;

  public:
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass4>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  void addLinkFields(std::shared_ptr<Random> random,
                     std::shared_ptr<Document> document,
                     const std::wstring &fieldName,
                     const std::wstring &linkValue,
                     bool multipleValuesPerDocument, bool globalOrdinalJoin);

  std::shared_ptr<TopDocs>
  createExpectedTopDocs(const std::wstring &queryValue, bool const from,
                        ScoreMode const scoreMode,
                        std::shared_ptr<IndexIterationContext> context);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<
            std::unordered_map::Entry<int, std::shared_ptr<JoinScore>>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestJoinUtil> outerInstance;

    org::apache::lucene::search::join::ScoreMode scoreMode =
        static_cast<org::apache::lucene::search::join::ScoreMode>(0);

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestJoinUtil> outerInstance,
        org::apache::lucene::search::join::ScoreMode scoreMode);

    int
    compare(std::unordered_map::Entry<int, std::shared_ptr<JoinScore>> hit1,
            std::unordered_map::Entry<int, std::shared_ptr<JoinScore>> hit2);
  };

private:
  std::shared_ptr<BitSet> createExpectedResult(
      const std::wstring &queryValue, bool from,
      std::shared_ptr<IndexReader> topLevelReader,
      std::shared_ptr<IndexIterationContext> context) ;

private:
  class IndexIterationContext
      : public std::enable_shared_from_this<IndexIterationContext>
  {
    GET_CLASS_NAME(IndexIterationContext)

  public:
    std::deque<std::wstring> randomUniqueValues;
    std::deque<bool> randomFrom;
    std::unordered_map<std::wstring, std::deque<std::shared_ptr<RandomDoc>>>
        fromDocuments =
            std::unordered_map<std::wstring,
                               std::deque<std::shared_ptr<RandomDoc>>>();
    std::unordered_map<std::wstring, std::deque<std::shared_ptr<RandomDoc>>>
        toDocuments =
            std::unordered_map<std::wstring,
                               std::deque<std::shared_ptr<RandomDoc>>>();
    std::unordered_map<std::wstring, std::deque<std::shared_ptr<RandomDoc>>>
        randomValueFromDocs =
            std::unordered_map<std::wstring,
                               std::deque<std::shared_ptr<RandomDoc>>>();
    std::unordered_map<std::wstring, std::deque<std::shared_ptr<RandomDoc>>>
        randomValueToDocs =
            std::unordered_map<std::wstring,
                               std::deque<std::shared_ptr<RandomDoc>>>();

    std::unordered_map<std::wstring,
                       std::unordered_map<int, std::shared_ptr<JoinScore>>>
        fromHitsToJoinScore = std::unordered_map<
            std::wstring,
            std::unordered_map<int, std::shared_ptr<JoinScore>>>();
    std::unordered_map<std::wstring,
                       std::unordered_map<int, std::shared_ptr<JoinScore>>>
        toHitsToJoinScore = std::unordered_map<
            std::wstring,
            std::unordered_map<int, std::shared_ptr<JoinScore>>>();

    std::shared_ptr<OrdinalMap> ordinalMap;

    std::shared_ptr<Directory> dir;
    std::shared_ptr<IndexSearcher> searcher;

    virtual ~IndexIterationContext();
  };

private:
  class RandomDoc : public std::enable_shared_from_this<RandomDoc>
  {
    GET_CLASS_NAME(RandomDoc)

  public:
    const std::wstring id;
    const std::deque<std::wstring> linkValues;
    const std::wstring value;
    const bool from;

  private:
    RandomDoc(const std::wstring &id, int numberOfLinkValues,
              const std::wstring &value, bool from);
  };

private:
  class JoinScore : public std::enable_shared_from_this<JoinScore>
  {
    GET_CLASS_NAME(JoinScore)

  public:
    float minScore = std::numeric_limits<float>::infinity();
    float maxScore = -std::numeric_limits<float>::infinity();
    float total = 0;
    int count = 0;

    virtual void addScore(float score);

    virtual float score(ScoreMode mode);
  };

private:
  class BitSetCollector : public SimpleCollector
  {
    GET_CLASS_NAME(BitSetCollector)

  private:
    const std::shared_ptr<BitSet> bitSet;
    int docBase = 0;

    BitSetCollector(std::shared_ptr<BitSet> bitSet);

  public:
    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<BitSetCollector> shared_from_this()
    {
      return std::static_pointer_cast<BitSetCollector>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestJoinUtil> shared_from_this()
  {
    return std::static_pointer_cast<TestJoinUtil>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
