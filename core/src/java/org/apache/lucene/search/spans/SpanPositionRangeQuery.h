#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
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
 * Checks to see if the {@link #getMatch()} lies between a start and end
 * position
 *
 * See {@link SpanFirstQuery} for a derivation that is optimized for the case
 * where start position is 0.
 */
class SpanPositionRangeQuery : public SpanPositionCheckQuery
{
  GET_CLASS_NAME(SpanPositionRangeQuery)
protected:
  int start = 0;
  int end = 0;

public:
  SpanPositionRangeQuery(std::shared_ptr<SpanQuery> match, int start, int end);

protected:
  AcceptStatus
  acceptPosition(std::shared_ptr<Spans> spans)  override;

  /**
   * @return The minimum position permitted in a match
   */
public:
  virtual int getStart();

  /**
   * @return the maximum end position permitted in a match.
   */
  virtual int getEnd();

  std::wstring toString(const std::wstring &field) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<SpanPositionRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanPositionRangeQuery>(
        SpanPositionCheckQuery::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/search/spans/
