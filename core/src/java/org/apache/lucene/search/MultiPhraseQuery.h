#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/PhraseMatcher.h"
#include  "core/src/java/org/apache/lucene/search/DocsQueue.h"
#include  "core/src/java/org/apache/lucene/search/PositionsQueue.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * A generalized version of {@link PhraseQuery}, with the possibility of
 * adding more than one term at the same position that are treated as a
disjunction (OR).
 * To use this class to search for the phrase "Microsoft app*" first create a
Builder and use
 * {@link Builder#add(Term)} on the term "microsoft" (assuming lowercase
analysis), then GET_CLASS_NAME(to)
 * find all terms that have "app" as prefix using {@link
LeafReader#terms(std::wstring)},
 * seeking to "app" then iterating and collecting terms until there is no longer
 * that prefix, and finally use {@link Builder#add(Term[])} to add them.
 * {@link Builder#build()} returns the fully constructed (and immutable)
MultiPhraseQuery.
 */
class MultiPhraseQuery : public Query
{
  GET_CLASS_NAME(MultiPhraseQuery)
  /** A builder for multi-phrase queries */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    std::wstring field; // becomes non-null on first add() then is unmodified
    const std::deque<std::deque<std::shared_ptr<Term>>> termArrays;
    const std::deque<int> positions;
    int slop = 0;

    /** Default constructor. */
  public:
    Builder();

    /** Copy constructor: this will create a builder that has the same
     *  configuration as the provided builder. */
    Builder(std::shared_ptr<MultiPhraseQuery> multiPhraseQuery);

    /** Sets the phrase slop for this query.
     * @see PhraseQuery#getSlop()
     */
    virtual std::shared_ptr<Builder> setSlop(int s);

    /** Add a single term at the next position in the phrase.
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term);

    /** Add multiple terms at the next position in the phrase.  Any of the terms
     * may match (a disjunction).
     * The array is not copied or mutated, the caller should consider it
     * immutable subsequent to calling this method.
     */
    virtual std::shared_ptr<Builder>
    add(std::deque<std::shared_ptr<Term>> &terms);

    /**
     * Allows to specify the relative position of terms within the phrase.
     * The array is not copied or mutated, the caller should consider it
     * immutable subsequent to calling this method.
     */
    virtual std::shared_ptr<Builder>
    add(std::deque<std::shared_ptr<Term>> &terms, int position);

    /** Builds a {@link MultiPhraseQuery}. */
    virtual std::shared_ptr<MultiPhraseQuery> build();
  };

private:
  const std::wstring field;
  std::deque<std::deque<std::shared_ptr<Term>>> const termArrays;
  std::deque<int> const positions;
  const int slop;

  MultiPhraseQuery(const std::wstring &field,
                   std::deque<std::deque<std::shared_ptr<Term>>> &termArrays,
                   std::deque<int> &positions, int slop);

  /** Sets the phrase slop for this query.
   * @see PhraseQuery#getSlop()
   */
public:
  virtual int getSlop();

  /**
   * Returns the arrays of arrays of terms in the multi-phrase.
   * Do not modify!
   */
  virtual std::deque<std::deque<std::shared_ptr<Term>>> getTermArrays();

  /**
   * Returns the relative positions of terms in this phrase.
   * Do not modify!
   */
  virtual std::deque<int> getPositions();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class PhraseWeightAnonymousInnerClass : public PhraseWeight
  {
    GET_CLASS_NAME(PhraseWeightAnonymousInnerClass)
  private:
    std::shared_ptr<MultiPhraseQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher;
    bool needsScores = false;
    float boost = 0;
    std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        termStates;

  public:
    PhraseWeightAnonymousInnerClass(
        std::shared_ptr<MultiPhraseQuery> outerInstance,
        const std::wstring &field,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores, float boost,
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &termStates);

  protected:
    std::shared_ptr<Similarity::SimWeight> getStats(
        std::shared_ptr<IndexSearcher> searcher)  override;

    std::shared_ptr<PhraseMatcher>
    getPhraseMatcher(std::shared_ptr<LeafReaderContext> context,
                     bool exposeOffsets)  override;

  public:
    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  protected:
    std::shared_ptr<PhraseWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PhraseWeightAnonymousInnerClass>(
          PhraseWeight::shared_from_this());
    }
  };

  /** Prints a user-readable version of this query. */
public:
  std::wstring toString(const std::wstring &f) override final;

  /** Returns true if <code>o</code> is equal to this. */
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<MultiPhraseQuery> other);

  /** Returns a hash code value for this object.*/
public:
  virtual int hashCode();

  // Breakout calculation of the termArrays hashcode
private:
  int termArraysHashCode();

  // Breakout calculation of the termArrays equals
  bool termArraysEquals(
      std::deque<std::deque<std::shared_ptr<Term>>> &termArrays1,
      std::deque<std::deque<std::shared_ptr<Term>>> &termArrays2);

  /**
   * Takes the logical union of multiple PostingsEnum iterators.
   * <p>
   * Note: positions are merged during freq()
   */
public:
  class UnionPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(UnionPostingsEnum)
    /** queue ordered by docid */
  public:
    const std::shared_ptr<DocsQueue> docsQueue;
    /** cost of this enum: sum of its subs */
    // C++ NOTE: Fields cannot have the same name as methods:
    const int64_t cost_;

    /** queue ordered by position for current doc */
    const std::shared_ptr<PositionsQueue> posQueue =
        std::make_shared<PositionsQueue>();
    /** current doc posQueue is working */
    int posQueueDoc = -2;
    /** deque of subs (unordered) */
    std::deque<std::shared_ptr<PostingsEnum>> const subs;

    UnionPostingsEnum(
        std::shared_ptr<std::deque<std::shared_ptr<PostingsEnum>>> subs);

    int freq()  override;

    int nextPosition()  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    /**
     * disjunction of postings ordered by docid.
     */
  public:
    class DocsQueue : public PriorityQueue<std::shared_ptr<PostingsEnum>>
    {
      GET_CLASS_NAME(DocsQueue)
    public:
      DocsQueue(int size);

      bool lessThan(std::shared_ptr<PostingsEnum> a,
                    std::shared_ptr<PostingsEnum> b) override;

    protected:
      std::shared_ptr<DocsQueue> shared_from_this()
      {
        return std::static_pointer_cast<DocsQueue>(
            org.apache.lucene.util.PriorityQueue<
                org.apache.lucene.index.PostingsEnum>::shared_from_this());
      }
    };

    /**
     * queue of terms for a single document. its a sorted array of
     * all the positions from all the postings
     */
  public:
    class PositionsQueue : public std::enable_shared_from_this<PositionsQueue>
    {
      GET_CLASS_NAME(PositionsQueue)
    private:
      int arraySize = 16;
      int index = 0;
      // C++ NOTE: Fields cannot have the same name as methods:
      int size_ = 0;
      std::deque<int> array_ = std::deque<int>(arraySize);

    public:
      virtual void add(int i);

      virtual int next();

      virtual void sort();

      virtual void clear();

      virtual int size();

    private:
      void growArray();
    };

  protected:
    std::shared_ptr<UnionPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<UnionPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

public:
  class PostingsAndPosition
      : public std::enable_shared_from_this<PostingsAndPosition>
  {
    GET_CLASS_NAME(PostingsAndPosition)
  public:
    const std::shared_ptr<PostingsEnum> pe;
    int pos = 0;
    int upto = 0;

    PostingsAndPosition(std::shared_ptr<PostingsEnum> pe);
  };

  // Slower version of UnionPostingsEnum that delegates offsets and positions,
  // for use by MatchesIterator
public:
  class UnionFullPostingsEnum : public UnionPostingsEnum
  {
    GET_CLASS_NAME(UnionFullPostingsEnum)

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = -1;
    bool started = false;

    const std::shared_ptr<PriorityQueue<std::shared_ptr<PostingsAndPosition>>>
        posQueue;
    const std::shared_ptr<std::deque<std::shared_ptr<PostingsAndPosition>>>
        subs;

    UnionFullPostingsEnum(std::deque<std::shared_ptr<PostingsEnum>> &subs);

  public:
    int freq()  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

  protected:
    std::shared_ptr<UnionFullPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<UnionFullPostingsEnum>(
          UnionPostingsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MultiPhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<MultiPhraseQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
