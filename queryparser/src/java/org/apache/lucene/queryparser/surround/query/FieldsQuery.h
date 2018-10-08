#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::surround::query
{
class SrndQuery;
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

using Query = org::apache::lucene::search::Query;

/**
 * Forms an OR query of the provided query across multiple fields.
 */
class FieldsQuery : public SrndQuery
{ // mostly untested
  GET_CLASS_NAME(FieldsQuery)
private:
  std::shared_ptr<SrndQuery> q;
  std::deque<std::wstring> fieldNames;
  const wchar_t fieldOp;
  static const std::wstring
      OR_OPERATOR_NAME; // for expanded queries, not normally visible

public:
  FieldsQuery(std::shared_ptr<SrndQuery> q,
              std::deque<std::wstring> &fieldNames, wchar_t fieldOp);

  FieldsQuery(std::shared_ptr<SrndQuery> q, const std::wstring &fieldName,
              wchar_t fieldOp);

  bool isFieldsSubQueryAcceptable() override;

  virtual std::shared_ptr<Query>
  makeLuceneQueryNoBoost(std::shared_ptr<BasicQueryFactory> qf);

  std::shared_ptr<Query>
  makeLuceneQueryFieldNoBoost(const std::wstring &fieldName,
                              std::shared_ptr<BasicQueryFactory> qf) override;

  virtual std::deque<std::wstring> getFieldNames();

  virtual wchar_t getFieldOperator();

  virtual std::wstring toString();

protected:
  virtual void fieldNamesToString(std::shared_ptr<StringBuilder> r);

protected:
  std::shared_ptr<FieldsQuery> shared_from_this()
  {
    return std::static_pointer_cast<FieldsQuery>(SrndQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
