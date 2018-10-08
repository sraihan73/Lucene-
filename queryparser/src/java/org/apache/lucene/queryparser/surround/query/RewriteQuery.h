#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <type_traits>

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

template <typename SQ>
class RewriteQuery : public Query
{
  GET_CLASS_NAME(RewriteQuery)
  static_assert(std::is_base_of<SrndQuery, SQ>::value,
                L"SQ must inherit from SrndQuery");

protected:
  const SQ srndQuery;
  const std::wstring fieldName;
  const std::shared_ptr<BasicQueryFactory> qf;

public:
  RewriteQuery(SQ srndQuery, const std::wstring &fieldName,
               std::shared_ptr<BasicQueryFactory> qf)
      : srndQuery(Objects::requireNonNull(srndQuery)),
        fieldName(Objects::requireNonNull(fieldName)),
        qf(Objects::requireNonNull(qf))
  {
  }

  std::shared_ptr<Query> rewrite(std::shared_ptr<IndexReader> reader) = 0;
  override override;

  std::wstring toString(const std::wstring &field) override
  {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return getClassName() +
           (field.isEmpty() ? L"" : L"(unused: " + field + L")") + L"(" +
           fieldName + L", " + srndQuery->toString() + L", " + qf->toString() +
           L")";
  }

  int hashCode() override
  {
    return classHash() ^ fieldName.hashCode() ^ qf->hashCode() ^
           srndQuery->hashCode();
  }

  bool equals(std::any other) override
  {
    return sameClassAs(other) && equalsTo(getClass().cast(other));
  }

private:
  template <typename T1>
  bool equalsTo(std::shared_ptr<RewriteQuery<T1>> other)
  {
    return fieldName == other->fieldName && qf->equals(other->qf) &&
           srndQuery->equals(other->srndQuery);
  }

protected:
  std::shared_ptr<RewriteQuery> shared_from_this()
  {
    return std::static_pointer_cast<RewriteQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/query/
