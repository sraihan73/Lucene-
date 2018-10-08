#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanRewriteMethod.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"

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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using TopTermsRewrite = org::apache::lucene::search::TopTermsRewrite;

/**
 * Wraps any {@link MultiTermQuery} as a {@link SpanQuery},
 * so it can be nested within other SpanQuery classes.
 * <p>
 * The query is rewritten by default to a {@link SpanOrQuery} containing
 * the expanded terms, but this can be customized.
 * <p>
 * Example:
 * <blockquote><pre class="prettyprint">
 * {@code
GET_CLASS_NAME(="prettyprint">)
 * WildcardQuery wildcard = new WildcardQuery(new Term("field", "bro?n"));
 * SpanQuery spanWildcard = new
SpanMultiTermQueryWrapper<WildcardQuery>(wildcard);
 * // do something with spanWildcard, such as use it in a SpanFirstQuery
 * }
 * </pre></blockquote>
 */
template <typename Q>
class SpanMultiTermQueryWrapper : public SpanQuery
{
  GET_CLASS_NAME(SpanMultiTermQueryWrapper)
  static_assert(
      std::is_base_of<org.apache.lucene.search.MultiTermQuery, Q>::value,
      L"Q must inherit from org.apache.lucene.search.MultiTermQuery");

protected:
  const Q query;

private:
  std::shared_ptr<SpanRewriteMethod> rewriteMethod;

  /**
   * Create a new SpanMultiTermQueryWrapper.
   *
   * @param query Query to wrap.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public
  // SpanMultiTermQueryWrapper(Q query)
  SpanMultiTermQueryWrapper(Q query) : query(Objects::requireNonNull(query))
  {
    this->rewriteMethod = selectRewriteMethod(query);
  }

private:
  static std::shared_ptr<SpanRewriteMethod>
  selectRewriteMethod(std::shared_ptr<MultiTermQuery> query)
  {
    std::shared_ptr<MultiTermQuery::RewriteMethod> method =
        query->getRewriteMethod();
    if (std::dynamic_pointer_cast<TopTermsRewrite>(method) != nullptr) {
      constexpr int pqsize =
          (std::static_pointer_cast<TopTermsRewrite>(method))->getSize();
      return std::make_shared<TopTermsSpanBooleanQueryRewrite>(pqsize);
    } else {
      return SCORING_SPAN_QUERY_REWRITE;
    }
  }

  /**
   * Expert: returns the rewriteMethod
   */
public:
  std::shared_ptr<SpanRewriteMethod> getRewriteMethod()
  {
    return rewriteMethod;
  }

  /**
   * Expert: sets the rewrite method. This only makes sense
   * to be a span rewrite method.
   */
  void setRewriteMethod(std::shared_ptr<SpanRewriteMethod> rewriteMethod)
  {
    this->rewriteMethod = rewriteMethod;
  }

  std::wstring getField() override { return query->getField(); }

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override
  {
    throw std::invalid_argument("Rewrite first!");
  }

  /** Returns the wrapped query */
  virtual std::shared_ptr<Query> getWrappedQuery() { return query; }

  std::wstring toString(const std::wstring &field) override
  {
    std::shared_ptr<StringBuilder> builder = std::make_shared<StringBuilder>();
    builder->append(L"SpanMultiTermQueryWrapper(");
    // NOTE: query.toString must be placed in a temp local to avoid compile
    // errors on Java 8u20 see
    // https://bugs.openjdk.java.net/browse/JDK-8056984?page=com.atlassian.streams.streams-jira-plugin:activity-stream-issue-tab
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring queryStr = query->toString(field);
    builder->append(queryStr);
    builder->append(L")");
    return builder->toString();
  }

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override
  {
    return rewriteMethod->rewrite(reader, query);
  }

  int hashCode() override { return classHash() * 31 + query->hashCode(); }

  bool equals(std::any other) override
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return sameClassAs(other) &&
    // query.equals(((SpanMultiTermQueryWrapper<?>) other).query);
        return sameClassAs(other) && query->equals((std::any_cast<SpanMultiTermQueryWrapper<?>>(other)).query);
  }

  /** Abstract class that defines how the query is rewritten. */
