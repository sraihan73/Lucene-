#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::surround::query
{
class TooManyBasicQueries;
}

namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class TermQuery;
}
namespace org::apache::lucene::search::spans
{
class SpanTermQuery;
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
/* Create basic queries to be used during rewrite.
 * The basic queries are TermQuery and SpanTermQuery.
 * An exception can be thrown when too many of these are used.
 * SpanTermQuery and TermQuery use IndexReader.termEnum(Term), which causes the
 * buffer usage.
 *
 * Use this class to limit the buffer usage for reading terms from an index.
 * Default is 1024, the same as the max. number of subqueries for a
 * BooleanQuery.
 */

using Term = org::apache::lucene::index::Term;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

/** Factory for creating basic term queries */
class BasicQueryFactory : public std::enable_shared_from_this<BasicQueryFactory>
{
  GET_CLASS_NAME(BasicQueryFactory)
public:
  BasicQueryFactory(int maxBasicQueries);

  BasicQueryFactory();

private:
  int maxBasicQueries = 0;
  int queriesMade = 0;

public:
  virtual int getNrQueriesMade();
  virtual int getMaxBasicQueries();

  virtual std::wstring toString();

private:
  bool atMax();

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void checkMax() ;

public:
  virtual std::shared_ptr<TermQuery>
  newTermQuery(std::shared_ptr<Term> term) ;

  virtual std::shared_ptr<SpanTermQuery>
  newSpanTermQuery(std::shared_ptr<Term> term) ;

  virtual int hashCode();

  /** Two BasicQueryFactory's are equal when they generate
   *  the same types of basic queries, or both cannot generate queries anymore.
   */
  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::queryparser::surround::query
