#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"

#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/ParallelArraysTermCollector.h"
#include  "core/src/java/org/apache/lucene/search/TermFreqBoostByteStart.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/search/BoostAttribute.h"
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
using TermContext = org::apache::lucene::index::TermContext;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

/**
 * Base rewrite method that translates each term into a query, and keeps
 * the scores as computed by the query.
 * <p>
 * @lucene.internal Only public to be accessible by spans package. */
template <typename B>
class ScoringRewrite : public TermCollectingRewrite<B>
{
  GET_CLASS_NAME(ScoringRewrite)

  /** A rewrite method that first translates each term into
   *  {@link BooleanClause.Occur#SHOULD} clause in a
   *  BooleanQuery, and keeps the scores as computed by the
   *  query.  Note that typically such scores are
   *  meaningless to the user, and require non-trivial CPU
   *  to compute, so it's almost always better to use {@link
   *  MultiTermQuery#CONSTANT_SCORE_REWRITE} instead.
   *
   *  <p><b>NOTE</b>: This rewrite method will hit {@link
   *  BooleanQuery.TooManyClauses} if the number of terms
   *  exceeds {@link BooleanQuery#getMaxClauseCount}.
   *
   *  @see MultiTermQuery#setRewriteMethod */
public:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<
      ScoringRewrite<std::shared_ptr<BooleanQuery::Builder>>>
      SCORING_BOOLEAN_REWRITE =
          std::make_shared<ScoringRewriteAnonymousInnerClass>();

private:
  class ScoringRewriteAnonymousInnerClass
      : public ScoringRewrite<std::shared_ptr<BooleanQuery::Builder>>
  {
    GET_CLASS_NAME(ScoringRewriteAnonymousInnerClass)
  public:
    ScoringRewriteAnonymousInnerClass();

  protected:
    std::shared_ptr<BooleanQuery::Builder> getTopLevelBuilder() override;

    std::shared_ptr<Query>
    build(std::shared_ptr<BooleanQuery::Builder> builder) override;

    void addClause(std::shared_ptr<BooleanQuery::Builder> topLevel,
                   std::shared_ptr<Term> term, int docCount, float boost,
                   std::shared_ptr<TermContext> states) override;

    void checkMaxClauseCount(int count) override;

  protected:
    std::shared_ptr<ScoringRewriteAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScoringRewriteAnonymousInnerClass>(
          ScoringRewrite<BooleanQuery.Builder>::shared_from_this());
    }
  };

  /** Like {@link #SCORING_BOOLEAN_REWRITE} except
   *  scores are not computed.  Instead, each matching
   *  document receives a constant score equal to the
   *  query's boost.
   *
   *  <p><b>NOTE</b>: This rewrite method will hit {@link
   *  BooleanQuery.TooManyClauses} if the number of terms
   *  exceeds {@link BooleanQuery#getMaxClauseCount}.
   *
   *  @see MultiTermQuery#setRewriteMethod */
public:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<RewriteMethod> CONSTANT_SCORE_BOOLEAN_REWRITE =
      std::make_shared<RewriteMethodAnonymousInnerClass>();

private:
  class RewriteMethodAnonymousInnerClass : public RewriteMethod
  {
    GET_CLASS_NAME(RewriteMethodAnonymousInnerClass)
  public:
    RewriteMethodAnonymousInnerClass();

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query)  override;

  protected:
    std::shared_ptr<RewriteMethodAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RewriteMethodAnonymousInnerClass>(
          org.apache.lucene.search.MultiTermQuery
              .RewriteMethod::shared_from_this());
    }
  };

  /** This method is called after every new term to check if the number of max
   * clauses (e.g. in BooleanQuery) is not exceeded. Throws the corresponding
   * {@link RuntimeException}. */
protected:
  virtual void checkMaxClauseCount(int count) = 0;

public:
  std::shared_ptr<Query> rewrite(
      std::shared_ptr<IndexReader> reader,
      std::shared_ptr<MultiTermQuery> query)  override final
  {
    constexpr B builder = getTopLevelBuilder();
    std::shared_ptr<ParallelArraysTermCollector> *const col =
        std::make_shared<ParallelArraysTermCollector>(shared_from_this());
    collectTerms(reader, query, col);

    constexpr int size = col->terms->size();
    if (size > 0) {
      const std::deque<int> sort = col->terms->sort();
      const std::deque<float> boost = col->array_->boost;
      std::deque<std::shared_ptr<TermContext>> termStates =
          col->array_->termState;
      for (int i = 0; i < size; i++) {
        constexpr int pos = sort[i];
        std::shared_ptr<Term> *const term = std::make_shared<Term>(
            query->getField(),
            col->terms->get(pos, std::make_shared<BytesRef>()));
        assert(reader->docFreq(term) == termStates[pos]->docFreq());
        addClause(builder, term, termStates[pos]->docFreq(), boost[pos],
                  termStates[pos]);
      }
    }
    return build(builder);
  }

public:
  class ParallelArraysTermCollector final : public TermCollector
  {
    GET_CLASS_NAME(ParallelArraysTermCollector)
  private:
    std::shared_ptr<ScoringRewrite<std::shared_ptr<B>>> outerInstance;

  public:
    ParallelArraysTermCollector(
        std::shared_ptr<ScoringRewrite<B>> outerInstance);

    const std::shared_ptr<TermFreqBoostByteStart> array_ =
        std::make_shared<TermFreqBoostByteStart>(16);
    const std::shared_ptr<BytesRefHash> terms = std::make_shared<BytesRefHash>(
        std::make_shared<ByteBlockPool>(
            std::make_shared<ByteBlockPool::DirectAllocator>()),
        16, array_);
    std::shared_ptr<TermsEnum> termsEnum;

  private:
    std::shared_ptr<BoostAttribute> boostAtt;

  public:
    void setNextEnum(std::shared_ptr<TermsEnum> termsEnum) override;

    bool collect(std::shared_ptr<BytesRef> bytes)  override;

  protected:
    std::shared_ptr<ParallelArraysTermCollector> shared_from_this()
    {
      return std::static_pointer_cast<ParallelArraysTermCollector>(
          TermCollector::shared_from_this());
    }
  };

  /** Special implementation of BytesStartArray that keeps parallel arrays for
   * boost and docFreq */
public:
  class TermFreqBoostByteStart final : public DirectBytesStartArray
  {
    GET_CLASS_NAME(TermFreqBoostByteStart)
  public:
    std::deque<float> boost;
    std::deque<std::shared_ptr<TermContext>> termState;

    TermFreqBoostByteStart(int initSize);

    std::deque<int> init() override;

    std::deque<int> grow() override;

    std::deque<int> clear() override;

  protected:
    std::shared_ptr<TermFreqBoostByteStart> shared_from_this()
    {
      return std::static_pointer_cast<TermFreqBoostByteStart>(
          org.apache.lucene.util.BytesRefHash
              .DirectBytesStartArray::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ScoringRewrite> shared_from_this()
  {
    return std::static_pointer_cast<ScoringRewrite>(
        TermCollectingRewrite<B>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
