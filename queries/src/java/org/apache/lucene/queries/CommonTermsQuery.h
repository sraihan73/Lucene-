#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::search
{
class BooleanClause;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
namespace org::apache::lucene::queries
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Query = org::apache::lucene::search::Query;

/**
 * A query that executes high-frequency terms in a optional sub-query to prevent
 * slow queries due to "common" terms like stopwords. This query
 * builds 2 queries off the {@link #add(Term) added} terms: low-frequency
 * terms are added to a required bool clause and high-frequency terms are
 * added to an optional bool clause. The optional clause is only executed if
 * the required "low-frequency" clause matches. In most cases, high-frequency
 * terms are unlikely to significantly contribute to the document score unless
 * at least one of the low-frequency terms are matched.  This query can improve
 * query execution times significantly if applicable.
 * <p>
 * {@link CommonTermsQuery} has several advantages over stopword filtering at
 * index or query time since a term can be "classified" based on the actual
 * document frequency in the index and can prevent slow queries even across
 * domains without specialized stopword files.
 * </p>
 * <p>
 * <b>Note:</b> if the query only contains high-frequency terms the query is
 * rewritten into a plain conjunction query ie. all high-frequency terms need to
 * match in order to match a document.
 * </p>
 */
class CommonTermsQuery : public Query
{
  GET_CLASS_NAME(CommonTermsQuery)
  /*
   * TODO maybe it would make sense to abstract this even further and allow to
   * rewrite to dismax rather than bool. Yet, this can already be subclassed
   * to do so.
   */
protected:
  const std::deque<std::shared_ptr<Term>> terms =
      std::deque<std::shared_ptr<Term>>();
  const float maxTermFrequency;
  const Occur lowFreqOccur;
  const Occur highFreqOccur;
  float lowFreqBoost = 1.0f;
  float highFreqBoost = 1.0f;
  float lowFreqMinNrShouldMatch = 0;
  float highFreqMinNrShouldMatch = 0;

  /**
   * Creates a new {@link CommonTermsQuery}
   *
   * @param highFreqOccur
   *          {@link Occur} used for high frequency terms
   * @param lowFreqOccur
   *          {@link Occur} used for low frequency terms
   * @param maxTermFrequency
   *          a value in [0..1) (or absolute number &gt;=1) representing the
   *          maximum threshold of a terms document frequency to be considered a
   *          low frequency term.
   * @throws IllegalArgumentException
   *           if {@link Occur#MUST_NOT} is pass as lowFreqOccur or
   *           highFreqOccur
   */
public:
  CommonTermsQuery(Occur highFreqOccur, Occur lowFreqOccur,
                   float maxTermFrequency);

  /**
   * Adds a term to the {@link CommonTermsQuery}
   *
   * @param term
   *          the term to add
   */
  virtual void add(std::shared_ptr<Term> term);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

protected:
  virtual int calcLowFreqMinimumNumberShouldMatch(int numOptional);

  virtual int calcHighFreqMinimumNumberShouldMatch(int numOptional);

private:
  int minNrShouldMatch(float minNrShouldMatch, int numOptional);

protected:
  virtual std::shared_ptr<Query>
  buildQuery(int const maxDoc,
             std::deque<std::shared_ptr<TermContext>> &contextArray,
             std::deque<std::shared_ptr<Term>> &queryTerms);

public:
  virtual void collectTermContext(
      std::shared_ptr<IndexReader> reader,
      std::deque<std::shared_ptr<LeafReaderContext>> &leaves,
      std::deque<std::shared_ptr<TermContext>> &contextArray,
      std::deque<std::shared_ptr<Term>> &queryTerms) ;

  /**
   * Specifies a minimum number of the low frequent optional BooleanClauses
   * which must be satisfied in order to produce a match on the low frequency
   * terms query part. This method accepts a float value in the range [0..1) as
   * a fraction of the actual query terms in the low frequent clause or a number
   * <tt>&gt;=1</tt> as an absolut number of clauses that need to match.
   *
   * <p>
   * By default no optional clauses are necessary for a match (unless there are
   * no required clauses). If this method is used, then the specified number of
   * clauses is required.
   * </p>
   *
   * @param min
   *          the number of optional clauses that must match
   */
  virtual void setLowFreqMinimumNumberShouldMatch(float min);

  /**
   * Gets the minimum number of the optional low frequent BooleanClauses which
   * must be satisfied.
   */
  virtual float getLowFreqMinimumNumberShouldMatch();

  /**
   * Specifies a minimum number of the high frequent optional BooleanClauses
   * which must be satisfied in order to produce a match on the low frequency
   * terms query part. This method accepts a float value in the range [0..1) as
   * a fraction of the actual query terms in the low frequent clause or a number
   * <tt>&gt;=1</tt> as an absolut number of clauses that need to match.
   *
   * <p>
   * By default no optional clauses are necessary for a match (unless there are
   * no required clauses). If this method is used, then the specified number of
   * clauses is required.
   * </p>
   *
   * @param min
   *          the number of optional clauses that must match
   */
  virtual void setHighFreqMinimumNumberShouldMatch(float min);

  /**
   * Gets the minimum number of the optional high frequent BooleanClauses which
   * must be satisfied.
   */
  virtual float getHighFreqMinimumNumberShouldMatch();

  /**
   * Gets the deque of terms.
   */
  virtual std::deque<std::shared_ptr<Term>> getTerms();

  /**
   * Gets the maximum threshold of a terms document frequency to be considered a
   * low frequency term.
   */
  virtual float getMaxTermFrequency();

  /**
   * Gets the {@link Occur} used for low frequency terms.
   */
  virtual Occur getLowFreqOccur();

  /**
   * Gets the {@link Occur} used for high frequency terms.
   */
  virtual Occur getHighFreqOccur();

  /**
   * Gets the boost used for low frequency terms.
   */
  virtual float getLowFreqBoost();

  /**
   * Gets the boost used for high frequency terms.
   */
  virtual float getHighFreqBoost();

  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<CommonTermsQuery> other);

  /**
   * Builds a new TermQuery instance.
   * <p>This is intended for subclasses that wish to customize the generated
   * queries.</p>
   * @param term term
   * @param context the TermContext to be used to create the low level term
   * query. Can be <code>null</code>.
   * @return new TermQuery instance
   */
protected:
  virtual std::shared_ptr<Query>
  newTermQuery(std::shared_ptr<Term> term,
               std::shared_ptr<TermContext> context);

protected:
  std::shared_ptr<CommonTermsQuery> shared_from_this()
  {
    return std::static_pointer_cast<CommonTermsQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries
