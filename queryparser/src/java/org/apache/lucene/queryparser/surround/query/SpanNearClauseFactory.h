#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::queryparser::surround::query
{
class BasicQueryFactory;
}
namespace org::apache::lucene::search::spans
{
class SpanQuery;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class Query;
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
/*
SpanNearClauseFactory:

Operations:

- create for a field name and an indexreader.

- add a weighted Term
  this should add a corresponding SpanTermQuery, or
  increase the weight of an existing one.

- add a weighted subquery SpanNearQuery

- create a clause for SpanNearQuery from the things added above.
  For this, create an array of SpanQuery's from the added ones.
  The clause normally is a SpanOrQuery over the added subquery SpanNearQuery
  the SpanTermQuery's for the added Term's
*/

/* When  it is necessary to suppress double subqueries as much as possible:
   hashCode() and equals() on unweighted SpanQuery are needed (possibly via
   getTerms(), the terms are individually hashable). Idem SpanNearQuery: hash on
   the subqueries and the slop. Evt. merge SpanNearQuery's by adding the weights
   of the corresponding subqueries.
 */

/* To be determined:
   Are SpanQuery weights handled correctly during search by Lucene?
   Should the resulting SpanOrQuery be sorted?
   Could other SpanQueries be added for use in this factory:
   - SpanOrQuery: in principle yes, but it only has access to its terms
                  via getTerms(); are the corresponding weights available?
   - SpanFirstQuery: treat similar to subquery SpanNearQuery. (ok?)
   - SpanNotQuery: treat similar to subquery SpanNearQuery. (ok?)
 */

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * Factory for {@link SpanOrQuery}
 */
class SpanNearClauseFactory
    : public std::enable_shared_from_this<SpanNearClauseFactory>
{ // FIXME: rename to SpanClauseFactory
  GET_CLASS_NAME(SpanNearClauseFactory)
public:
  SpanNearClauseFactory(std::shared_ptr<IndexReader> reader,
                        const std::wstring &fieldName,
                        std::shared_ptr<BasicQueryFactory> qf);

private:
  std::shared_ptr<IndexReader> reader;
  std::wstring fieldName;
  std::unordered_map<std::shared_ptr<SpanQuery>, float> weightBySpanQuery;
  std::shared_ptr<BasicQueryFactory> qf;

public:
  virtual std::shared_ptr<IndexReader> getIndexReader();

  virtual std::wstring getFieldName();

  virtual std::shared_ptr<BasicQueryFactory> getBasicQueryFactory();

  virtual int size();

  virtual void clear();

protected:
  virtual void addSpanQueryWeighted(std::shared_ptr<SpanQuery> sq,
                                    float weight);

public:
  virtual void addTermWeighted(std::shared_ptr<Term> t,
                               float weight) ;

  virtual void addSpanQuery(std::shared_ptr<Query> q);

  virtual std::shared_ptr<SpanQuery> makeSpanClause();
};

} // namespace org::apache::lucene::queryparser::surround::query
