#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
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

namespace org::apache::lucene::search::join
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/**
 * A query that returns all the matching child documents for a specific parent
 * document indexed together in the same block. The provided child query
 * determines which matching child doc is being returned.
 *
 * @lucene.experimental
 */
class ParentChildrenBlockJoinQuery : public Query
{
  GET_CLASS_NAME(ParentChildrenBlockJoinQuery)

private:
  const std::shared_ptr<BitSetProducer> parentFilter;
  const std::shared_ptr<Query> childQuery;
  const int parentDocId;

  /**
   * Creates a <code>ParentChildrenBlockJoinQuery</code> instance
   *
   * @param parentFilter  A filter identifying parent documents.
   * @param childQuery    A child query that determines which child docs are
   * matching
   * @param parentDocId   The top level doc id of that parent to return children
   * documents for
   */
public:
  ParentChildrenBlockJoinQuery(std::shared_ptr<BitSetProducer> parentFilter,
                               std::shared_ptr<Query> childQuery,
                               int parentDocId);

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<ParentChildrenBlockJoinQuery> outerInstance;

    std::shared_ptr<Weight> childWeight;
    int readerIndex = 0;

  public:
    WeightAnonymousInnerClass(
        std::shared_ptr<ParentChildrenBlockJoinQuery> outerInstance,
        std::shared_ptr<Weight> childWeight, int readerIndex);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      int localParentDocId = 0;
      int firstChildDocId = 0;
      std::shared_ptr<DocIdSetIterator> childrenIterator;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          int localParentDocId, int firstChildDocId,
          std::shared_ptr<DocIdSetIterator> childrenIterator);

      int doc = 0;

      int docID() override;

      int nextDoc()  override;

      int advance(int target)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            org.apache.lucene.search.DocIdSetIterator::shared_from_this());
      }
    };

  private:
    class ScorerAnonymousInnerClass : public Scorer
    {
      GET_CLASS_NAME(ScorerAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<Scorer> childrenScorer;
      std::shared_ptr<DocIdSetIterator> it;

    public:
      ScorerAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<Scorer> childrenScorer,
          std::shared_ptr<DocIdSetIterator> it);

      int docID() override;

      float score()  override;

      std::shared_ptr<DocIdSetIterator> iterator() override;

    protected:
      std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerAnonymousInnerClass>(
            org.apache.lucene.search.Scorer::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ParentChildrenBlockJoinQuery> shared_from_this()
  {
    return std::static_pointer_cast<ParentChildrenBlockJoinQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
