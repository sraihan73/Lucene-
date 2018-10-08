#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::search
{
class RewriteMethod;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class IndexReaderContext;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;

/**
 * A {@link Query} that blends index statistics across multiple terms.
 * This is particularly useful when several terms should produce identical
 * scores, regardless of their index statistics.
 * <p>For instance imagine that you are resolving synonyms at search time,
 * all terms should produce identical scores instead of the default behavior,
 * which tends to give higher scores to rare terms.
 * <p>An other useful use-case is cross-field search: imagine that you would
 * like to search for {@code john} on two fields: {@code first_name} and
 * {@code last_name}. You might not want to give a higher weight to matches
 * on the field where {@code john} is rarer, in which case
 * {@link BlendedTermQuery} would help as well.
 * @lucene.experimental
 */
class BlendedTermQuery final : public Query
{
  GET_CLASS_NAME(BlendedTermQuery)

  /** A Builder for {@link BlendedTermQuery}. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)

  private:
    int numTerms = 0;
    std::deque<std::shared_ptr<Term>> terms =
        std::deque<std::shared_ptr<Term>>(0);
    std::deque<float> boosts = std::deque<float>(0);
    std::deque<std::shared_ptr<TermContext>> contexts =
        std::deque<std::shared_ptr<TermContext>>(0);
    std::shared_ptr<RewriteMethod> rewriteMethod = DISJUNCTION_MAX_REWRITE;

    /** Sole constructor. */
  public:
    Builder();

    /** Set the {@link RewriteMethod}. Default is to use
     *  {@link BlendedTermQuery#DISJUNCTION_MAX_REWRITE}.
     *  @see RewriteMethod */
    virtual std::shared_ptr<Builder>
    setRewriteMethod(std::shared_ptr<RewriteMethod> rewiteMethod);

    /** Add a new {@link Term} to this builder, with a default boost of {@code
     * 1}.
     *  @see #add(Term, float) */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term);

    /** Add a {@link Term} with the provided boost. The higher the boost, the
     *  more this term will contribute to the overall score of the
     *  {@link BlendedTermQuery}. */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term,
                                         float boost);

    /**
     * Expert: Add a {@link Term} with the provided boost and context.
     * This method is useful if you already have a {@link TermContext}
     * object constructed for the given term.
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term,
                                         float boost,
                                         std::shared_ptr<TermContext> context);

    /** Build the {@link BlendedTermQuery}. */
    virtual std::shared_ptr<BlendedTermQuery> build();
  };

  /** A {@link RewriteMethod} defines how queries for individual terms should
   *  be merged.
   *  @lucene.experimental
   *  @see BlendedTermQuery#BOOLEAN_REWRITE
   *  @see BlendedTermQuery.DisjunctionMaxRewrite */
public:
  class RewriteMethod : public std::enable_shared_from_this<RewriteMethod>
  {
    GET_CLASS_NAME(RewriteMethod)

    /** Sole constructor */
  protected:
    RewriteMethod();

    /** Merge the provided sub queries into a single {@link Query} object. */
  public:
    virtual std::shared_ptr<Query>
    rewrite(std::deque<std::shared_ptr<Query>> &subQueries) = 0;
  };

  /**
   * A {@link RewriteMethod} that adds all sub queries to a {@link
   * BooleanQuery}. This {@link RewriteMethod} is useful when matching on
   * several fields is considered better than having a good match on a single
   * field.
   */
public:
  static const std::shared_ptr<RewriteMethod> BOOLEAN_REWRITE;

private:
  class RewriteMethodAnonymousInnerClass : public RewriteMethod
  {
    GET_CLASS_NAME(RewriteMethodAnonymousInnerClass)
  public:
    RewriteMethodAnonymousInnerClass();

    std::shared_ptr<Query>
    rewrite(std::deque<std::shared_ptr<Query>> &subQueries) override;

  protected:
    std::shared_ptr<RewriteMethodAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RewriteMethodAnonymousInnerClass>(
          RewriteMethod::shared_from_this());
    }
  };

  /**
   * A {@link RewriteMethod} that creates a {@link DisjunctionMaxQuery} out
   * of the sub queries. This {@link RewriteMethod} is useful when having a
   * good match on a single field is considered better than having average
   * matches on several fields.
   */
public:
  class DisjunctionMaxRewrite : public RewriteMethod
  {
    GET_CLASS_NAME(DisjunctionMaxRewrite)

  private:
    const float tieBreakerMultiplier;

    /** This {@link RewriteMethod} will create {@link DisjunctionMaxQuery}
     *  instances that have the provided tie breaker.
     *  @see DisjunctionMaxQuery */
  public:
    DisjunctionMaxRewrite(float tieBreakerMultiplier);

    std::shared_ptr<Query>
    rewrite(std::deque<std::shared_ptr<Query>> &subQueries) override;

    bool equals(std::any obj) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<DisjunctionMaxRewrite> shared_from_this()
    {
      return std::static_pointer_cast<DisjunctionMaxRewrite>(
          RewriteMethod::shared_from_this());
    }
  };

  /** {@link DisjunctionMaxRewrite} instance with a tie-breaker of {@code 0.01}.
   */
public:
  static const std::shared_ptr<RewriteMethod> DISJUNCTION_MAX_REWRITE;

private:
  std::deque<std::shared_ptr<Term>> const terms;
  std::deque<float> const boosts;
  std::deque<std::shared_ptr<TermContext>> const contexts;
  const std::shared_ptr<RewriteMethod> rewriteMethod;

  BlendedTermQuery(std::deque<std::shared_ptr<Term>> &terms,
                   std::deque<float> &boosts,
                   std::deque<std::shared_ptr<TermContext>> &contexts,
                   std::shared_ptr<RewriteMethod> rewriteMethod);

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  private:
    std::shared_ptr<BlendedTermQuery> outerInstance;

    std::deque<std::shared_ptr<Term>> terms;
    std::deque<float> boosts;
    std::deque<std::shared_ptr<TermContext>> contexts;

  public:
    InPlaceMergeSorterAnonymousInnerClass(
        std::shared_ptr<BlendedTermQuery> outerInstance,
        std::deque<std::shared_ptr<Term>> &terms, std::deque<float> &boosts,
        std::deque<std::shared_ptr<TermContext>> &contexts);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

public:
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<BlendedTermQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Query> rewrite(std::shared_ptr<IndexReader> reader) throw(
      IOException) override final;

private:
  static std::shared_ptr<TermContext>
  adjustFrequencies(std::shared_ptr<IndexReaderContext> readerContext,
                    std::shared_ptr<TermContext> ctx, int artificialDf,
                    int64_t artificialTtf);

protected:
  std::shared_ptr<BlendedTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<BlendedTermQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
