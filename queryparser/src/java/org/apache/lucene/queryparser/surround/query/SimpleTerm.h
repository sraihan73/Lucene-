#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::queryparser::surround::query
{
class MatchingTermVisitor;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::queryparser::surround::query
{
class SpanNearClauseFactory;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::surround::query
{
class BasicQueryFactory;
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
namespace org::apache::lucene::queryparser::surround::query
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;

/**
 * Base class for queries that expand to sets of simple terms.
 */
class SimpleTerm : public SrndQuery,
                   public DistanceSubQuery,
                   public Comparable<std::shared_ptr<SimpleTerm>>
{
  GET_CLASS_NAME(SimpleTerm)
public:
  SimpleTerm(bool q);

private:
  bool quoted = false;

public:
  virtual bool isQuoted();

  virtual std::wstring getQuote();
  virtual std::wstring getFieldOperator();

  virtual std::wstring toStringUnquoted() = 0;

  /** @deprecated (March 2011) Not normally used, to be removed from Lucene 4.0.
   *   This class implementing Comparable is to be removed at the same time.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Deprecated public int compareTo(SimpleTerm ost)
  int compareTo(std::shared_ptr<SimpleTerm> ost) override;

protected:
  virtual void suffixToString(std::shared_ptr<StringBuilder> r);

public:
  virtual std::wstring toString();

  virtual void visitMatchingTerms(std::shared_ptr<IndexReader> reader,
                                  const std::wstring &fieldName,
                                  std::shared_ptr<MatchingTermVisitor> mtv) = 0;

  /**
   * Callback to visit each matching term during "rewrite"
   * in {@link #visitMatchingTerm(Term)}
   */
public:
  class MatchingTermVisitor
  {
    GET_CLASS_NAME(MatchingTermVisitor)
  public:
    virtual void visitMatchingTerm(std::shared_ptr<Term> t) = 0;
  };

public:
  std::wstring distanceSubQueryNotAllowed() override;
  void addSpanQueries(std::shared_ptr<SpanNearClauseFactory> sncf) throw(
      IOException) override;

private:
  class MatchingTermVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            MatchingTermVisitorAnonymousInnerClass>,
        public MatchingTermVisitor
  {
    GET_CLASS_NAME(MatchingTermVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTerm> outerInstance;

    std::shared_ptr<org::apache::lucene::queryparser::surround::query::
                        SpanNearClauseFactory>
        sncf;

  public:
    MatchingTermVisitorAnonymousInnerClass(
        std::shared_ptr<SimpleTerm> outerInstance,
        std::shared_ptr<org::apache::lucene::queryparser::surround::query::
                            SpanNearClauseFactory>
            sncf);

    void
    visitMatchingTerm(std::shared_ptr<Term> term)  override;
  };

public:
  std::shared_ptr<Query>
  makeLuceneQueryFieldNoBoost(const std::wstring &fieldName,
                              std::shared_ptr<BasicQueryFactory> qf) override;

protected:
  std::shared_ptr<SimpleTerm> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTerm>(SrndQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
