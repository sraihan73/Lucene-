#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/spans/FilterSpans.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

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

using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;

/**
 * Matches spans near the beginning of a field.
 * <p>
 * This class is a simple extension of {@link SpanPositionRangeQuery} in that it
 * assumes the start to be zero and only checks the end boundary.
 */
class SpanFirstQuery : public SpanPositionRangeQuery
{
  GET_CLASS_NAME(SpanFirstQuery)

  /** Construct a SpanFirstQuery matching spans in <code>match</code> whose end
   * position is less than or equal to <code>end</code>. */
public:
  SpanFirstQuery(std::shared_ptr<SpanQuery> match, int end);

protected:
  AcceptStatus
  acceptPosition(std::shared_ptr<Spans> spans)  override;

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<SpanFirstQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanFirstQuery>(
        SpanPositionRangeQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