public:
  class SpanRewriteMethod : public MultiTermQuery::RewriteMethod
  {
    GET_CLASS_NAME(SpanRewriteMethod)
  public:
    std::shared_ptr<SpanQuery>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query) = 0;
    override override;

  protected:
    std::shared_ptr<SpanRewriteMethod> shared_from_this()
    {
      return std::static_pointer_cast<SpanRewriteMethod>(
          org.apache.lucene.search.MultiTermQuery
              .RewriteMethod::shared_from_this());
    }
  };

  /**
   * A rewrite method that first translates each term into a SpanTermQuery in a
   * {@link Occur#SHOULD} clause in a BooleanQuery, and keeps the
   * scores as computed by the query.
   *
   * @see #setRewriteMethod
   */
public:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<SpanRewriteMethod> SCORING_SPAN_QUERY_REWRITE =
      std::make_shared<SpanRewriteMethodAnonymousInnerClass>();

private:
  class SpanRewriteMethodAnonymousInnerClass : public SpanRewriteMethod
  {
    GET_CLASS_NAME(SpanRewriteMethodAnonymousInnerClass)
  public:
    SpanRewriteMethodAnonymousInnerClass();

  private:
    const std::shared_ptr<
        ScoringRewrite<std::deque<std::shared_ptr<SpanQuery>>>>
        delegate_ = std::make_shared<ScoringRewriteAnonymousInnerClass>();

  private:
    class ScoringRewriteAnonymousInnerClass
        : public ScoringRewrite<std::deque<std::shared_ptr<SpanQuery>>>
    {
      GET_CLASS_NAME(ScoringRewriteAnonymousInnerClass)
    public:
      ScoringRewriteAnonymousInnerClass();

    protected:
      std::deque<std::shared_ptr<SpanQuery>> getTopLevelBuilder() override;

      std::shared_ptr<Query>
      build(std::deque<std::shared_ptr<SpanQuery>> &builder);

      void checkMaxClauseCount(int count) override;

      void addClause(std::deque<std::shared_ptr<SpanQuery>> &topLevel,
                     std::shared_ptr<Term> term, int docCount, float boost,
                     std::shared_ptr<TermContext> states) override;

    protected:
      std::shared_ptr<ScoringRewriteAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScoringRewriteAnonymousInnerClass>(
            org.apache.lucene.search
                .ScoringRewrite<java.util.List<SpanQuery>>::shared_from_this());
      }
    };

  public:
    std::shared_ptr<SpanQuery>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query)  override;

  protected:
    std::shared_ptr<SpanRewriteMethodAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SpanRewriteMethodAnonymousInnerClass>(
          SpanRewriteMethod::shared_from_this());
    }
  };

  /**
   * A rewrite method that first translates each term into a SpanTermQuery in a
   * {@link Occur#SHOULD} clause in a BooleanQuery, and keeps the
   * scores as computed by the query.
   *
   * <p>
   * This rewrite method only uses the top scoring terms so it will not overflow
   * the bool max clause count.
   *
   * @see #setRewriteMethod
   */
public:
  class TopTermsSpanBooleanQueryRewrite final : public SpanRewriteMethod
  {
    GET_CLASS_NAME(TopTermsSpanBooleanQueryRewrite)
  private:
    const std::shared_ptr<
        TopTermsRewrite<std::deque<std::shared_ptr<SpanQuery>>>>
        delegate_;

    /**
     * Create a TopTermsSpanBooleanQueryRewrite for
     * at most <code>size</code> terms.
     */
  public:
    TopTermsSpanBooleanQueryRewrite(int size);

    /** return the maximum priority queue size */
  public:
    int getSize();

    std::shared_ptr<SpanQuery>
    rewrite(std::shared_ptr<IndexReader> reader,
            std::shared_ptr<MultiTermQuery> query)  override;

    virtual int hashCode();

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<TopTermsSpanBooleanQueryRewrite> shared_from_this()
    {
      return std::static_pointer_cast<TopTermsSpanBooleanQueryRewrite>(
          SpanRewriteMethod::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SpanMultiTermQueryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<SpanMultiTermQueryWrapper>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
