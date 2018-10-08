#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::surround::query
{
class SimpleTerm;
}

namespace org::apache::lucene::queryparser::surround::query
{
class BasicQueryFactory;
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
class Term;
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
using Query = org::apache::lucene::search::Query;

class SimpleTermRewriteQuery : public RewriteQuery<std::shared_ptr<SimpleTerm>>
{
  GET_CLASS_NAME(SimpleTermRewriteQuery)

public:
  SimpleTermRewriteQuery(std::shared_ptr<SimpleTerm> srndQuery,
                         const std::wstring &fieldName,
                         std::shared_ptr<BasicQueryFactory> qf);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

private:
  class MatchingTermVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            MatchingTermVisitorAnonymousInnerClass>,
        public SimpleTerm::MatchingTermVisitor
  {
    GET_CLASS_NAME(MatchingTermVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTermRewriteQuery> outerInstance;

    std::deque<std::shared_ptr<Query>> luceneSubQueries;

  public:
    MatchingTermVisitorAnonymousInnerClass(
        std::shared_ptr<SimpleTermRewriteQuery> outerInstance,
        std::deque<std::shared_ptr<Query>> &luceneSubQueries);

    void
    visitMatchingTerm(std::shared_ptr<Term> term)  override;
  };

protected:
  std::shared_ptr<SimpleTermRewriteQuery> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTermRewriteQuery>(
        RewriteQuery<SimpleTerm>::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
