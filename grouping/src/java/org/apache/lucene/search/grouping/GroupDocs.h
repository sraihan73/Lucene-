#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

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
namespace org::apache::lucene::search::grouping
{

using ScoreDoc = org::apache::lucene::search::ScoreDoc;

/** Represents one group in the results.
 *
 * @lucene.experimental */
template <typename T>
class GroupDocs : public std::enable_shared_from_this<GroupDocs>
{
  GET_CLASS_NAME(GroupDocs)
  /** The groupField value for all docs in this group; this
   *  may be null if hits did not have the groupField. */
public:
  const T groupValue;

  /** Max score in this group */
  const float maxScore;

  /** Overall aggregated score of this group (currently only
   *  set by join queries). */
  const float score;

  /** Hits; this may be {@link
   * org.apache.lucene.search.FieldDoc} instances if the
   * withinGroupSort sorted by fields. */
  std::deque<std::shared_ptr<ScoreDoc>> const scoreDocs;

  /** Total hits within this group */
  const int64_t totalHits;

  /** Matches the groupSort passed to {@link
   *  FirstPassGroupingCollector}. */
  std::deque<std::any> const groupSortValues;

  GroupDocs(float score, float maxScore, int64_t totalHits,
            std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs, T groupValue,
            std::deque<std::any> &groupSortValues)
      : groupValue(groupValue), maxScore(maxScore), score(score),
        scoreDocs(scoreDocs), totalHits(totalHits),
        groupSortValues(groupSortValues)
  {
  }
};

} // namespace org::apache::lucene::search::grouping
