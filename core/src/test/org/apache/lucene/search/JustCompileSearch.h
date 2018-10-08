#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/LeafFieldComparator.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
namespace org::apache::lucene::util
{
template <typename T>
class PriorityQueue;
}
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Holds all implementations of classes in the o.a.l.search package as a
 * back-compatibility test. It does not run any tests per-se, however if
 * someone adds a method to an interface or abstract method to an abstract
 * class, one of the implementations here will fail to compile and so we know
 * back-compat policy was violated.
 */
class JustCompileSearch final
    : public std::enable_shared_from_this<JustCompileSearch>
{
  GET_CLASS_NAME(JustCompileSearch)

private:
  static const std::wstring UNSUPPORTED_MSG;

public:
  class JustCompileCollector final : public SimpleCollector
  {
    GET_CLASS_NAME(JustCompileCollector)

  public:
    void collect(int doc) override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<JustCompileCollector> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileCollector>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  class JustCompileDocIdSet final : public DocIdSet
  {
    GET_CLASS_NAME(JustCompileDocIdSet)

  public:
    std::shared_ptr<DocIdSetIterator> iterator() override;

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<JustCompileDocIdSet> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileDocIdSet>(
          DocIdSet::shared_from_this());
    }
  };

public:
  class JustCompileDocIdSetIterator final : public DocIdSetIterator
  {
    GET_CLASS_NAME(JustCompileDocIdSetIterator)

  public:
    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    int64_t cost() override;

  protected:
    std::shared_ptr<JustCompileDocIdSetIterator> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileDocIdSetIterator>(
          DocIdSetIterator::shared_from_this());
    }
  };

public:
  class JustCompileFieldComparator final : public FieldComparator<std::any>
  {
    GET_CLASS_NAME(JustCompileFieldComparator)

  public:
    void setTopValue(std::any value) override;

    std::any value(int slot) override;

    std::shared_ptr<LeafFieldComparator> getLeafComparator(
        std::shared_ptr<LeafReaderContext> context)  override;

    int compare(int slot1, int slot2) override;

  protected:
    std::shared_ptr<JustCompileFieldComparator> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileFieldComparator>(
          FieldComparator<Object>::shared_from_this());
    }
  };

public:
  class JustCompileFieldComparatorSource final : public FieldComparatorSource
  {
    GET_CLASS_NAME(JustCompileFieldComparatorSource)

  public:
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: @Override public FieldComparator<?> newComparator(std::wstring
    // fieldname, int numHits, int sortPos, bool reversed)
    std::shared_ptr < FieldComparator <
        ? >> newComparator(const std::wstring &fieldname, int numHits,
                           int sortPos, bool reversed) override;

  protected:
    std::shared_ptr<JustCompileFieldComparatorSource> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileFieldComparatorSource>(
          FieldComparatorSource::shared_from_this());
    }
  };

public:
  class JustCompileFilteredDocIdSetIterator final
      : public FilteredDocIdSetIterator
  {
    GET_CLASS_NAME(JustCompileFilteredDocIdSetIterator)

  public:
    JustCompileFilteredDocIdSetIterator(
        std::shared_ptr<DocIdSetIterator> innerIter);

  protected:
    bool match(int doc) override;

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<JustCompileFilteredDocIdSetIterator> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileFilteredDocIdSetIterator>(
          FilteredDocIdSetIterator::shared_from_this());
    }
  };

public:
  class JustCompileQuery final : public Query
  {
    GET_CLASS_NAME(JustCompileQuery)

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any obj) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<JustCompileQuery> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileQuery>(
          Query::shared_from_this());
    }
  };

public:
  class JustCompileScorer final : public Scorer
  {
    GET_CLASS_NAME(JustCompileScorer)

  protected:
    JustCompileScorer(std::shared_ptr<Weight> weight);

  public:
    float score() override;

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  protected:
    std::shared_ptr<JustCompileScorer> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileScorer>(
          Scorer::shared_from_this());
    }
  };

public:
  class JustCompileSimilarity final : public Similarity
  {
    GET_CLASS_NAME(JustCompileSimilarity)

  public:
    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer>
    simScorer(std::shared_ptr<Similarity::SimWeight> stats,
              std::shared_ptr<LeafReaderContext> context) override;

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

  protected:
    std::shared_ptr<JustCompileSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  class JustCompileTopDocsCollector final
      : public TopDocsCollector<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(JustCompileTopDocsCollector)

  protected:
    JustCompileTopDocsCollector(
        std::shared_ptr<PriorityQueue<std::shared_ptr<ScoreDoc>>> pq);

  public:
    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

    std::shared_ptr<TopDocs> topDocs() override;

    std::shared_ptr<TopDocs> topDocs(int start) override;

    std::shared_ptr<TopDocs> topDocs(int start, int end) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<JustCompileTopDocsCollector> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileTopDocsCollector>(
          TopDocsCollector<ScoreDoc>::shared_from_this());
    }
  };

public:
  class JustCompileWeight final : public Weight
  {
    GET_CLASS_NAME(JustCompileWeight)

  protected:
    JustCompileWeight();

  public:
    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context, int doc) override;

    std::shared_ptr<Scorer>
    scorer(std::shared_ptr<LeafReaderContext> context) override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<JustCompileWeight> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileWeight>(
          Weight::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/
