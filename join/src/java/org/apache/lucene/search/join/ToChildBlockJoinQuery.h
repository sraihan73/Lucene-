#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/util/BitSet.h"
#include  "core/src/java/org/apache/lucene/search/ChildScorer.h"
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
namespace org::apache::lucene::search::join
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitSet = org::apache::lucene::util::BitSet;

/**
 * Just like {@link ToParentBlockJoinQuery}, except this
 * query joins in reverse: you provide a Query matching
 * parent documents and it joins down to child
 * documents.
 *
 * @lucene.experimental
 */

class ToChildBlockJoinQuery : public Query
{
  GET_CLASS_NAME(ToChildBlockJoinQuery)

  /** Message thrown from {@link
   *  ToChildBlockJoinScorer#validateParentDoc} on misuse,
   *  when the parent query incorrectly returns child docs. */
public:
  static const std::wstring INVALID_QUERY_MESSAGE;
  static const std::wstring ILLEGAL_ADVANCE_ON_PARENT;

private:
  const std::shared_ptr<BitSetProducer> parentsFilter;
  const std::shared_ptr<Query> parentQuery;

  /**
   * Create a ToChildBlockJoinQuery.
   *
   * @param parentQuery Query that matches parent documents
   * @param parentsFilter Filter identifying the parent documents.
   */
public:
  ToChildBlockJoinQuery(std::shared_ptr<Query> parentQuery,
                        std::shared_ptr<BitSetProducer> parentsFilter);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** Return our parent query. */
  virtual std::shared_ptr<Query> getParentQuery();

private:
  class ToChildBlockJoinWeight : public FilterWeight
  {
    GET_CLASS_NAME(ToChildBlockJoinWeight)
  private:
    const std::shared_ptr<BitSetProducer> parentsFilter;
    const bool doScores;

  public:
    ToChildBlockJoinWeight(std::shared_ptr<Query> joinQuery,
                           std::shared_ptr<Weight> parentWeight,
                           std::shared_ptr<BitSetProducer> parentsFilter,
                           bool doScores);

    // NOTE: acceptDocs applies (and is checked) only in the
    // child document space
    std::shared_ptr<Scorer>
    scorer(std::shared_ptr<LeafReaderContext> readerContext) throw(
        IOException) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<ToChildBlockJoinWeight> shared_from_this()
    {
      return std::static_pointer_cast<ToChildBlockJoinWeight>(
          org.apache.lucene.search.FilterWeight::shared_from_this());
    }
  };

public:
  class ToChildBlockJoinScorer : public Scorer
  {
    GET_CLASS_NAME(ToChildBlockJoinScorer)
  private:
    const std::shared_ptr<Scorer> parentScorer;
    const std::shared_ptr<DocIdSetIterator> parentIt;
    const std::shared_ptr<BitSet> parentBits;
    const bool doScores;

    float parentScore = 0;

    int childDoc = -1;
    int parentDoc = 0;

  public:
    ToChildBlockJoinScorer(std::shared_ptr<Weight> weight,
                           std::shared_ptr<Scorer> parentScorer,
                           std::shared_ptr<BitSet> parentBits, bool doScores);

    std::shared_ptr<std::deque<std::shared_ptr<Scorer::ChildScorer>>>
    getChildren() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ToChildBlockJoinScorer> outerInstance;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<ToChildBlockJoinScorer> outerInstance);

      int docID() override;

      int nextDoc()  override;

      int advance(int childTarget)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            org.apache.lucene.search.DocIdSetIterator::shared_from_this());
      }
    };

    /** Detect mis-use, where provided parent query in fact
     *  sometimes returns child documents.  */
  private:
    void validateParentDoc();

  public:
    int docID() override;

    float score()  override;

    virtual int getParentDoc();

  protected:
    std::shared_ptr<ToChildBlockJoinScorer> shared_from_this()
    {
      return std::static_pointer_cast<ToChildBlockJoinScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<ToChildBlockJoinQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<ToChildBlockJoinQuery> shared_from_this()
  {
    return std::static_pointer_cast<ToChildBlockJoinQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
