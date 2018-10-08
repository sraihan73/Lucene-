#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/surround/query/SrndQuery.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/surround/query/BasicQueryFactory.h"

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

using Query = org::apache::lucene::search::Query;

/** Base class for composite queries (such as AND/OR/NOT) */
class ComposedQuery : public SrndQuery
{

public:
  ComposedQuery(std::deque<std::shared_ptr<SrndQuery>> &qs, bool operatorInfix,
                const std::wstring &opName);

protected:
  virtual void recompose(std::deque<std::shared_ptr<SrndQuery>> &queries);

  std::wstring opName;

public:
  virtual std::wstring getOperatorName();

protected:
  std::deque<std::shared_ptr<SrndQuery>> queries;

public:
  virtual std::shared_ptr<Iterator<std::shared_ptr<SrndQuery>>>
  getSubQueriesIterator();

  virtual int getNrSubQueries();

  virtual std::shared_ptr<SrndQuery> getSubQuery(int qn);

private:
  bool operatorInfix = false;

public:
  virtual bool isOperatorInfix();

  virtual std::deque<std::shared_ptr<Query>>
  makeLuceneSubQueriesField(const std::wstring &fn,
                            std::shared_ptr<BasicQueryFactory> qf);

  virtual std::wstring toString();

  /* Override for different spacing */
protected:
  virtual std::wstring getPrefixSeparator();
  virtual std::wstring getBracketOpen();
  virtual std::wstring getBracketClose();

  virtual void infixToString(std::shared_ptr<StringBuilder> r);

  virtual void prefixToString(std::shared_ptr<StringBuilder> r);

public:
  bool isFieldsSubQueryAcceptable() override;

protected:
  std::shared_ptr<ComposedQuery> shared_from_this()
  {
    return std::static_pointer_cast<ComposedQuery>(
        SrndQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/query/
