#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/BlendedTermQuery.h"

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
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Builder = org::apache::lucene::search::BooleanQuery::Builder;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * An abstract {@link Query} that matches documents
 * containing a subset of terms provided by a {@link
 * FilteredTermsEnum} enumeration.
 *
 * <p>This query cannot be used directly; you must subclass
 * it and define {@link #getTermsEnum(Terms,AttributeSource)} to provide a
{@link
 * FilteredTermsEnum} that iterates through the terms to be
 * matched.
 *
 * <p><b>NOTE</b>: if {@link #setRewriteMethod} is either
 * {@link #CONSTANT_SCORE_BOOLEAN_REWRITE} or {@link
 * #SCORING_BOOLEAN_REWRITE}, you may encounter a
 * {@link BooleanQuery.TooManyClauses} exception during
 * searching, which happens when the number of terms to be
 * searched exceeds {@link
 * BooleanQuery#getMaxClauseCount()}.  Setting {@link
 * #setRewriteMethod} to {@link #CONSTANT_SCORE_REWRITE}
 * prevents this.
 *
 * <p>The recommended rewrite method is {@link
 * #CONSTANT_SCORE_REWRITE}: it doesn't spend CPU
 * computing unhelpful scores, and is the most
 * performant rewrite method given the query. If you
 * need scoring (like {@link FuzzyQuery}, use
 * {@link TopTermsScoringBooleanQueryRewrite} which uses
 * a priority queue to only collect competitive terms
 * and not hit this limitation.
 *
 * Note that org.apache.lucene.queryparser.classic.QueryParser produces
 * MultiTermQueries using {@link #CONSTANT_SCORE_REWRITE}
GET_CLASS_NAME(ic.QueryParser)
 * by default.
 */
class MultiTermQuery : public Query
{
  GET_CLASS_NAME(MultiTermQuery)
protected:
  const std::wstring field;
  std::shared_ptr<RewriteMethod> rewriteMethod = CONSTANT_SCORE_REWRITE;

  /** Abstract class that defines how the query is rewritten. */
public:
  class RewriteMethod : public std::enable_shared_from_this<RewriteMethod>
  {
    GET_CLASS_NAME(RewriteMethod)
  public:
    virtual std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query) = 0;
    /**
     * Returns the {@link MultiTermQuery}s {@link TermsEnum}
     * @see MultiTermQuery#getTermsEnum(Terms, AttributeSource)
     */
  protected:
    virtual std::shared_ptr<TermsEnum>
    getTermsEnum(std::shared_ptr<MultiTermQuery> query,
                 std::shared_ptr<Terms> terms,
                 std::shared_ptr<AttributeSource> atts) ;
  };

  /** A rewrite method that first creates a private Filter,
   *  by visiting each term in sequence and marking all docs
   *  for that term.  Matching documents are assigned a
   *  constant score equal to the query's boost.
   *
   *  <p> This method is faster than the BooleanQuery
   *  rewrite methods when the number of matched terms or
   *  matched documents is non-trivial. Also, it will never
   *  hit an errant {@link BooleanQuery.TooManyClauses}
   *  exception.
   *
   *  @see #setRewriteMethod */
public:
  static const std::shared_ptr<RewriteMethod> CONSTANT_SCORE_REWRITE;

private:
  class RewriteMethodAnonymousInnerClass : public RewriteMethod
  {
    GET_CLASS_NAME(RewriteMethodAnonymousInnerClass)
  public:
    RewriteMethodAnonymousInnerClass();

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query) override;

  protected:
    std::shared_ptr<RewriteMethodAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RewriteMethodAnonymousInnerClass>(
          RewriteMethod::shared_from_this());
    }
  };

  /** A rewrite method that first translates each term into
   *  {@link BooleanClause.Occur#SHOULD} clause in a
   *  BooleanQuery, and keeps the scores as computed by the
   *  query.  Note that typically such scores are
   *  meaningless to the user, and require non-trivial CPU
   *  to compute, so it's almost always better to use {@link
   *  #CONSTANT_SCORE_REWRITE} instead.
   *
   *  <p><b>NOTE</b>: This rewrite method will hit {@link
   *  BooleanQuery.TooManyClauses} if the number of terms
   *  exceeds {@link BooleanQuery#getMaxClauseCount}.
   *
   *  @see #setRewriteMethod */
public:
  static const std::shared_ptr<RewriteMethod> SCORING_BOOLEAN_REWRITE;

  /** Like {@link #SCORING_BOOLEAN_REWRITE} except
   *  scores are not computed.  Instead, each matching
   *  document receives a constant score equal to the
   *  query's boost.
   *
   *  <p><b>NOTE</b>: This rewrite method will hit {@link
   *  BooleanQuery.TooManyClauses} if the number of terms
   *  exceeds {@link BooleanQuery#getMaxClauseCount}.
   *
   *  @see #setRewriteMethod */
  static const std::shared_ptr<RewriteMethod> CONSTANT_SCORE_BOOLEAN_REWRITE;

  /**
   * A rewrite method that first translates each term into
   * {@link BooleanClause.Occur#SHOULD} clause in a BooleanQuery, and keeps the
   * scores as computed by the query.
   *
   * <p>
   * This rewrite method only uses the top scoring terms so it will not overflow
   * the bool max clause count. It is the default rewrite method for
   * {@link FuzzyQuery}.
   *
   * @see #setRewriteMethod
   */
public:
  class TopTermsScoringBooleanQueryRewrite final
      : public TopTermsRewrite<std::shared_ptr<BooleanQuery::Builder>>
  {
    GET_CLASS_NAME(TopTermsScoringBooleanQueryRewrite)

    /**
     * Create a TopTermsScoringBooleanQueryRewrite for
     * at most <code>size</code> terms.
     * <p>
     * NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than
     * <code>size</code>, then it will be used instead.
     */
  public:
    TopTermsScoringBooleanQueryRewrite(int size);

  protected:
    int getMaxSize() override;

    std::shared_ptr<BooleanQuery::Builder> getTopLevelBuilder() override;

    std::shared_ptr<Query> build(std::shared_ptr<Builder> builder) override;

    void addClause(std::shared_ptr<BooleanQuery::Builder> topLevel,
                   std::shared_ptr<Term> term, int docCount, float boost,
                   std::shared_ptr<TermContext> states) override;

  protected:
    std::shared_ptr<TopTermsScoringBooleanQueryRewrite> shared_from_this()
    {
      return std::static_pointer_cast<TopTermsScoringBooleanQueryRewrite>(
          TopTermsRewrite<BooleanQuery.Builder>::shared_from_this());
    }
  };

  /**
   * A rewrite method that first translates each term into
   * {@link BooleanClause.Occur#SHOULD} clause in a BooleanQuery, but adjusts
   * the frequencies used for scoring to be blended across the terms, otherwise
   * the rarest term typically ranks highest (often not useful eg in the set of
   * expanded terms in a FuzzyQuery).
   *
   * <p>
   * This rewrite method only uses the top scoring terms so it will not overflow
   * the bool max clause count.
   *
   * @see #setRewriteMethod
   */
public:
  class TopTermsBlendedFreqScoringRewrite final
      : public TopTermsRewrite<std::shared_ptr<BlendedTermQuery::Builder>>
  {
    GET_CLASS_NAME(TopTermsBlendedFreqScoringRewrite)

    /**
     * Create a TopTermsBlendedScoringBooleanQueryRewrite for at most
     * <code>size</code> terms.
     * <p>
     * NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than
     * <code>size</code>, then it will be used instead.
     */
  public:
    TopTermsBlendedFreqScoringRewrite(int size);

  protected:
    int getMaxSize() override;

    std::shared_ptr<BlendedTermQuery::Builder> getTopLevelBuilder() override;

    std::shared_ptr<Query>
    build(std::shared_ptr<BlendedTermQuery::Builder> builder) override;

    void addClause(std::shared_ptr<BlendedTermQuery::Builder> topLevel,
                   std::shared_ptr<Term> term, int docCount, float boost,
                   std::shared_ptr<TermContext> states) override;

  protected:
    std::shared_ptr<TopTermsBlendedFreqScoringRewrite> shared_from_this()
    {
      return std::static_pointer_cast<TopTermsBlendedFreqScoringRewrite>(
          TopTermsRewrite<BlendedTermQuery.Builder>::shared_from_this());
    }
  };

  /**
   * A rewrite method that first translates each term into
   * {@link BooleanClause.Occur#SHOULD} clause in a BooleanQuery, but the scores
   * are only computed as the boost.
   * <p>
   * This rewrite method only uses the top scoring terms so it will not overflow
   * the bool max clause count.
   *
   * @see #setRewriteMethod
   */
public:
  class TopTermsBoostOnlyBooleanQueryRewrite final
      : public TopTermsRewrite<std::shared_ptr<BooleanQuery::Builder>>
  {
    GET_CLASS_NAME(TopTermsBoostOnlyBooleanQueryRewrite)

    /**
     * Create a TopTermsBoostOnlyBooleanQueryRewrite for
     * at most <code>size</code> terms.
     * <p>
     * NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than
     * <code>size</code>, then it will be used instead.
     */
  public:
    TopTermsBoostOnlyBooleanQueryRewrite(int size);

  protected:
    int getMaxSize() override;

    std::shared_ptr<BooleanQuery::Builder> getTopLevelBuilder() override;

    std::shared_ptr<Query>
    build(std::shared_ptr<BooleanQuery::Builder> builder) override;

    void addClause(std::shared_ptr<BooleanQuery::Builder> topLevel,
                   std::shared_ptr<Term> term, int docFreq, float boost,
                   std::shared_ptr<TermContext> states) override;

  protected:
    std::shared_ptr<TopTermsBoostOnlyBooleanQueryRewrite> shared_from_this()
    {
      return std::static_pointer_cast<TopTermsBoostOnlyBooleanQueryRewrite>(
          TopTermsRewrite<BooleanQuery.Builder>::shared_from_this());
    }
  };

  /**
   * Constructs a query matching terms that cannot be represented with a single
   * Term.
   */
public:
  MultiTermQuery(const std::wstring &field);

  /** Returns the field name for this query */
  std::wstring getField();

  /** Construct the enumeration to be used, expanding the
   *  pattern term.  This method should only be called if
   *  the field exists (ie, implementations can assume the
   *  field does exist).  This method should not return null
   *  (should instead return {@link TermsEnum#EMPTY} if no
   *  terms match).  The TermsEnum must already be
   *  positioned to the first matching term.
   * The given {@link AttributeSource} is passed by the {@link RewriteMethod} to
   * provide attributes, the rewrite method uses to inform about e.g. maximum
   * competitive boosts. This is currently only used by {@link TopTermsRewrite}
   */
protected:
  virtual std::shared_ptr<TermsEnum>
  getTermsEnum(std::shared_ptr<Terms> terms,
               std::shared_ptr<AttributeSource> atts) = 0;

  /** Convenience method, if no attributes are needed:
   * This simply passes empty attributes and is equal to:
   * <code>getTermsEnum(terms, new AttributeSource())</code>
   */
  std::shared_ptr<TermsEnum>
  getTermsEnum(std::shared_ptr<Terms> terms) ;

  /**
   * To rewrite to a simpler form, instead return a simpler
   * enum from {@link #getTermsEnum(Terms, AttributeSource)}.  For example,
   * to rewrite to a single term, return a {@link SingleTermsEnum}
   */
public:
  std::shared_ptr<Query> rewrite(std::shared_ptr<IndexReader> reader) throw(
      IOException) override final;

  /**
   * @see #setRewriteMethod
   */
  virtual std::shared_ptr<RewriteMethod> getRewriteMethod();

  /**
   * Sets the rewrite method to be used when executing the
   * query.  You can use one of the four core methods, or
   * implement your own subclass of {@link RewriteMethod}. */
  virtual void setRewriteMethod(std::shared_ptr<RewriteMethod> method);

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<MultiTermQuery> other);

protected:
  std::shared_ptr<MultiTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<MultiTermQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
