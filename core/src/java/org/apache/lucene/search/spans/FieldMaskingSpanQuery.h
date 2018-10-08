#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
using Query = org::apache::lucene::search::Query;

/**
 * <p>Wrapper to allow {@link SpanQuery} objects participate in composite
 * single-field SpanQueries by 'lying' about their search field. That is,
 * the masked SpanQuery will function as normal,
 * but {@link SpanQuery#getField()} simply hands back the value supplied
 * in this class's constructor.</p>
 *
 * <p>This can be used to support Queries like {@link SpanNearQuery} or
 * {@link SpanOrQuery} across different fields, which is not ordinarily
 * permitted.</p>
 *
 * <p>This can be useful for denormalized relational data: for example, when
 * indexing a document with conceptually many 'children': </p>
 *
 * <pre>
 *  teacherid: 1
 *  studentfirstname: james
 *  studentsurname: jones
 *
 *  teacherid: 2
 *  studenfirstname: james
 *  studentsurname: smith
 *  studentfirstname: sally
 *  studentsurname: jones
 * </pre>
 *
 * <p>a SpanNearQuery with a slop of 0 can be applied across two
 * {@link SpanTermQuery} objects as follows:
 * <pre class="prettyprint">
 *    SpanQuery q1  = new SpanTermQuery(new Term("studentfirstname", "james"));
 *    SpanQuery q2  = new SpanTermQuery(new Term("studentsurname", "jones"));
 *    SpanQuery q2m = new FieldMaskingSpanQuery(q2, "studentfirstname");
 *    Query q = new SpanNearQuery(new SpanQuery[]{q1, q2m}, -1, false);
 * </pre>
 * to search for 'studentfirstname:james studentsurname:jones' and find
 * teacherid 1 without matching teacherid 2 (which has a 'james' in position 0
 * and 'jones' in position 1).
 *
 * <p>Note: as {@link #getField()} returns the masked field, scoring will be
 * done using the Similarity and collection statistics of the field name
 * supplied, but with the term statistics of the real field. This may lead to
 * exceptions, poor performance, and unexpected scoring behaviour.
 */
class FieldMaskingSpanQuery final : public SpanQuery
{
  GET_CLASS_NAME(FieldMaskingSpanQuery)
private:
  const std::shared_ptr<SpanQuery> maskedQuery;
  const std::wstring field;

public:
  FieldMaskingSpanQuery(std::shared_ptr<SpanQuery> maskedQuery,
                        const std::wstring &maskedField);

  std::wstring getField() override;

  std::shared_ptr<SpanQuery> getMaskedQuery();

  // :NOTE: getBoost and setBoost are not proxied to the maskedQuery
  // ...this is done to be more consistent with things like SpanFirstQuery

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<FieldMaskingSpanQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<FieldMaskingSpanQuery> shared_from_this()
  {
    return std::static_pointer_cast<FieldMaskingSpanQuery>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
